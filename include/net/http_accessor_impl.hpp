/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:08
	file base:	http_accessor_impl
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _HTTP_ACCESSOR_IMPL_H_
#define _HTTP_ACCESSOR_IMPL_H_

#include "asio.hpp"
#include <boost/thread.hpp>
#define CURL_STATICLIB
#include <curl/curl.h>
#include "http_accessor.hpp"
#include "unique_id_generator.hpp"

namespace faith 
{
	namespace net 
	{
		// 1, it is multi-thread safety
		// 2, it is moonlight box friendly class
		// 3, support both http & https request, and mixed use
		// 4, support multiple simultaneous request
		class http_accessor_impl
		{
			enum
			{
				e_init_work_buffer_size			=   16*1024,	//初始工作缓存大小(字节)
				e_max_simultaneous_requests		=	5000,	//最大并发请求数,并发请求中超过并发连接的部分会进入等待队列
			};

			struct request_packet
			{
				xstring											url;
				std::vector<std::string>						head_list;
				xstring											params;
				http_accessor_code::e_http_request_type			request_type;
			};

			struct work_item
			{
				unsigned int									request_id;
				http_accessor::result_handler_type				handler;
				CURL *											curl;
				void *											buffer;
				unsigned int									buffer_size;
				unsigned int									data_size;
				unsigned int									error_code;
				xstring											error_info;	
                struct curl_slist*                              chunk;
			};

			typedef std::list<work_item *>						work_list;

		public:
			http_accessor_impl();
			~http_accessor_impl();
		public:
			bool												init();
			void												release();
			void												thread_func_send();
			unsigned int										async_request(const xstring& url,std::vector<std::string>& head_list,const xstring& params,http_accessor::result_handler_type handler,http_accessor_code::e_http_request_type request_type);
		private:
			unsigned int										mlb_async_request(const request_packet& packet, http_accessor::result_handler_type handler);
			void												post_handler(work_item * item);
			void												call_handler(work_item * item);
			static size_t										curl_writefunc(void *ptr, size_t size, size_t nmemb, void *stream);
	        void                                                set_work_item(work_item* work,const request_packet& packet);
			void                                                easy_error_handler(work_item* p_work_item);
			void                                                multi_error_handler(work_item* p_work_item);
			int                                                 curl_multi_select(CURLM * curl_m);
			void                                                do_read();
		    work_item*                                          new_work_item();
			void                                                destroy_work_item(work_item* p_work);
		private:
			work_list                                           m_ready_work_list;          //准备好的工作对象列表
			boost::asio::detail::mutex							m_impl_mutex;
			::faith::unique_id_generator<unsigned int>			m_id_generator;
			bool												m_running;
			CURLM *												m_curlm;
			boost::recursive_mutex								m_mutex;				// for thread safe
		};
	}
}

#endif
