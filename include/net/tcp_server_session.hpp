/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:56
	file base:	tcp_server_session
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _TCP_SERVER_SESSION_H_
#define _TCP_SERVER_SESSION_H_

#include "asio.hpp"
#include <boost/function.hpp>
#include "tcp_session.hpp"

namespace faith 
{
	namespace net 
	{
		class tcp_server_session : public tcp_session< tcp_server_session >
		{
			typedef boost::function<void(unsigned int,const void*,size_t)> recv_handler_type;
		public:			
			tcp_server_session(
				unsigned int connindex,
				unsigned int thread_id,
				boost::asio::io_service &io_service,
				recv_handler_type handler_recv,
				tcp_session_option & option,
				send_buffer_pool_type & send_buffer_pool,
				recv_buffer_pool_type & recv_buffer_pool
				);
			~tcp_server_session();
		public:
			void	start();
			unsigned int get_thread_id() const { return m_thread_id; }
		private:
			unsigned int m_thread_id;
		};
	}
}

#endif
