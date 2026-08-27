/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:22
	file base:	tcp_client_session
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "tcp_client_session.hpp"
#include "scheduler.hpp"
#include <boost/bind.hpp>
#include "mem_pool.hpp"

#pragma warning(disable:4503)

namespace faith 
{
	namespace net 
	{
		tcp_client_session::tcp_client_session( 
			unsigned int conn_index,
			const xstring& host,const xstring& service,
			boost::asio::io_service& io_service,
			boost::asio::io_context::strand& strand,
			connection_handler_type handler_connection,
			onrecv_handler_type handler_recv,
			tcp_session_option & option,
			send_buffer_pool_type & send_buffer_pool,
			recv_buffer_pool_type & recv_buffer_pool
			):
			tcp_session<tcp_client_session>(
				conn_index,io_service,
				handler_recv,
				option,
				send_buffer_pool,
				recv_buffer_pool),
				m_hostname(host),
				m_servicename(service),			
				m_connection_handler(handler_connection),
				m_resolver(io_service),
				m_io_service(io_service),
				m_strand(strand),
				m_connection_state(e_cs_none)
		{
		}

		tcp_client_session::~tcp_client_session(void)
		{
			switch(m_connection_state)
			{
			case e_cs_resolving:
				post_connection_handler(get_conn_index(),tcp_client::e_ci_addr_resovle_failed,_XTEXT("Unknown error."));
				break;
			case e_cs_connecting:
				post_connection_handler(get_conn_index(),tcp_client::e_ci_connection_failed,_XTEXT("Unknown error."));
				break;
			default:
				break;
			}			
		}

		void tcp_client_session::start()
		{
			// Start an asynchronous resolve to translate the server and service names
			// into a list of endpoints.
			m_connection_state = e_cs_resolving;
#if defined(FAITH_UNICODE)
			int host_len = (m_hostname.size() + 1)* 3;
			int servicname_len = (m_servicename.size() + 1) * 3;
			char* temp_host = (char*)common::mem_pool::getInstance().alloc(host_len);
			char* temp_service = (char*)common::mem_pool::getInstance().alloc(servicname_len);
			assert(temp_host && temp_service);
			common::utility::_iconv_one("UCS-2LE",locale_charset(),(void*)m_hostname.c_str(), m_hostname.size()*sizeof(xchar),temp_host,host_len);
			common::utility::_iconv_one("UCS-2LE",locale_charset(),(void*)m_servicename.c_str(), m_servicename.size()*sizeof(xchar),temp_service,servicname_len);
			std::string host_a(temp_host);
			std::string service_a(temp_service);
			common::mem_pool::getInstance().free(temp_service,servicname_len);
			common::mem_pool::getInstance().free(temp_host,host_len);
			m_resolver.async_resolve(host_a, service_a,
				m_strand.wrap(boost::bind(&tcp_client_session::handle_resolve,shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::results)));
#else
			m_resolver.async_resolve(m_hostname, m_servicename,
				m_strand.wrap(boost::bind(&tcp_client_session::handle_resolve,shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::results)));
#endif
		}

		void tcp_client_session::handle_resolve(const boost::system::error_code& err,tcp::resolver::results_type results)
		{
			if (!err)
			{
				m_connection_handler(get_conn_index(),tcp_client::e_ci_addr_resovle_successed,_XTEXT("address resolved successfully"));
				m_connection_state = e_cs_connecting;
				boost::asio::async_connect(get_socket(), results,
					m_strand.wrap(boost::bind(&tcp_client_session::handle_connect,shared_from_this(),
					boost::asio::placeholders::error)));
			}
			else
			{
				m_connection_state = e_cs_none;
				post_connection_handler(get_conn_index(),tcp_client::e_ci_addr_resovle_failed,_asio_message(err.message()));
			}
		}

		void tcp_client_session::handle_connect(const boost::system::error_code& err)
		{
			if (!err)
			{
				open();
				m_connection_state = e_cs_connected;
				m_connection_handler(get_conn_index(),tcp_client::e_ci_connection_successed,_XTEXT("connection established"));
			}
			else
			{
				m_connection_state = e_cs_none;
				post_connection_handler(get_conn_index(),tcp_client::e_ci_connection_failed,_asio_message(err.message()));
			}
		}

		void tcp_client_session::post_connection_handler(unsigned int conn_index,tcp_client::e_connect_info info,const xstring  & msg)
		{
			scheduler::getInstance().get_impl()->inner_post(boost::bind(m_connection_handler,conn_index,info,msg));
		}
	}
}
