#include "http_client.hpp"

#include <cstring>

namespace faith
{
	http_client::~http_client()
	{
		shutdown();
	}

	bool http_client::init()
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		if (m_inited)
		{
			return true;
		}
		if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
		{
			return false;
		}
		m_multi = curl_multi_init();
		if (m_multi == nullptr)
		{
			curl_global_cleanup();
			return false;
		}
		m_inited = true;
		return true;
	}

	void http_client::shutdown()
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		if (!m_inited)
		{
			return;
		}
		for (auto& item : m_pending)
		{
			if (item.second && item.second->easy)
			{
				curl_multi_remove_handle(m_multi, item.second->easy);
				curl_easy_cleanup(item.second->easy);
				item.second->easy = nullptr;
			}
			if (item.second && item.second->headers)
			{
				curl_slist_free_all(item.second->headers);
				item.second->headers = nullptr;
			}
		}
		m_pending.clear();
		if (m_multi)
		{
			curl_multi_cleanup(m_multi);
			m_multi = nullptr;
		}
		curl_global_cleanup();
		m_inited = false;
	}

	size_t http_client::write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
	{
		auto* item = static_cast<pending_request*>(userdata);
		if (item == nullptr || ptr == nullptr)
		{
			return 0;
		}
		const size_t bytes = size * nmemb;
		item->response_body.append(ptr, bytes);
		return bytes;
	}

	bool http_client::configure_easy(pending_request& item)
	{
		item.easy = curl_easy_init();
		if (item.easy == nullptr)
		{
			return false;
		}

		curl_easy_setopt(item.easy, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(item.easy, CURLOPT_WRITEFUNCTION, &http_client::write_callback);
		curl_easy_setopt(item.easy, CURLOPT_WRITEDATA, &item);
		curl_easy_setopt(item.easy, CURLOPT_PRIVATE, &item);
		curl_easy_setopt(item.easy, CURLOPT_ERRORBUFFER, item.error_buf);
		curl_easy_setopt(item.easy, CURLOPT_BUFFERSIZE, 512 * 1024L);

		long proto_mask = CURLPROTO_HTTP | CURLPROTO_HTTPS;
		curl_easy_setopt(item.easy, CURLOPT_PROTOCOLS, proto_mask);
		curl_easy_setopt(item.easy, CURLOPT_REDIR_PROTOCOLS, proto_mask);

		curl_easy_setopt(item.easy, CURLOPT_SSL_VERIFYPEER, item.request.ssl.verify_peer ? 1L : 0L);
		curl_easy_setopt(item.easy, CURLOPT_SSL_VERIFYHOST, item.request.ssl.verify_host ? 2L : 0L);
		if (!item.request.ssl.ca_file.empty())
		{
			curl_easy_setopt(item.easy, CURLOPT_CAINFO, item.request.ssl.ca_file.c_str());
		}
		if (!item.request.ssl.cert_file.empty())
		{
			curl_easy_setopt(item.easy, CURLOPT_SSLCERT, item.request.ssl.cert_file.c_str());
		}
		if (!item.request.ssl.key_file.empty())
		{
			curl_easy_setopt(item.easy, CURLOPT_SSLKEY, item.request.ssl.key_file.c_str());
		}

		if (item.request.method == e_http_request_type_get)
		{
			curl_easy_setopt(item.easy, CURLOPT_HTTPGET, 1L);
			curl_easy_setopt(item.easy, CURLOPT_URL, (item.request.url + item.request.body).c_str());
		}
		else
		{
			curl_easy_setopt(item.easy, CURLOPT_URL, item.request.url.c_str());
			curl_easy_setopt(item.easy, CURLOPT_POST, 1L);
			curl_easy_setopt(item.easy, CURLOPT_COPYPOSTFIELDS, item.request.body.c_str());
		}

		for (const auto& header : item.request.headers)
		{
			item.headers = curl_slist_append(item.headers, header.c_str());
		}
		if (item.headers)
		{
			curl_easy_setopt(item.easy, CURLOPT_HTTPHEADER, item.headers);
		}
		return true;
	}

	bool http_client::request_async(const http_request& request, http_client_handler handler)
	{
		if (!handler || request.url.empty())
		{
			return false;
		}
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		if (!m_inited && !init())
		{
			return false;
		}

		auto item = std::make_unique<pending_request>();
		item->request = request;
		item->handler = handler;
		if (!configure_easy(*item))
		{
			return false;
		}

		const CURLMcode mc = curl_multi_add_handle(m_multi, item->easy);
		if (mc != CURLM_OK)
		{
			curl_easy_cleanup(item->easy);
			if (item->headers)
			{
				curl_slist_free_all(item->headers);
			}
			return false;
		}

		m_pending.emplace(m_next_id++, std::move(item));
		return true;
	}

	void http_client::complete_finished()
	{
		int msgs = 0;
		while (CURLMsg* msg = curl_multi_info_read(m_multi, &msgs))
		{
			if (msg->msg != CURLMSG_DONE || msg->easy_handle == nullptr)
			{
				continue;
			}

			pending_request* doing = nullptr;
			curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &doing);
			if (doing == nullptr)
			{
				curl_multi_remove_handle(m_multi, msg->easy_handle);
				curl_easy_cleanup(msg->easy_handle);
				continue;
			}

			http_response response;
			response.error_code = static_cast<int>(msg->data.result);
			if (msg->data.result != CURLE_OK)
			{
				response.error = doing->error_buf[0] ? doing->error_buf : curl_easy_strerror(msg->data.result);
			}
			else
			{
				long status = 0;
				curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &status);
				response.http_status = static_cast<int>(status);
				response.body = doing->response_body;
			}

			http_client_handler handler = doing->handler;
			CURL* easy = doing->easy;
			curl_slist* headers = doing->headers;

			for (auto it = m_pending.begin(); it != m_pending.end(); ++it)
			{
				if (it->second.get() == doing)
				{
					m_pending.erase(it);
					break;
				}
			}

			curl_multi_remove_handle(m_multi, easy);
			curl_easy_cleanup(easy);
			if (headers)
			{
				curl_slist_free_all(headers);
			}

			if (handler)
			{
				handler(response);
			}
		}
	}

	void http_client::poll()
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		if (!m_inited || m_pending.empty())
		{
			return;
		}

		fd_set fdread;
		fd_set fdwrite;
		fd_set fdexcep;
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);

		int maxfd = -1;
		curl_multi_fdset(m_multi, &fdread, &fdwrite, &fdexcep, &maxfd);
		if (maxfd != -1)
		{
			timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 1000;
			select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
		}

		int running = 0;
		while (curl_multi_perform(m_multi, &running) == CURLM_CALL_MULTI_PERFORM)
		{
		}
		complete_finished();
	}
}
