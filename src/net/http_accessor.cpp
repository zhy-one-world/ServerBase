/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:19
	file base:	http_accessor
	file ext:	cpp
	author:		lucifer~yu

	purpose:	Compatibility facade over http_client / http_server.
*********************************************************************/
#include "http_accessor.hpp"
#include "http_client.hpp"
#include "http_server.hpp"

#include <curl/curl.h>

namespace faith
{
	http_accessor::http_accessor()
	{
	}

	http_accessor::~http_accessor()
	{
		stop_listen();
		http_client::getInstance().shutdown();
	}

	bool http_accessor::init()
	{
		return http_client::getInstance().init();
	}

	void http_accessor::run()
	{
		http_client::getInstance().poll();
	}

	bool http_accessor::request_async(const http_request& request, http_client_handler handler)
	{
		return http_client::getInstance().request_async(request, handler);
	}

	bool http_accessor::listen(const http_listen_options& options, http_inbound_handler handler)
	{
		return http_server::getInstance().listen(options, handler);
	}

	void http_accessor::reply(long handle, const http_outbound_response& response)
	{
		http_server::getInstance().reply(handle, response);
	}

	void http_accessor::reply(long handle, int status, const std::string& body)
	{
		http_server::getInstance().reply(handle, status, body);
	}

	void http_accessor::stop_listen()
	{
		http_server::getInstance().stop();
	}

	unsigned int http_accessor::async_request(
		const xstring& url,
		std::vector<std::string>& head_list,
		const xstring& params,
		http_client_callback_type handler,
		e_http_request_type request_type)
	{
		http_request request;
		request.url = url;
		request.headers = head_list;
		request.body = params;
		request.method = request_type;
		request.ssl.verify_peer = false;
		request.ssl.verify_host = true;

		const bool ok = request_async(request, [handler](const http_response& response)
		{
			if (handler)
			{
				handler(response.error_code, response.error, response.body);
			}
		});
		return ok ? 0u : static_cast<unsigned int>(CURLE_FAILED_INIT);
	}

	bool http_accessor::start_listen(
		int listen_port,
		http_server_callback_type call_back,
		const std::string& only_this_ip)
	{
		http_listen_options options;
		options.scheme = http_scheme::http;
		options.bind_ip = only_this_ip.empty() ? "0.0.0.0" : only_this_ip;
		options.port = listen_port;

		return listen(options, [call_back](const http_inbound_request& inbound)
		{
			if (!call_back)
			{
				return;
			}
			s_http_receive_info info;
			info.m_client_ip = inbound.client_ip;
			info.m_found_headersmap = inbound.headers;
			info.m_req_listen_port = inbound.listen_port;
			info.m_req_handle_index = inbound.handle;
			info.m_req_type = inbound.method;
			info.m_req_body = inbound.body;
			info.m_full_path = inbound.full_path;
			info.m_path = inbound.path;
			info.m_query = inbound.query;
			info.m_fragment = inbound.fragment;
			call_back(info);
		});
	}

	void http_accessor::repose_client_req(int /*target_port*/, long handle_index, const std::string& repose_body)
	{
		reply(handle_index, 200, repose_body);
	}
}
