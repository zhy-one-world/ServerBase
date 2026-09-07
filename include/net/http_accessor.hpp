/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:11
	file base:	http_accessor
	file ext:	hpp
	author:		zhy

	purpose:	Compatibility facade over http_client / http_server.
*********************************************************************/
#ifndef _HTTP_ACCESSOR_H_
#define _HTTP_ACCESSOR_H_

#include <boost/function.hpp>
#include <boost/cstdint.hpp>
#include <vector>
#include "xchar.hpp"
#include "singleton.hpp"
#include "http_def.hpp"
#include "http_types.hpp"

#pragma comment(lib, "wldap32.lib")

namespace faith
{
	class http_accessor : public singleton<http_accessor>
	{
		friend class singleton<http_accessor>;
	private:
		http_accessor();
		~http_accessor();
	public:
		typedef boost::function<void(
			uint64_t request_uid,
			unsigned int http_error_code,
			const xstring& http_error_info,
			const xstring& http_result
			)> result_handler_type;

		bool init();
		void run();

		bool request_async(const http_request& request, http_client_handler handler);
		bool listen(const http_listen_options& options, http_inbound_handler handler);
		void reply(long handle, const http_outbound_response& response);
		void reply(long handle, int status, const std::string& body);
		void stop_listen();

		unsigned int async_request(const xstring& url, std::vector<std::string>& head_list, const xstring& params, http_client_callback_type handler, e_http_request_type request_type);
		bool start_listen(int listen_port, http_server_callback_type call_back, const std::string& only_this_ip = "0.0.0.0");
		void repose_client_req(int target_port, long handle_index, const std::string& repose_body);
	};

} // end of namespace faith

#endif
