/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:22
	file base:	tcp_server_session
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "tcp_server_session.hpp"
#include "scheduler.hpp"
#include "tcp_server.hpp"
#include "tcp_server_impl.hpp"
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

namespace faith 
{
	namespace net 
	{
		//
		//	Implemention of TCPServerSession
		//
		tcp_server_session::tcp_server_session(
			unsigned int connindex,
			boost::asio::io_service &io_service,
			recv_handler_type handler_recv,
			tcp_session_option & option,
			send_buffer_pool_type & send_buffer_pool,
			recv_buffer_pool_type & recv_buffer_pool
			):
			tcp_session<tcp_server_session>(connindex,io_service,handler_recv,option,send_buffer_pool,recv_buffer_pool)
 		{				
		}

		tcp_server_session::~tcp_server_session()
		{
		}

		void tcp_server_session::start()
		{
			open();
		}
	}
}
