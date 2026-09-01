#pragma once
#include "http_client_unit.h"
#include "http_server_unit.h"
#include "event2/http_struct.h"
#include "event2/keyvalq_struct.h"


namespace faith
{
	typedef std::map<long long, http_client_unit*> http_client_map;
	typedef http_client_map::iterator http_client_map_it;

	typedef std::map<int, http_server_unit*> http_server_map;
	typedef http_server_map::iterator http_server_map_it;

	class http_manager
	{
	public:
		http_manager();
		~http_manager();

	public:
		void					clear_data();
		bool					init();
		void					run();
		bool					is_runing() { return m_is_runing; };
		void					set_runing(bool is_run) { m_is_runing = is_run; };

		void					stop_client_run();
		void					stop_server_run();

	public:
		//---------------------------------------对外部分---------------------------------------

		long					async_request(const std::string& url, std::vector<std::string>& head_list, const std::string& params, http_client_callback_type handler, e_http_request_type request_type);
		bool					start_listen(int listen_port, http_server_callback_type call_back, const std::string& only_this_ip = "0.0.0.0");
		void					repose_client_req(int target_port, long handle_index, const std::string& repose_body);

		//---------------------------------------对外部分---------------------------------------
	public:
		static size_t			curl_writefunc(void *ptr, size_t size, size_t nmemb, void *stream);
		void					do_read();

		static void				http_server_global_callback(evhttp_request* req, void* arg);



	private:
		bool m_is_runing;
		boost::recursive_mutex	m_map_lock;
		long long				m_process_count;
		http_client_map			m_http_client_map;
		http_server_unit*		m_http_server_ptr;
		CURLM*					m_curlm;
	};
}
