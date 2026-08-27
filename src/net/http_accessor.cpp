/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:19
	file base:	http_accessor
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
//#ifdef FIAITHSERVER
#include "http_accessor.hpp"
#include "http_manager.h"

namespace faith
{
	http_accessor::http_accessor()
		:m_impl(*new http_manager())
	{
	}

	http_accessor::~http_accessor()
	{
		delete & m_impl;
	}

	bool http_accessor::init()
	{
		return m_impl.init();
	}

	void http_accessor::run()
	{
		m_impl.run();
	}
	unsigned int http_accessor::async_request(const xstring& url, std::vector<std::string>& head_list, const xstring& params, http_client_callback_type handler, e_http_request_type request_type)
	{
		return m_impl.async_request(url, head_list, params, handler, request_type);
	}
	bool http_accessor::start_listen(int listen_port, http_server_callback_type call_back, const std::string& only_this_ip)
	{
		return m_impl.start_listen(listen_port, call_back, only_this_ip);
	}
	void http_accessor::repose_client_req(int target_port, long handle_index, const std::string& repose_body)
	{
		m_impl.repose_client_req(target_port, handle_index, repose_body);
	}
}

//#endif