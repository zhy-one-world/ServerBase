#pragma once

//#define CURL_STATICLIB
#include <boost/thread.hpp>
#include <event.h>
#include <event2/util.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/listener.h> 
#include "http_def.hpp"

namespace faith
{

	typedef std::map<long, evhttp_request*> http_receive_handle_map;
	typedef http_receive_handle_map::iterator http_receive_handle_map_it;




	class http_server_unit
	{
		int							m_listen_port;
		http_server_callback_type	m_call_back;
		event_base*					m_event_base;
		evutil_socket_t				m_evutil_socket;
		event*						m_event;
		evconnlistener*				m_listener;
		evhttp*						m_http;
		evhttp_bound_socket*		m_socket_bound_handle;
		bool						m_is_runing;
		bool						m_is_end;
		http_receive_handle_map		m_http_receive_handle_map;
		long						m_receive_handle_count;
	public:
		http_server_unit();
		~http_server_unit();
		void						clear_data();
		void						heart_tick();
		int							get_listen_port() { return m_listen_port; };
		bool						is_end() { return m_is_end; };
		bool						is_runing() { return m_is_runing; };
		void						start_listen(int listen_port, http_server_callback_type call_back, const std::string& only_this_ip);
		void						end_listen();
		void						do_callback(const s_http_receive_info& receive_info);
		void						repose_client(long client_handle_index, const std::string& repose_body);

		long						add_receive_handle(evhttp_request* new_handle);

	private:
		void						send_repose(evhttp_request* target_req, const std::string& repose_body);
		http_receive_handle_map_it	remove_receive_handle(http_receive_handle_map_it target_it);
	};
}
