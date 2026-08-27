
#include "http_server_unit.h"
#include <boost/foreach.hpp>
#include "http_manager.h"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"wldap32.lib")
//#include "mem_pool.hpp"

namespace faith
{
	http_server_unit::http_server_unit()
	{
		clear_data();
	}


	http_server_unit::~http_server_unit()
	{

	}

	void http_server_unit::clear_data()
	{
		m_listen_port = -1;
		m_call_back = http_server_callback_type();
		m_event_base = nullptr;
		m_evutil_socket = INVALID_SOCKET;
		m_event = nullptr;
		m_listener = nullptr;
		m_http = nullptr;
		m_socket_bound_handle = nullptr;
		m_is_runing = false;
		m_is_end = false;
		m_http_receive_handle_map.clear();
		m_receive_handle_count = 0;
	}

	void http_server_unit::start_listen(int listen_port, http_server_callback_type call_back, const std::string& only_this_ip)
	{
		if (m_is_runing)
		{
			return;
		}

		m_listen_port = listen_port;
		m_call_back = call_back;


		struct sockaddr_in sin;
		memset(&sin, 0, sizeof(struct sockaddr_in));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(m_listen_port);

		m_event_base = event_base_new();
		if (m_event_base == nullptr)
		{
			end_listen();
			return;
		}

		m_http = evhttp_new(m_event_base);
		if (m_http == nullptr)
		{
			end_listen();
			return;
		}

		m_socket_bound_handle = evhttp_bind_socket_with_handle(m_http, only_this_ip.c_str(), m_listen_port);
		if (m_socket_bound_handle == nullptr)
		{
			end_listen();
			return;
		}

		//下面这个可以根据指定路径触发回调
		//evhttp_set_cb(m_http, "/dump", dump_request_cb, NULL);

		evhttp_set_gencb(m_http, &http_manager::http_server_global_callback, this);

		//boost::thread run_thread(&http_global_run);

		m_is_runing = true;
		boost::thread run_thread(&http_server_unit::heart_tick, this);
		//boost::thread run_thread(event_base_dispatch, m_event_base);

		//return event_base_dispatch(m_event_base);
	}

	void http_server_unit::heart_tick()
	{
		while (m_is_runing)
		{
			event_base_loop(m_event_base, EVLOOP_NONBLOCK);
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		}
	}
	void http_server_unit::end_listen()
	{
		//boost::recursive_mutex::scoped_lock server_lock(http_globel_client_lock);

		m_listen_port = -1;
		m_call_back = http_server_callback_type();

		for (http_receive_handle_map_it it = m_http_receive_handle_map.begin(); it != m_http_receive_handle_map.end();)
		{
			evhttp_request* temp_req = it->second;
			it = remove_receive_handle(it);
			if (temp_req == nullptr)
			{
				continue;
			}
			send_repose(temp_req, "server closed");
		}

		if (m_evutil_socket == INVALID_SOCKET)
		{
			evutil_closesocket(m_evutil_socket);
		}

		if (m_event != nullptr)
		{
			event_free(m_event);
		}

		if (m_listener != nullptr)
		{
			evconnlistener_free(m_listener);
		}

		if (m_socket_bound_handle)
		{
			evhttp_del_accept_socket(m_http, m_socket_bound_handle);
		}

		if (m_http != nullptr)
		{
			evhttp_free(m_http);
		}

		if (m_event_base == nullptr)
		{
			event_base_free(m_event_base);
		}


		clear_data();

		m_is_runing = false;
		m_is_end = true;
	}

	void http_server_unit::do_callback(const s_http_receive_info& receive_info)
	{
		m_call_back(receive_info);
	}

	void http_server_unit::repose_client(long client_handle_index, const std::string& repose_body)
	{
		http_receive_handle_map_it it = m_http_receive_handle_map.find(client_handle_index);
		if (it == m_http_receive_handle_map.end())
		{
			return;
		}
		send_repose(it->second, repose_body);
		remove_receive_handle(it);
	}

	long http_server_unit::add_receive_handle(evhttp_request* new_handle)
	{
		if (new_handle == nullptr)
		{
			return -1;
		}
		m_http_receive_handle_map.insert({ m_receive_handle_count, new_handle });
		return m_receive_handle_count++;
	}

	void http_server_unit::send_repose(evhttp_request* target_req, const std::string& repose_body)
	{
		if (target_req == nullptr)
		{
			return;
		}
		evbuffer* temp_buffer = evbuffer_new();
		if (temp_buffer == nullptr)
		{
			return;
		}
		evbuffer_add_printf(temp_buffer, repose_body.c_str());
		evhttp_send_reply(target_req, HTTP_OK, "Client", temp_buffer);
		evbuffer_free(temp_buffer);
	}

	http_receive_handle_map_it http_server_unit::remove_receive_handle(http_receive_handle_map_it target_it)
	{
		if (target_it == m_http_receive_handle_map.end())
		{
			return target_it;
		}
		//std::cout << __FUNCTION__ << " " << __LINE__ << " index " << target_it->first << std::endl;
		return m_http_receive_handle_map.erase(target_it);
	}
}
