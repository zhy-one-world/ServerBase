#pragma once

#include <boost/thread.hpp>
#include <map>
#include <memory>

#include <curl/curl.h>

#include "http_types.hpp"
#include "singleton.hpp"

namespace faith
{
	class http_client : public singleton<http_client>
	{
		friend class singleton<http_client>;

	public:
		bool init();
		void poll();
		void shutdown();

		bool request_async(const http_request& request, http_client_handler handler);

	private:
		http_client() = default;
		~http_client();

		struct pending_request
		{
			http_request request;
			http_client_handler handler;
			CURL* easy = nullptr;
			curl_slist* headers = nullptr;
			std::string response_body;
			char error_buf[256] = { 0 };
		};

		static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);
		bool configure_easy(pending_request& item);
		void complete_finished();

		bool m_inited = false;
		CURLM* m_multi = nullptr;
		boost::recursive_mutex m_mutex;
		long long m_next_id = 1;
		std::map<long long, std::unique_ptr<pending_request>> m_pending;
	};
}
