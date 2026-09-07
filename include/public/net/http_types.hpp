#pragma once

#include <boost/function.hpp>
#include <map>
#include <string>
#include <vector>

#include "http_def.hpp"

namespace faith
{
	enum class http_scheme
	{
		http = 0,
		https = 1
	};

	struct ssl_client_options
	{
		bool verify_peer = false;
		bool verify_host = true;
		std::string ca_file;
		std::string cert_file;
		std::string key_file;
	};

	struct ssl_server_options
	{
		std::string cert_file;
		std::string key_file;
		std::string ca_file;
		bool require_client_cert = false;
	};

	struct http_request
	{
		std::string url;
		e_http_request_type method = e_http_request_type_get;
		std::vector<std::string> headers;
		std::string body;
		ssl_client_options ssl;
	};

	struct http_response
	{
		int error_code = 0;
		int http_status = 0;
		std::string error;
		std::string body;
	};

	typedef boost::function<void(const http_response&)> http_client_handler;

	struct http_listen_options
	{
		http_scheme scheme = http_scheme::http;
		std::string bind_ip = "0.0.0.0";
		int port = 0;
		ssl_server_options ssl;
	};

	struct http_inbound_request
	{
		std::string client_ip;
		std::map<std::string, std::string> headers;
		int listen_port = -1;
		long handle = -1;
		int method = 1;
		std::string body;
		std::string full_path;
		std::string path;
		std::string query;
		std::string fragment;
	};

	typedef boost::function<void(const http_inbound_request&)> http_inbound_handler;

	struct http_outbound_response
	{
		int status = 200;
		std::string reason = "OK";
		std::string body;
		std::vector<std::pair<std::string, std::string>> headers;
	};
}
