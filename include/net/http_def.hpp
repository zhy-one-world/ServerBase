#pragma once

#include <boost/function.hpp>
#include <map>
namespace faith
{
	enum e_http_request_type
	{
		e_http_request_type_get,
		e_http_request_type_post,
	};

	const int init_work_buffer_size = 16 * 1024;
	const int curl_error_base = 1000;


	typedef boost::function<void(
		int http_error_code,		// 由 HttpAccessorCode中 枚举值定义
		const std::string& http_error_info,
		const std::string& http_result			// html源文件，相当于ascii编码串。（unicode/ansi版本）
		)> http_client_callback_type;


	typedef std::map<std::string, std::string> http_found_headers_map;
	typedef http_found_headers_map::iterator http_found_headers_map_it;

	struct s_http_receive_info
	{
		std::string				m_client_ip;
		http_found_headers_map	m_found_headersmap;
		int						m_req_listen_port;
		long					m_req_handle_index;
		int						m_req_type;				//枚举见 evhttp_cmd_type
		std::string				m_req_body;

		//例 某客户端如此访问
		//post to www.url.come/test_path?param1=abc
		//此时
		//m_full_path = www.url.come/test_path?param1=abc
		//m_path = /test_path
		//m_query = param1=abc
		//m_fragment = 用"#"隔开的参数部分(不在上述事例中)
		std::string					m_full_path;
		std::string					m_path;
		std::string					m_query;
		std::string					m_fragment;

		void clear_data()
		{
			m_client_ip = "";
			m_found_headersmap.clear();
			m_req_listen_port = -1;
			m_req_handle_index = -1;
			m_req_type = 1;//EVHTTP_REQ_GET
			m_req_body = "";
			m_full_path = "";
			m_path = "";
			m_query = "";
			m_fragment = "";
		}

		s_http_receive_info()
		{
			clear_data();
		}

	};
	typedef boost::function<void(
		const s_http_receive_info& receive_info
		)> http_server_callback_type;
}


