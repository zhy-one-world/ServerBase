/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:22
	file base:	tcp_server
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "asio.hpp"
#include "tcp_server_impl.hpp"
#include "tcp_server.hpp"
#include "scheduler.hpp"
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

namespace faith
{
	namespace net 
	{
		//
		//	Implemention of TCPServer
		//
		tcp_server::tcp_server( 
			serverstatus_handler_type status_handler,
			onconnected_handler_type onconnected_handler,
			onclose_handler_type onclose_handler,
			onrecv_handler_type onrecv_handler,
			xstring ip,int tcp_port ) : impl_ptr(new tcp_server_impl(
											status_handler,
											onconnected_handler,
											onclose_handler,
											onrecv_handler,
											scheduler::getInstance().get_impl()->get_ioservice(),
											scheduler::getInstance().get_impl()->get_strand(),
											ip,tcp_port))
		{
		}

		tcp_server::tcp_server(
			serverstatus_handler_type status_handler,
			onconnected_handler_type onconnected_handler,
			onclose_handler_type onclose_handler,
			onrecv_handler_type onrecv_handler,
			xstring ip,int tcp_port,
			unsigned int thread_id ) : impl_ptr(new tcp_server_impl(
											status_handler,
											onconnected_handler,
											onclose_handler,
											onrecv_handler,
											scheduler::getInstance().get_impl()->get_ioservice(thread_id),
											scheduler::getInstance().get_impl()->get_strand(thread_id),
											ip,tcp_port))
		{
		}

		tcp_server::tcp_server( 
			serverstatus_handler_type status_handler,
			onconnected_handler_type onconnected_handler,
			onclose_handler_type onclose_handler,
			onrecv_handler_type onrecv_handler,int tcp_port ) : impl_ptr(new tcp_server_impl(
																	status_handler,
																	onconnected_handler,
																	onclose_handler,
																	onrecv_handler,
																	scheduler::getInstance().get_impl()->get_ioservice() ,
																	scheduler::getInstance().get_impl()->get_strand(),
																	tcp_port))
		{
		}

		tcp_server::~tcp_server()
		{
		}

		bool tcp_server::start( void )
		{
			return impl_ptr->start();
		}

		void tcp_server::stop( bool wait_until_finished /*= false*/ )
		{
			impl_ptr->stop(wait_until_finished);
		}

		std::size_t tcp_server::get_conn_count()
		{
			return impl_ptr->get_conn_count();
		}

		xstring tcp_server::get_ip_addr( unsigned int conn_index )
		{
			return impl_ptr->get_ip_addr(conn_index);
		}

		unsigned short tcp_server::get_ip_port( unsigned int conn_index )
		{
			return impl_ptr->get_ip_port(conn_index);
		}

		int tcp_server::send( unsigned int conn_index,const void *data_ptr,size_t data_len )
		{
			return impl_ptr->send(conn_index,data_ptr,data_len);
		}

		int	tcp_server::send_multi(unsigned int conn_index,const datablock_queue_type& data_queue)
		{
			return impl_ptr->send_multi(conn_index,data_queue);
		}

		bool tcp_server::close( unsigned int conn_index )
		{
			return impl_ptr->close(conn_index);
		}

		bool tcp_server::set_option(const boost::any& option_item)
		{
			return impl_ptr->set_option(option_item);
		}

		bool tcp_server::get_option(boost::any& option_item)
		{
			return impl_ptr->get_option(option_item);
		}
		void tcp_server::init_client_server(unsigned int server_num, unsigned int init_num)
		{
			impl_ptr->init_client_server(server_num);
		}
	}
}
