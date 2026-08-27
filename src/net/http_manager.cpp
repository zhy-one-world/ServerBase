
#include "http_manager.h"
#include <iostream>

namespace faith
{
	//void http_global_run()
	//{
	//	while (http_manager::get_instance().is_runing())
	//	{
	//		http_manager::get_instance().http_manager_do_run();
	//		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	//	}
	//}

	http_manager::http_manager()
	{
		m_http_server_ptr = nullptr;
		m_curlm = nullptr;
		m_process_count = 0;
		m_http_client_map.clear();
	}


	http_manager::~http_manager()
	{

	}

	void http_manager::clear_data()
	{
		m_process_count = 0;
		stop_client_run();
		stop_server_run();
		curl_multi_cleanup(m_curlm);
		curl_global_cleanup();
	}

	bool http_manager::init()
	{
		if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
		{
			return false;
		}
		m_curlm = curl_multi_init();
		if (m_curlm == nullptr)
		{
			return false;
		}

		long timeout = 100;
		curl_multi_timeout(m_curlm, &timeout);

		set_runing(true);
		//boost::thread run_thread(&http_global_run);
		//run_thread.join();

		return true;
	}

	void http_manager::run()
	{
		boost::recursive_mutex::scoped_lock lock(m_map_lock);
		if (m_http_client_map.empty())
		{
			return;
		}
		for (http_client_map_it it = m_http_client_map.begin(); it != m_http_client_map.end();)
		{
			http_client_unit* temp_ptr = it->second;
			if (temp_ptr == nullptr)
			{
				it = m_http_client_map.erase(it);
				continue;
			}

			if (temp_ptr->is_end())
			{

				it = m_http_client_map.erase(it);
				delete temp_ptr;
				continue;
			}

			if (!temp_ptr->is_runing())
			{
				temp_ptr->set_is_runing();
				int error_code = curl_multi_add_handle(m_curlm, temp_ptr->get_self_curl());
				if (error_code != CURLM_OK)
				{
					temp_ptr->do_end();
				}
			}
			it++;
		}

		fd_set fdread;
		fd_set fdwrite;
		fd_set fdexcep;

		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);

		// 获取文件描述符集fd_set
		int maxfd = -1;
		CURLMcode mc = curl_multi_fdset(m_curlm, &fdread, &fdwrite, &fdexcep, &maxfd);
		if (mc != CURLM_OK)
		{
			return;
			//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("curl_multi_fdset:"<<mc));
		}

		if (maxfd == -1)
		{
			////FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("curl_multi_fdset maxfd == -1,sleep(10)"));
			//boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}
		else
		{
			// select超时时间
			struct timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 1000;

			//FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("select before:" << maxfd));
			int rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
			//FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("select after:" << maxfd << "," <<  rc));
			if (rc == -1)
			{
				/* select error */
				//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("select error : -1"));
			}
		}
		//执行
		int running_handles = 0;
		while (curl_multi_perform(m_curlm, &running_handles) == CURLM_CALL_MULTI_PERFORM);

		do_read();
	}

	void http_manager::stop_client_run()
	{
		boost::recursive_mutex::scoped_lock lock(m_map_lock);
		for (http_client_map_it it = m_http_client_map.begin(); it != m_http_client_map.end(); it++)
		{
			it->second->do_end(true);
			delete it->second;
			it->second = nullptr;
		}
		m_http_client_map.clear();
	}

	void http_manager::stop_server_run()
	{
		if (m_http_server_ptr)
		{
			m_http_server_ptr->end_listen();
			delete m_http_server_ptr;
			m_http_server_ptr = nullptr;
		}
	}

	long http_manager::async_request(
		const std::string& url,
		std::vector<std::string>& head_list,
		const std::string& params,
		http_client_callback_type handler,
		e_http_request_type request_type)
	{
		boost::recursive_mutex::scoped_lock lock(m_map_lock);

		http_client_unit* temp_client_unit_ptr = new http_client_unit;
		if (temp_client_unit_ptr == nullptr)
		{
			return CURLE_FAILED_INIT;
		}
		int res = temp_client_unit_ptr->init(m_process_count, url, head_list, params, handler, request_type);
		if (res != CURLE_OK)
		{
			delete temp_client_unit_ptr;
			return res;
		}

		m_http_client_map.insert({ m_process_count++, temp_client_unit_ptr });
		return res;
	}

	bool http_manager::start_listen(int listen_port, http_server_callback_type call_back, const std::string& only_this_ip)
	{
		if (m_http_server_ptr)
		{
			return false;
		}
		m_http_server_ptr = new http_server_unit;
		m_http_server_ptr->start_listen(listen_port, call_back, only_this_ip);

		if (!m_http_server_ptr->is_runing())
		{
			delete m_http_server_ptr;
			m_http_server_ptr = nullptr;
			return false;
		}
		return true;
	}

	void http_manager::repose_client_req(int target_port, long handle_index, const std::string& repose_body)
	{
		if (m_http_server_ptr)
		{
			m_http_server_ptr->repose_client(handle_index, repose_body);
		}
	}

	size_t http_manager::curl_writefunc(void *ptr, size_t size, size_t nmemb, void *stream)
	{
		if (ptr == nullptr
			|| stream == nullptr)
		{
			return 0;
		}

		http_client_unit* temp_client_unit = (http_client_unit*)stream;
		if (temp_client_unit == nullptr)
		{
			return 0;
		}
		return temp_client_unit->append_result_data(ptr, size, nmemb);
	}

	void http_manager::do_read()
	{
		if (m_curlm == nullptr)
		{
			return;
		}

		int msgs_in_queue = 1;
		while (msgs_in_queue > 0)
		{
			CURLMsg* msg = curl_multi_info_read(m_curlm, &msgs_in_queue);
			if (msg == NULL)
			{
				////FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("curl_multi_info_read:msg == null"));
				break;
			}

			if (msg->msg != CURLMSG_DONE)
			{
				//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("do_read not done:"+msg->msg));
				continue;
			}

			http_client_unit* doing_unit = nullptr;

			CURLcode ccode = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &doing_unit);
			if (ccode != CURLE_OK)
			{
				//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("do_read faith_curl_easy_getinfo failed:"+ccode));
				continue;
			}

			if (doing_unit == nullptr)
			{
				//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("do_read get_work:null"));
				continue;
			}

			std::ostringstream buf;
			buf << curl_easy_strerror(msg->data.result);
			doing_unit->set_error_code(msg->data.result);
			doing_unit->set_error_str(buf.str());
			doing_unit->do_end();
		}
	}

	void socket_read_cb(bufferevent *bev, void *arg)
	{
		char msg[4096];

		size_t len = bufferevent_read(bev, msg, sizeof(msg) - 1);

		//msg[len] = '\0';
		//printf("server read the data %s\n", msg);



		bufferevent_read(bev, msg, sizeof(msg) - 1);

		msg[len] = '\0';
		printf("server read the data %s\n", msg);
	}

	void socket_write_cb(bufferevent *bev, void *arg)
	{
		char reply[] = "I has read your data";
		bufferevent_write(bev, reply, strlen(reply));

	}


	void socket_event_cb(bufferevent *bev, short events, void *arg)
	{
		if (events & BEV_EVENT_EOF)
			printf("connection closed\n");
		else if (events & BEV_EVENT_ERROR)
			printf("some other error\n");

		//这将自动close套接字和free读写缓冲区  
		bufferevent_free(bev);
	}

	void http_manager::http_server_global_callback(evhttp_request* req, void* arg)
	{
		//boost::recursive_mutex::scoped_lock server_lock(http_globel_server_lock);

		if (req == nullptr)
		{
			return;
		}

		http_server_unit* server_unit = (http_server_unit*)arg;
		if (server_unit == nullptr)
		{
			return;
		}
		long receiver_handle_index = server_unit->add_receive_handle(req);

		s_http_receive_info callback_param;
		callback_param.m_req_listen_port = server_unit->get_listen_port();
		callback_param.m_req_handle_index = receiver_handle_index;
		callback_param.m_req_type = evhttp_request_get_command(req);
		const char* full_path = evhttp_request_get_uri(req);
		if (full_path != nullptr)
		{
			callback_param.m_full_path = full_path;
		}
		evhttp_uri* decoded = evhttp_uri_parse(full_path);
		if (decoded == nullptr)
		{
			printf("It's not a good URI. Sending BADREQUEST\n");
			evhttp_send_error(req, HTTP_BADREQUEST, 0);
			return;
		}

		const char* client_host = evhttp_uri_get_host(decoded);
		if (client_host != nullptr)
		{
			callback_param.m_client_ip = client_host;
		}
		else
		{
			callback_param.m_client_ip = req->remote_host;
		}

		const char* path = evhttp_uri_get_path(decoded);
		if (path != nullptr)
		{
			callback_param.m_path = path;
		}

		const char* query = evhttp_uri_get_query(decoded);
		if (query != nullptr)
		{
			callback_param.m_query = query;
		}

		const char* fragment = evhttp_uri_get_fragment(decoded);
		if (fragment != nullptr)
		{
			callback_param.m_fragment = fragment;
		}

		evbuffer * temp_buf = evhttp_request_get_input_buffer(req);
		size_t buf_size = evbuffer_get_length(temp_buf);
		char* temp_body_buf = new char[buf_size + 1];
		memset(temp_body_buf, 0, buf_size + 1);
		memcpy(temp_body_buf, evbuffer_pullup(temp_buf, -1), buf_size);
		callback_param.m_req_body = temp_body_buf;
		delete temp_body_buf;


		//evbuffer* evb = evbuffer_new();
		//if (evb == nullptr)
		//{
		//	return;
		//}
		//evbuffer_add_printf(evb, "dasdasdasdasdasdasd");
		//evhttp_send_reply(req, HTTP_OK, "Client", evb);



		evkeyvalq* temp_evkeyvalq = evhttp_request_get_input_headers(req);
		if (temp_evkeyvalq == nullptr)
		{
			return;
		}
		evkeyval* temp_header_it = temp_evkeyvalq->tqh_first;
		if (temp_header_it != nullptr)
		{
			for (int i = 0; i < 99; ++i)
			{
				callback_param.m_found_headersmap.insert({ temp_header_it->key, temp_header_it->value });
				temp_header_it = temp_header_it->next.tqe_next;
				if (temp_header_it == nullptr)
				{
					break;
				}
			}
		}

		evhttp_uri_free(decoded);

		server_unit->do_callback(callback_param);
	}
}