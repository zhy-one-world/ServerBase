#include "http_server.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <vector>

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "crypt32.lib")

namespace faith
{
	namespace
	{
		constexpr int k_https_read_timeout_ms = 10000;
		constexpr std::size_t k_https_max_request = 1024 * 1024;

		bool set_socket_timeout(SOCKET sock, int timeout_ms)
		{
			return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
				reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms)) == 0
				&& setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,
					reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms)) == 0;
		}

		std::string status_reason(int status)
		{
			switch (status)
			{
			case 200: return "OK";
			case 400: return "Bad Request";
			case 404: return "Not Found";
			case 405: return "Method Not Allowed";
			case 500: return "Internal Server Error";
			default: return "OK";
			}
		}
	}

	http_server::~http_server()
	{
		stop();
	}

	bool http_server::listen(const http_listen_options& options, http_inbound_handler handler)
	{
		if (m_listening.load() || !handler || options.port <= 0)
		{
			return false;
		}

		m_options = options;
		m_handler = handler;
		m_stop = false;

		bool ok = false;
		if (options.scheme == http_scheme::https)
		{
			ok = start_https();
		}
		else
		{
			ok = start_http();
		}

		if (!ok)
		{
			stop();
			return false;
		}
		m_listening = true;
		return true;
	}

	bool http_server::start_http()
	{
		m_event_base = event_base_new();
		if (m_event_base == nullptr)
		{
			return false;
		}
		m_evhttp = evhttp_new(m_event_base);
		if (m_evhttp == nullptr)
		{
			return false;
		}
		m_bound = evhttp_bind_socket_with_handle(
			m_evhttp, m_options.bind_ip.c_str(), static_cast<ev_uint16_t>(m_options.port));
		if (m_bound == nullptr)
		{
			return false;
		}
		evhttp_set_gencb(m_evhttp, &http_server::evhttp_generic_callback, this);
		m_http_thread = std::make_unique<boost::thread>(&http_server::http_loop, this);
		return true;
	}

	void http_server::http_loop()
	{
		while (!m_stop.load())
		{
			event_base_loop(m_event_base, EVLOOP_NONBLOCK);
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		}
	}

	bool http_server::load_ssl_ctx()
	{
		OPENSSL_init_ssl(0, nullptr);
		m_ssl_ctx = SSL_CTX_new(TLS_server_method());
		if (m_ssl_ctx == nullptr)
		{
			return false;
		}
		if (m_options.ssl.cert_file.empty() || m_options.ssl.key_file.empty())
		{
			return false;
		}
		if (SSL_CTX_use_certificate_file(m_ssl_ctx, m_options.ssl.cert_file.c_str(), SSL_FILETYPE_PEM) != 1)
		{
			return false;
		}
		if (SSL_CTX_use_PrivateKey_file(m_ssl_ctx, m_options.ssl.key_file.c_str(), SSL_FILETYPE_PEM) != 1)
		{
			return false;
		}
		if (SSL_CTX_check_private_key(m_ssl_ctx) != 1)
		{
			return false;
		}
		if (!m_options.ssl.ca_file.empty())
		{
			if (SSL_CTX_load_verify_locations(m_ssl_ctx, m_options.ssl.ca_file.c_str(), nullptr) != 1)
			{
				return false;
			}
		}
		if (m_options.ssl.require_client_cert)
		{
			SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
		}
		return true;
	}

	void http_server::free_ssl_ctx()
	{
		if (m_ssl_ctx)
		{
			SSL_CTX_free(m_ssl_ctx);
			m_ssl_ctx = nullptr;
		}
	}

	bool http_server::start_https()
	{
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);

		if (!load_ssl_ctx())
		{
			free_ssl_ctx();
			return false;
		}

		SOCKET listen_sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (listen_sock == INVALID_SOCKET)
		{
			free_ssl_ctx();
			return false;
		}

		BOOL reuse = TRUE;
		setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

		sockaddr_in addr;
		std::memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(static_cast<u_short>(m_options.port));
		if (m_options.bind_ip == "0.0.0.0" || m_options.bind_ip.empty())
		{
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
		}
		else if (InetPtonA(AF_INET, m_options.bind_ip.c_str(), &addr.sin_addr) != 1)
		{
			closesocket(listen_sock);
			free_ssl_ctx();
			return false;
		}

		if (bind(listen_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
		{
			closesocket(listen_sock);
			free_ssl_ctx();
			return false;
		}
		if (::listen(listen_sock, SOMAXCONN) != 0)
		{
			closesocket(listen_sock);
			free_ssl_ctx();
			return false;
		}

		m_listen_socket = static_cast<std::uintptr_t>(listen_sock);
		m_https_thread = std::make_unique<boost::thread>(&http_server::https_loop, this);
		return true;
	}

	bool http_server::read_https_request(SSL* ssl, std::string& raw)
	{
		raw.clear();
		char buf[4096];
		while (raw.size() < k_https_max_request)
		{
			const int n = SSL_read(ssl, buf, sizeof(buf));
			if (n <= 0)
			{
				return !raw.empty();
			}
			raw.append(buf, buf + n);
			const auto header_end = raw.find("\r\n\r\n");
			if (header_end == std::string::npos)
			{
				continue;
			}
			std::size_t content_length = 0;
			const std::string headers = raw.substr(0, header_end);
			std::string lower = headers;
			std::transform(lower.begin(), lower.end(), lower.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			const auto cl_pos = lower.find("content-length:");
			if (cl_pos != std::string::npos)
			{
				const auto line_end = lower.find("\r\n", cl_pos);
				const auto value_begin = cl_pos + std::strlen("content-length:");
				const std::string value = headers.substr(
					value_begin,
					(line_end == std::string::npos ? headers.size() : line_end) - value_begin);
				content_length = static_cast<std::size_t>(std::strtoul(value.c_str(), nullptr, 10));
			}
			const std::size_t total = header_end + 4 + content_length;
			if (raw.size() >= total)
			{
				raw.resize(total);
				return true;
			}
		}
		return false;
	}

	bool http_server::parse_http_request(const std::string& raw, http_inbound_request& out, std::string& error)
	{
		const auto header_end = raw.find("\r\n\r\n");
		if (header_end == std::string::npos)
		{
			error = "incomplete headers";
			return false;
		}
		std::istringstream stream(raw.substr(0, header_end));
		std::string request_line;
		if (!std::getline(stream, request_line))
		{
			error = "missing request line";
			return false;
		}
		if (!request_line.empty() && request_line.back() == '\r')
		{
			request_line.pop_back();
		}

		std::string method;
		std::string target;
		std::string version;
		{
			std::istringstream line(request_line);
			line >> method >> target >> version;
		}
		if (method.empty() || target.empty())
		{
			error = "bad request line";
			return false;
		}

		if (method == "GET")
		{
			out.method = EVHTTP_REQ_GET;
		}
		else if (method == "POST")
		{
			out.method = EVHTTP_REQ_POST;
		}
		else if (method == "PUT")
		{
			out.method = EVHTTP_REQ_PUT;
		}
		else if (method == "DELETE")
		{
			out.method = EVHTTP_REQ_DELETE;
		}
		else
		{
			out.method = EVHTTP_REQ_GET;
		}

		out.full_path = target;
		const auto qpos = target.find('?');
		const auto fpos = target.find('#');
		const auto path_end = (qpos < fpos) ? qpos : fpos;
		out.path = target.substr(0, path_end);
		if (qpos != std::string::npos)
		{
			const auto qend = (fpos != std::string::npos && fpos > qpos) ? fpos : std::string::npos;
			out.query = target.substr(qpos + 1, qend == std::string::npos ? std::string::npos : qend - qpos - 1);
		}
		if (fpos != std::string::npos)
		{
			out.fragment = target.substr(fpos + 1);
		}

		std::string header_line;
		while (std::getline(stream, header_line))
		{
			if (!header_line.empty() && header_line.back() == '\r')
			{
				header_line.pop_back();
			}
			if (header_line.empty())
			{
				break;
			}
			const auto colon = header_line.find(':');
			if (colon == std::string::npos)
			{
				continue;
			}
			std::string key = header_line.substr(0, colon);
			std::string value = header_line.substr(colon + 1);
			while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
			{
				value.erase(value.begin());
			}
			out.headers[key] = value;
		}

		out.body = raw.substr(header_end + 4);
		out.listen_port = m_options.port;
		return true;
	}

	bool http_server::write_https_response(SSL* ssl, const http_outbound_response& response)
	{
		std::ostringstream oss;
		const std::string reason = response.reason.empty() ? status_reason(response.status) : response.reason;
		oss << "HTTP/1.1 " << response.status << " " << reason << "\r\n";
		bool has_content_type = false;
		bool has_content_length = false;
		bool has_connection = false;
		for (const auto& h : response.headers)
		{
			oss << h.first << ": " << h.second << "\r\n";
			std::string key = h.first;
			std::transform(key.begin(), key.end(), key.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			if (key == "content-type") has_content_type = true;
			if (key == "content-length") has_content_length = true;
			if (key == "connection") has_connection = true;
		}
		if (!has_content_type)
		{
			oss << "Content-Type: application/json; charset=utf-8\r\n";
		}
		if (!has_content_length)
		{
			oss << "Content-Length: " << response.body.size() << "\r\n";
		}
		if (!has_connection)
		{
			oss << "Connection: close\r\n";
		}
		oss << "\r\n" << response.body;
		const std::string payload = oss.str();
		std::size_t sent = 0;
		while (sent < payload.size())
		{
			const int n = SSL_write(ssl, payload.data() + sent, static_cast<int>(payload.size() - sent));
			if (n <= 0)
			{
				return false;
			}
			sent += static_cast<std::size_t>(n);
		}
		return true;
	}

	void http_server::close_https_pending(pending_inbound& item)
	{
		if (item.ssl)
		{
			SSL_shutdown(item.ssl);
			SSL_free(item.ssl);
			item.ssl = nullptr;
		}
		if (item.socket != static_cast<std::uintptr_t>(-1))
		{
			closesocket(static_cast<SOCKET>(item.socket));
			item.socket = static_cast<std::uintptr_t>(-1);
		}
	}

	void http_server::https_loop()
	{
		SOCKET listen_sock = static_cast<SOCKET>(m_listen_socket);
		while (!m_stop.load())
		{
			fd_set read_set;
			FD_ZERO(&read_set);
			FD_SET(listen_sock, &read_set);
			timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 200 * 1000;
			const int ready = select(0, &read_set, nullptr, nullptr, &tv);
			if (ready <= 0 || !FD_ISSET(listen_sock, &read_set))
			{
				continue;
			}

			sockaddr_in peer;
			int peer_len = sizeof(peer);
			SOCKET client = accept(listen_sock, reinterpret_cast<sockaddr*>(&peer), &peer_len);
			if (client == INVALID_SOCKET)
			{
				continue;
			}
			set_socket_timeout(client, k_https_read_timeout_ms);

			SSL* ssl = SSL_new(m_ssl_ctx);
			if (ssl == nullptr)
			{
				closesocket(client);
				continue;
			}
			SSL_set_fd(ssl, static_cast<int>(client));
			if (SSL_accept(ssl) != 1)
			{
				SSL_free(ssl);
				closesocket(client);
				continue;
			}

			std::string raw;
			if (!read_https_request(ssl, raw))
			{
				SSL_shutdown(ssl);
				SSL_free(ssl);
				closesocket(client);
				continue;
			}

			http_inbound_request inbound;
			std::string error;
			if (!parse_http_request(raw, inbound, error))
			{
				http_outbound_response bad;
				bad.status = 400;
				bad.reason = "Bad Request";
				bad.body = "{\"ok\":false,\"error\":\"bad request\"}";
				write_https_response(ssl, bad);
				SSL_shutdown(ssl);
				SSL_free(ssl);
				closesocket(client);
				continue;
			}

			char ip_buf[INET_ADDRSTRLEN] = { 0 };
			InetNtopA(AF_INET, &peer.sin_addr, ip_buf, sizeof(ip_buf));
			inbound.client_ip = ip_buf;

			pending_inbound pending;
			pending.is_https = true;
			pending.ssl = ssl;
			pending.socket = static_cast<std::uintptr_t>(client);
			pending.info = inbound;

			long handle = 0;
			{
				std::lock_guard<std::mutex> lock(m_pending_mutex);
				handle = m_next_handle++;
				pending.info.handle = handle;
				m_pending.emplace(handle, pending);
			}

			if (m_handler)
			{
				m_handler(pending.info);
			}
		}
	}

	void http_server::evhttp_generic_callback(evhttp_request* req, void* arg)
	{
		auto* self = static_cast<http_server*>(arg);
		if (self)
		{
			self->on_evhttp_request(req);
		}
	}

	void http_server::on_evhttp_request(evhttp_request* req)
	{
		if (req == nullptr)
		{
			return;
		}

		http_inbound_request inbound;
		inbound.listen_port = m_options.port;
		inbound.method = evhttp_request_get_command(req);
		const char* full_path = evhttp_request_get_uri(req);
		if (full_path)
		{
			inbound.full_path = full_path;
		}
		evhttp_uri* decoded = evhttp_uri_parse(full_path);
		if (decoded == nullptr)
		{
			evhttp_send_error(req, HTTP_BADREQUEST, 0);
			return;
		}
		if (const char* path = evhttp_uri_get_path(decoded))
		{
			inbound.path = path;
		}
		if (const char* query = evhttp_uri_get_query(decoded))
		{
			inbound.query = query;
		}
		if (const char* fragment = evhttp_uri_get_fragment(decoded))
		{
			inbound.fragment = fragment;
		}
		if (req->remote_host)
		{
			inbound.client_ip = req->remote_host;
		}

		evbuffer* input = evhttp_request_get_input_buffer(req);
		if (input)
		{
			const size_t len = evbuffer_get_length(input);
			if (len > 0)
			{
				inbound.body.assign(reinterpret_cast<const char*>(evbuffer_pullup(input, -1)), len);
			}
		}

		if (evkeyvalq* headers = evhttp_request_get_input_headers(req))
		{
			for (evkeyval* it = headers->tqh_first; it != nullptr; it = it->next.tqe_next)
			{
				if (it->key && it->value)
				{
					inbound.headers[it->key] = it->value;
				}
			}
		}
		evhttp_uri_free(decoded);

		pending_inbound pending;
		pending.is_https = false;
		pending.ev_req = req;
		pending.info = inbound;

		long handle = 0;
		{
			std::lock_guard<std::mutex> lock(m_pending_mutex);
			handle = m_next_handle++;
			pending.info.handle = handle;
			m_pending.emplace(handle, pending);
		}

		if (m_handler)
		{
			m_handler(pending.info);
		}
	}

	void http_server::reply(long handle, int status, const std::string& body)
	{
		http_outbound_response response;
		response.status = status;
		response.reason = status_reason(status);
		response.body = body;
		reply(handle, response);
	}

	void http_server::reply(long handle, const http_outbound_response& response)
	{
		pending_inbound pending;
		{
			std::lock_guard<std::mutex> lock(m_pending_mutex);
			auto it = m_pending.find(handle);
			if (it == m_pending.end())
			{
				return;
			}
			pending = it->second;
			m_pending.erase(it);
		}

		if (pending.is_https)
		{
			if (pending.ssl)
			{
				write_https_response(pending.ssl, response);
			}
			close_https_pending(pending);
			return;
		}

		if (pending.ev_req == nullptr)
		{
			return;
		}
		evbuffer* buf = evbuffer_new();
		if (buf == nullptr)
		{
			return;
		}
		if (!response.body.empty())
		{
			evbuffer_add(buf, response.body.data(), response.body.size());
		}
		for (const auto& h : response.headers)
		{
			evhttp_add_header(evhttp_request_get_output_headers(pending.ev_req),
				h.first.c_str(), h.second.c_str());
		}
		if (response.headers.empty())
		{
			evhttp_add_header(evhttp_request_get_output_headers(pending.ev_req),
				"Content-Type", "application/json; charset=utf-8");
		}
		evhttp_send_reply(pending.ev_req, response.status, response.reason.c_str(), buf);
		evbuffer_free(buf);
	}

	void http_server::stop()
	{
		m_stop = true;
		if (m_http_thread && m_http_thread->joinable())
		{
			m_http_thread->join();
		}
		m_http_thread.reset();

		if (m_https_thread && m_https_thread->joinable())
		{
			m_https_thread->join();
		}
		m_https_thread.reset();

		{
			std::lock_guard<std::mutex> lock(m_pending_mutex);
			for (auto& item : m_pending)
			{
				if (item.second.is_https)
				{
					close_https_pending(item.second);
				}
				else if (item.second.ev_req)
				{
					evhttp_send_error(item.second.ev_req, HTTP_SERVUNAVAIL, "server closed");
					item.second.ev_req = nullptr;
				}
			}
			m_pending.clear();
		}

		if (m_bound && m_evhttp)
		{
			evhttp_del_accept_socket(m_evhttp, m_bound);
			m_bound = nullptr;
		}
		if (m_evhttp)
		{
			evhttp_free(m_evhttp);
			m_evhttp = nullptr;
		}
		if (m_event_base)
		{
			event_base_free(m_event_base);
			m_event_base = nullptr;
		}

		if (m_listen_socket != static_cast<std::uintptr_t>(-1))
		{
			closesocket(static_cast<SOCKET>(m_listen_socket));
			m_listen_socket = static_cast<std::uintptr_t>(-1);
		}
		free_ssl_ctx();
		m_listening = false;
		m_handler = http_inbound_handler();
	}
}
