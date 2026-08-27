/********************************************************************
created:	2014/05/04
created:	4:5:2014   19:20
file base:	http_accessor_impl
file ext:	cpp
author:		lucifer~yu

purpose:	
*********************************************************************/
//#ifdef FIAITHSERVER

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include "http_accessor_impl.hpp"
#include "mem_pool.hpp"
#include "scheduler.hpp"
#include "mlb.hpp"

namespace faith
{
	namespace net
	{
		static const int CURLM_ERROR_BASE = 1000;

		http_accessor_impl::http_accessor_impl():
		m_running(false), m_id_generator(http_accessor::invalid_request_uid),m_curlm(nullptr)
		{
			m_id_generator.set_max_count(e_max_simultaneous_requests);
		}

		http_accessor_impl::work_item* http_accessor_impl::new_work_item()
		{
			work_item* item = new work_item();
			item->buffer_size = e_init_work_buffer_size;
			item->buffer = common::mem_pool::getInstance().alloc(item->buffer_size);
			if (item->buffer == NULL)
			{
				delete item;
				return NULL;
			}
			item->curl = faith_curl_easy_init();
			if (item->curl == NULL)
			{
				delete item;
				return NULL;
			}
			item->chunk = NULL;

			faith_curl_easy_setopt(item->curl,CURLOPT_WRITEFUNCTION,&http_accessor_impl::curl_writefunc);
			faith_curl_easy_setopt(item->curl,CURLOPT_WRITEDATA,item);
			faith_curl_easy_setopt(item->curl,CURLOPT_PRIVATE,item);
			long proto_mask = CURLPROTO_HTTP|CURLPROTO_HTTPS;
			faith_curl_easy_setopt(item->curl,CURLOPT_PROTOCOLS,proto_mask);
			faith_curl_easy_setopt(item->curl,CURLOPT_REDIR_PROTOCOLS,proto_mask);
			faith_curl_easy_setopt(item->curl,CURLOPT_SSL_VERIFYPEER, false);
			faith_curl_easy_setopt(item->curl,CURLOPT_SSL_VERIFYHOST, 1);							
			// 				faith_curl_easy_setopt(item->curl,CURLOPT_VERBOSE,1);
			// 				faith_curl_easy_setopt(item->curl,CURLOPT_HEADER,1);
			return item;
		}

		void http_accessor_impl::destroy_work_item(work_item* p_work)
		{
			if(p_work == NULL)
				return;

			m_id_generator.return_id(p_work->request_id);

			if (p_work->curl)
			{
				faith_curl_easy_cleanup(p_work->curl);
				p_work->curl = nullptr;
			}
			if (p_work->buffer)
			{
				common::mem_pool::getInstance().free(p_work->buffer, p_work->buffer_size);
				p_work->buffer = nullptr;
			}
			if (p_work->chunk)
			{
				faith_curl_slist_free_all(p_work->chunk);
				p_work->chunk = nullptr;
			}
			delete p_work;
			p_work = nullptr;
		}

		http_accessor_impl::~http_accessor_impl()
		{
			m_ready_work_list.clear();
		}

		bool http_accessor_impl::init()
		{
			if(m_running)
			{
				return false;
			}

			if(faith_curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
			{
				return false;
			};

			m_curlm = faith_curl_multi_init();
			if(m_curlm == NULL)
				return false;

			faith_curl_easy_setopt(m_curlm,CURLOPT_NOSIGNAL,1);
			long timeout = 100;
			faith_curl_multi_timeout(m_curlm,&timeout);
			m_running = true;
			return true;
		}

		void http_accessor_impl::release()
		{
			if(m_running)
			{
				m_running = false;
				m_ready_work_list.clear();
				faith_curl_multi_cleanup(m_curlm);
				faith_curl_global_cleanup();
			}
		}

		namespace
		{
			extern xchar sz_async_request[] = _XTEXT("HTTPAccessor::async_request");
		}

		unsigned int http_accessor_impl::async_request(const xstring& url,std::vector<std::string>& head_list,const xstring& params,http_accessor::result_handler_type handler,http_accessor_code::e_http_request_type request_type)
		{
			////FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("async_request")); 
			request_packet packet;
			packet.url = url;
			packet.params = params;
			packet.request_type = request_type;
			packet.head_list = head_list;

			//common::mlb_helper<sz_async_request,unsigned int (const request_packet&)> helper(
			//	boost::bind(&http_accessor_impl::mlb_async_request,this,_1,handler)
			//	);
			return mlb_async_request(packet, handler);
		}

		unsigned int http_accessor_impl::mlb_async_request(const request_packet& packet, http_accessor::result_handler_type handler)
		{
			unsigned int id = m_id_generator.get_id();
			if(id == http_accessor::invalid_request_uid)
			{
				//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("invalid_request_uid"));
				handler(0, CURLE_FAILED_INIT, "init id fail", "init id fail");
				return -1;
			}
		
			work_item* item = new_work_item();
			if( item == NULL)
			{
				//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("no work task,refuse"));
				m_id_generator.return_id(id);
				handler(0, CURLE_FAILED_INIT, "init ptr fail", "init ptr fail");
				return -1;
			}

			item->request_id = id;
			item->handler = handler;
		
			set_work_item(item,packet);
			if (item->error_code == CURLE_OK) {
				boost::recursive_mutex::scoped_lock server_lock(m_mutex);
				m_ready_work_list.push_back(item);
			}
			else
			{
				easy_error_handler(item);
				//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("set work attr failed:"));
			}

		//	//FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("async_request end"));
			return id;
		}

		void http_accessor_impl::set_work_item(work_item* work,const request_packet& packet)
		{
			work->data_size = 0;			
			work->error_code = CURLE_OK;  //默认值

			if(packet.request_type == http_accessor_code::e_request_type_get)
			{
				work->error_code = faith_curl_easy_setopt(work->curl,CURLOPT_URL,(packet.url + packet.params).c_str());
				if (work->error_code != CURLE_OK)
				{
					return;
				}
			}
			else if(packet.request_type == http_accessor_code::e_request_type_post)
			{
				work->error_code = faith_curl_easy_setopt(work->curl,CURLOPT_URL,packet.url.c_str());
			//	//FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("url:" << packet.url.c_str()));

				if(work->error_code != CURLE_OK)
				{
					return;
				}

				work->error_code = faith_curl_easy_setopt(work->curl,CURLOPT_POST,1);
				if(work->error_code != CURLE_OK)
				{
					return;
				}

				work->error_code = faith_curl_easy_setopt(work->curl,CURLOPT_COPYPOSTFIELDS,packet.params.c_str());
			//	//FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("para:" << packet.params.c_str()));
				if(work->error_code != CURLE_OK)
				{
					return;
				}
			}
			else
			{
				work->error_code = CURLE_UNSUPPORTED_PROTOCOL;
				return;
			}

			const std::vector<std::string>& head_list = packet.head_list;
			size_t hl_size = head_list.size();
			if(hl_size > 0)
			{
				// 设置http头
				for(int i = 0;i < hl_size;i++)
				{
					work->chunk = faith_curl_slist_append(work->chunk,head_list[i].c_str());
				}

				work->error_code = faith_curl_easy_setopt(work->curl,CURLOPT_HTTPHEADER,work->chunk);
				if(work->error_code != CURLE_OK)
				{
					return;
				}
			}
		}

		void http_accessor_impl::thread_func_send()
		{
			////FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT(" thread_func_send"));
			//while(m_running)
			{
				{
					boost::recursive_mutex::scoped_lock server_lock(m_mutex);
					for(int i = 0; !m_ready_work_list.empty() && i < 10; ++i)
					{
						work_item* work = m_ready_work_list.front();
						m_ready_work_list.pop_front();
						////FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("pop work"<< work->request_id));
						if(work != NULL)
						{
							work->error_code = faith_curl_multi_add_handle(m_curlm,work->curl);
							if(work->error_code != CURLM_OK)
							{
								multi_error_handler(work);
								//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("curl_multi_add_handle failed:"));
							}
// 							else
// 							{
// 								//FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("curl_multi_add_handle succ:"));
// 							}
						}
						else 
						{
							//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("pop,but no work task"));
						}
					}
				}

				fd_set fdread;
				fd_set fdwrite;
				fd_set fdexcep;

				FD_ZERO(&fdread);
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdexcep);

				// 获取文件描述符集fd_set
				int maxfd = -1;
				CURLMcode mc = faith_curl_multi_fdset(m_curlm, &fdread, &fdwrite, &fdexcep, &maxfd);
				if( mc != CURLM_OK)
				{					
					//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("curl_multi_fdset:"<<mc));
				}

				if(maxfd == -1)
				{	
					////FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("curl_multi_fdset maxfd == -1,sleep(10)"));
					boost::this_thread::sleep(boost::posix_time::milliseconds(10));
				}
				else{
					// select超时时间
					struct timeval timeout;
					timeout.tv_sec = 0;		
					timeout.tv_usec = 100000;
					
					////FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("select before:" << maxfd));
					int rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
					////FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("select after:" << maxfd << "," <<  rc));
					if(rc == -1)
					{
						/* select error */
						//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("select error : -1"));
					}
				}
				//执行
				int running_handles = 0;
				while(faith_curl_multi_perform(m_curlm,&running_handles) == CURLM_CALL_MULTI_PERFORM);
				do_read();
			}
		}

		void http_accessor_impl::do_read()
		{
			// 获得结果
			int msgs_in_queue = 1;
			while(msgs_in_queue > 0)
			{
				CURLMsg* msg = faith_curl_multi_info_read(m_curlm,&msgs_in_queue);
				if(msg == NULL)
				{
					////FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("curl_multi_info_read:msg == null"));
					break;
				}

				if(msg->msg != CURLMSG_DONE)
				{
					//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("do_read not done:"+msg->msg));
					continue;
				}

				work_item* get_work = NULL;

				CURLcode ccode = faith_curl_easy_getinfo(msg->easy_handle,CURLINFO_PRIVATE,&get_work);
				if( ccode != CURLE_OK)
				{
					//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("do_read faith_curl_easy_getinfo failed:"+ccode));
					continue;
				}

				if(get_work == NULL)
				{
					//FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("do_read get_work:null"));
					continue;
				}

				xostringstream buf;
				buf << faith_curl_easy_strerror(msg->data.result);

				get_work->error_code = msg->data.result;
				get_work->error_info = buf.str();
				post_handler(get_work);
			}
		}

		size_t http_accessor_impl::curl_writefunc(void *ptr, size_t size, size_t nmemb, void *stream)
		{
			if(ptr == NULL || stream == NULL)
				return 0;

			work_item* item = static_cast<work_item*>(stream);

			size_t length = size * nmemb;
			if(length)
			{
				size_t new_data_size = item->data_size + length;
				if(new_data_size <= item->buffer_size)
				{
					memcpy(static_cast<char *>(item->buffer) + item->data_size,ptr,length);
					item->data_size = new_data_size;
				}
				else
				{
					unsigned int new_buffer_size = item->buffer_size * 2;
					while(new_buffer_size < new_data_size)
					{
						new_buffer_size *= 2;
					}
					void* new_buffer = common::mem_pool::getInstance().alloc(new_buffer_size);
					if(new_buffer == NULL)
					{
						return 0;
					}

					memcpy(new_buffer,item->buffer,item->data_size);
					memcpy(static_cast<char *>(new_buffer)+item->data_size,ptr,length);
					common::mem_pool::getInstance().free(item->buffer,item->buffer_size);
					item->buffer = new_buffer;
					item->buffer_size = new_buffer_size;
					item->data_size = new_data_size;
				}
			}
			return length;
		}

		void http_accessor_impl::post_handler(work_item * item)
		{
			//scheduler::getInstance().post_raw(boost::bind(&http_accessor_impl::call_handler,this,item));
			call_handler(item);
		}
		void http_accessor_impl::call_handler(work_item * item)
		{
			if (nullptr == item)
			{
				return;
			}

			xstring http_result = "";
			if (item->buffer && item->data_size > 0)
			{
				http_result = xstring(static_cast<const char *>(item->buffer), item->data_size);
			}
			work_item temp_item = *item;
			destroy_work_item(item);
			temp_item.handler(temp_item.request_id, temp_item.error_code, temp_item.error_info,http_result);
				
		}

		void http_accessor_impl::easy_error_handler(work_item* p_work_item)
		{
			xostringstream buf;
			buf << faith_curl_easy_strerror(static_cast<CURLcode>(p_work_item->error_code));
			p_work_item->error_info = buf.str();
			post_handler(p_work_item);
		}

		void http_accessor_impl::multi_error_handler(work_item* p_work_item)
		{
			xostringstream buf;
			buf << faith_curl_multi_strerror(static_cast<CURLMcode>(p_work_item->error_code));
			p_work_item->error_info = buf.str();
			p_work_item->error_code += CURLM_ERROR_BASE;
			post_handler(p_work_item);
		}
	}
}
//#endif