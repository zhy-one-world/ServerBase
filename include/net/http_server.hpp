#pragma once

#include <atomic>
#include <boost/thread.hpp>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "http_types.hpp"
#include "singleton.hpp"

struct ssl_ctx_st;
typedef struct ssl_ctx_st SSL_CTX;
struct ssl_st;
typedef struct ssl_st SSL;
struct event_base;
struct evhttp;
struct evhttp_bound_socket;
struct evhttp_request;

namespace faith
{
	class http_server : public singleton<http_server>
	{
		friend class singleton<http_server>;

	public:
		bool listen(const http_listen_options& options, http_inbound_handler handler);
		void reply(long handle, const http_outbound_response& response);
		void reply(long handle, int status, const std::string& body);
		void stop();
		bool is_listening() const { return m_listening.load(); }
		int listen_port() const { return m_options.port; }

	private:
		http_server() = default;
		~http_server();

		bool start_http();
		bool start_https();
		void http_loop();
		void https_loop();

		bool load_ssl_ctx();
		void free_ssl_ctx();

		static void evhttp_generic_callback(evhttp_request* req, void* arg);
		void on_evhttp_request(evhttp_request* req);

		struct pending_inbound
		{
			bool is_https = false;
			evhttp_request* ev_req = nullptr;
			SSL* ssl = nullptr;
			std::uintptr_t socket = static_cast<std::uintptr_t>(-1);
			http_inbound_request info;
		};

		bool parse_http_request(const std::string& raw, http_inbound_request& out, std::string& error);
		bool read_https_request(SSL* ssl, std::string& raw);
		bool write_https_response(SSL* ssl, const http_outbound_response& response);
		void close_https_pending(pending_inbound& item);

		http_listen_options m_options;
		http_inbound_handler m_handler;
		std::atomic<bool> m_listening{ false };
		std::atomic<bool> m_stop{ false };

		event_base* m_event_base = nullptr;
		evhttp* m_evhttp = nullptr;
		evhttp_bound_socket* m_bound = nullptr;
		std::unique_ptr<boost::thread> m_http_thread;

		SSL_CTX* m_ssl_ctx = nullptr;
		std::uintptr_t m_listen_socket = static_cast<std::uintptr_t>(-1);
		std::unique_ptr<boost::thread> m_https_thread;

		std::mutex m_pending_mutex;
		long m_next_handle = 1;
		std::map<long, pending_inbound> m_pending;
	};
}
