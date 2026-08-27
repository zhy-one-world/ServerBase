#pragma once

//#define CURL_STATICLIB
#include <boost/thread.hpp>
#include <curl/curl.h>
#include "http_def.hpp"
namespace faith
{
	class http_client_unit
	{
		std::string						m_target_url;
		std::vector<std::string>		m_head_list;
		std::string						m_params;
		http_client_callback_type	m_call_back;
		e_http_request_type			m_request_type;
		long						m_index;
		bool						m_is_runing;
		bool						m_is_init_finish;
		bool						m_is_end;
		CURL*						m_curl;
		curl_slist*					m_clist;
		int						m_error_code;
		std::string						m_error_info;
		void*						m_buffer;
		int						m_buffer_size;
		size_t						m_data_size;

		bool						base_init();
	public:
		http_client_unit();
		~http_client_unit();
		bool						is_end() { return m_is_end; };
		bool						is_runing() { return m_is_runing; };
		void						set_is_runing() { m_is_runing = true; };
		CURL*						get_self_curl() { return m_curl; };
		void						clear_data();
		int							init(const long& index, const std::string& url, std::vector<std::string>& head_list, const std::string& params, http_client_callback_type call_back, e_http_request_type request_type);
		size_t						append_result_data(void* new_data_part, size_t new_data_size, size_t new_data_nmemb);
		void						do_end(bool is_stop = false);
		void						call_handler();

		void						set_error_code(int new_error_code) { m_error_code = new_error_code; };
		void						set_error_str(std::string new_error_info) { m_error_info = new_error_info; };
	};
}
