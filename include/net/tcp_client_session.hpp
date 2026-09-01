/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:00
	file base:	tcp_client_session
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _TCP_CLIENT_SESSION_H_
#define _TCP_CLIENT_SESSION_H_

#include "asio.hpp"
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <memory>
#include "xchar.hpp"
#include "tcp_session.hpp"
#include "tcp_client.hpp"

namespace faith 
{
	namespace net
	{
		using namespace boost::asio::ip;

		//class TCPClientSession;	

		class tcp_client_session : public tcp_session< tcp_client_session >
		{
			typedef std::shared_ptr<tcp_client_session>									tcp_client_session_ptr;
			typedef boost::function<void(unsigned int,const void*,size_t)>					onrecv_handler_type;
			typedef boost::function<void(tcp_client_session_ptr,tcp_client::e_connect_info,xstring)>	connection_handler_type;
			enum e_connection_state
			{
				e_cs_none,
				e_cs_resolving,
				e_cs_connecting,
				e_cs_connected
			};

		public:			
			tcp_client_session( 
				unsigned int conn_index,
				const xstring& host,const xstring& port,
				boost::asio::io_service &io_service,boost::asio::io_context::strand& strand,
				connection_handler_type handler_connection,
				onrecv_handler_type handler_recv,
				tcp_session_option & option,
				send_buffer_pool_type & send_buffer_pool,
				recv_buffer_pool_type & recv_buffer_pool
				);
			~tcp_client_session();
		public:
			void						start();
		private:
			void						handle_connect(const boost::system::error_code& err);
			void						handle_resolve(const boost::system::error_code& err,tcp::resolver::results_type results);
			void						post_connection_handler(tcp_client_session_ptr session,tcp_client::e_connect_info info,const xstring & msg);
		private:
			e_connection_state			m_connection_state;
			boost::asio::io_service&	m_io_service;
			boost::asio::io_context::strand&		m_strand;
			tcp::resolver				m_resolver;
			connection_handler_type		m_connection_handler;	// invoke it during connection
			xstring						m_hostname;
			xstring						m_servicename;
		};
	}
}

#endif
