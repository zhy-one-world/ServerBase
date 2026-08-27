/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:11
	file base:	http_accessor
	file ext:	hpp
	author:		zhy
	
	purpose:	
*********************************************************************/
#ifndef _HTTP_ACCESSOR_H_
#define _HTTP_ACCESSOR_H_

#include <boost/function.hpp>
#include <boost/cstdint.hpp>
#include <vector>
#include "xchar.hpp"
#include "singleton.hpp"
#include "http_def.hpp"

#pragma comment(lib, "wldap32.lib")
//autolink end

namespace faith 
{
	class http_manager;

	class http_accessor : public singleton<http_accessor>
	{
		friend class singleton<http_accessor>;
	private:
		http_manager &	m_impl;
		http_accessor();
		~http_accessor();
	public:
		typedef boost::function<void(
			uint64_t request_uid,
			unsigned int http_error_code,		// 由 HttpAccessorCode中 枚举值定义
			const xstring& http_error_info,
			const xstring& http_result			// html源文件，相当于ascii编码串。（unicode/ansi版本）
			)> result_handler_type; 

		bool init();
		void run();
		// return value: request uid
		unsigned int async_request(const xstring& url, std::vector<std::string>& head_list, const xstring& params, http_client_callback_type handler, e_http_request_type request_type);
		bool start_listen(int listen_port, http_server_callback_type call_back, const std::string& only_this_ip = "0.0.0.0");
		void repose_client_req(int target_port, long handle_index, const std::string& repose_body);
	};

} // end of namespace faith 

#endif
