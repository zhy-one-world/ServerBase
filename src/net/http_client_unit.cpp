#include <boost/foreach.hpp>
#include "http_client_unit.h"
#include "http_manager.h"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"wldap32.lib")
//#include "mem_pool.hpp"
namespace faith
{
	http_client_unit::http_client_unit()
	{
		clear_data();
	}


	http_client_unit::~http_client_unit()
	{

	}

	bool http_client_unit::base_init()
	{
		m_curl = curl_easy_init();

		m_buffer_size = init_work_buffer_size;
		m_buffer = malloc(m_buffer_size);
		if (m_buffer == nullptr)
		{
			return false;
		}

		curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &http_manager::curl_writefunc);
		curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(m_curl, CURLOPT_PRIVATE, this);
		long proto_mask = CURLPROTO_HTTP | CURLPROTO_HTTPS;
		curl_easy_setopt(m_curl, CURLOPT_PROTOCOLS, proto_mask);
		curl_easy_setopt(m_curl, CURLOPT_REDIR_PROTOCOLS, proto_mask);
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 1);

		//默认为16384，会导致下载速度慢
		curl_easy_setopt(m_curl, CURLOPT_BUFFERSIZE, (512 * 1024));

		//限速，暂时不限速
		//curl_easy_setopt(m_curl, CURLOPT_MAX_RECV_SPEED_LARGE, 100000);

		return true;
	}

	void http_client_unit::clear_data()
	{
		m_curl = nullptr;
		m_clist = nullptr;
		m_buffer = nullptr;

		m_target_url = "";
		m_head_list.clear();
		m_params = "";
		m_call_back = http_client_callback_type();
		m_request_type = e_http_request_type_get;
		m_is_runing = false;
		m_is_end = false;
		m_is_init_finish = false;
		m_buffer_size = 0;
		m_data_size = 0;
		m_error_code = 0;
		m_error_info = "";
	}

	int http_client_unit::init(
		const long& index,
		const std::string& url,
		std::vector<std::string>& head_list,
		const std::string& params,
		http_client_callback_type call_back,
		e_http_request_type request_type)
	{
		int return_value = CURLE_FAILED_INIT;

		clear_data();

		m_index = index;
		m_target_url = url;
		m_head_list = head_list;
		m_params = params;
		m_call_back = call_back;
		m_request_type = request_type;

		if (!base_init())
		{
			return return_value;
		}

		if (m_is_init_finish)
		{
			return return_value;
		}

		if (m_request_type == e_http_request_type_get)
		{
			return_value = curl_easy_setopt(m_curl, CURLOPT_URL, (m_target_url + m_params).c_str());
		}
		else if (m_request_type == e_http_request_type_post)
		{
			return_value = curl_easy_setopt(m_curl, CURLOPT_URL, m_target_url.c_str());
			if (return_value != CURLE_OK)
			{
				return return_value;
			}

			return_value = curl_easy_setopt(m_curl, CURLOPT_POST, 1);
			if (return_value != CURLE_OK)
			{
				return return_value;
			}

			return_value = curl_easy_setopt(m_curl, CURLOPT_COPYPOSTFIELDS, m_params.c_str());
			if (return_value != CURLE_OK)
			{
				return return_value;
			}
		}

		for (int i = 0; i < m_head_list.size(); ++i)
		{
			m_clist = curl_slist_append(m_clist, m_head_list[i].c_str());
			return_value = curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_clist);
			if (return_value != CURLE_OK)
			{
				return return_value;
			}
		}


		m_is_init_finish = true;
		return return_value;
	}

	size_t http_client_unit::append_result_data(void* new_data_part, size_t new_data_size, size_t new_data_nmemb)
	{
		size_t length = new_data_size * new_data_nmemb;
		if (length)
		{
			size_t new_data_size = m_data_size + length;
			if (new_data_size <= m_buffer_size)
			{
				memcpy(static_cast<char *>(m_buffer) + m_data_size, new_data_part, length);
				m_data_size = new_data_size;
			}
			else
			{
				int new_buffer_size = m_buffer_size * 2;
				while (new_buffer_size < new_data_size)
				{
					new_buffer_size *= 2;
				}
				void* new_buffer = malloc(new_buffer_size);
				if (new_buffer == nullptr)
				{
					return 0;
				}

				if (m_buffer == nullptr)
				{
					memcpy(new_buffer, new_data_part, length);
				}
				else
				{
					memcpy(new_buffer, m_buffer, m_data_size);
					memcpy(static_cast<char *>(new_buffer) + m_data_size, new_data_part, length);
					free(m_buffer);
				}

				m_buffer = new_buffer;
				m_buffer_size = new_buffer_size;
				m_data_size = new_data_size;
			}
		}

		char* abc = (char*)m_buffer;
		return length;
	}

	void http_client_unit::do_end(bool is_stop)
	{
		m_is_end = true;
		m_is_runing = false;
		if (!is_stop)
		{
			call_handler();
		}

		if (m_curl)
		{
			curl_easy_cleanup(m_curl);
		}

		if (m_clist)
		{
			curl_slist_free_all(m_clist);
		}

		if (m_buffer)
		{
			free(m_buffer);
		}
	}

	void http_client_unit::call_handler()
	{
		std::string http_result = "";
		if (m_buffer && m_data_size > 0)
		{
			http_result = std::string(static_cast<const char *>(m_buffer), m_data_size);
		}
		m_call_back(m_error_code, m_error_info, http_result);
	}
}