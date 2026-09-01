/********************************************************************

	created:	2014/05/04

	created:	4:5:2014   19:21

	file base:	tcp_client

	file ext:	cpp

	author:		lucifer~yu

	

	purpose:	

*********************************************************************/

#include "asio.hpp"

#include "tcp_client_impl.hpp"

#include "tcp_client.hpp"

#include "vmp_header.h"



namespace faith 

{

	namespace net

	{

		tcp_client*			tcp_client::m_instance_ptr = NULL;

		//boost::once_flag	tcp_client::m_instance_flag = BOOST_ONCE_INIT;



		tcp_client::tcp_client() : impl_ptr(new tcp_client_impl())

		{

		}



		tcp_client::~tcp_client()

		{

		}



		tcp_client& tcp_client::get_instance(void) 

		{

			//boost::call_once(tcp_client::create_instance,m_instance_flag);

			create_instance();

			return *m_instance_ptr;

		}



		void tcp_client::create_instance(void)

		{

			static tcp_client sheduler_instance;

			m_instance_ptr = &sheduler_instance;

		}



		tcp_client_session_ptr tcp_client::connect_to(xstring ip,xstring service_port,

			connection_handler_type connection_handler,

			onclose_handler_type onclose_handler,

			onrecv_handler_type onrecv_handler)

		{

			VMPBEGIN

			tcp_client_session_ptr ret = impl_ptr->connect_to(ip,service_port,connection_handler,onclose_handler,onrecv_handler);

			VMPEND

			return ret;

		}



		void tcp_client::disconnect(const tcp_client_session_ptr& session)

		{

			impl_ptr->disconnect(session);

		}



		int tcp_client::send(const tcp_client_session_ptr& session,const void *data_ptr,size_t data_len)

		{

			VMPBEGIN

			int ret = impl_ptr->send(session,data_ptr,data_len);

			VMPEND

			return ret;

		}



		int	tcp_client::send_multi(const tcp_client_session_ptr& session,const datablock_queue_type& data_queue)

		{

			return impl_ptr->send_multi(session,data_queue);

		}



		bool tcp_client::set_option(const boost::any& option_item)

		{

			return impl_ptr->set_option(option_item);

		}



		bool tcp_client::get_option(boost::any& option_item)

		{

			return impl_ptr->get_option(option_item);

		}

	}

}

