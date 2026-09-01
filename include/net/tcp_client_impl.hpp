/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:00
	file base:	tcp_client_impl
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _TCP_CLIENT_IMPL_H_
#define _TCP_CLIENT_IMPL_H_

#include <memory>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include "tcp_client_session.hpp"
#include "tcp_client.hpp"
#include "plugin.hpp"
#include "datablock.hpp"
#include "xchar.hpp"
#include "string_buffer.hpp"

namespace faith
{
	namespace net
	{
		class tcp_client_impl : public options_container, private boost::noncopyable
		{
			typedef boost::function<void(tcp_client_session_ptr)>										onclose_handler_type;
			typedef boost::function<void(tcp_client_session_ptr,const void*,size_t)>					onrecv_handler_type;
			typedef boost::function<void(tcp_client_session_ptr,tcp_client::e_connect_info,xstring)>	connection_handler_type;
		public:
			tcp_client_impl();
			~tcp_client_impl();
		public:
			tcp_client_session_ptr	connect_to(
				xstring ip,xstring service_port,
				connection_handler_type connection_handler,
				onclose_handler_type onclose_handler,
				onrecv_handler_type onrecv_handler);

			void					disconnect(const tcp_client_session_ptr& session);
			int						send(const tcp_client_session_ptr& session,const void *data_ptr,size_t data_len);
			int						send_multi(const tcp_client_session_ptr& session,const datablock_queue_type& data_queue);
			bool					set_option(const boost::any& option_item);
		private:
			void					finish_session_close(
				onclose_handler_type onclose_handler,
				tcp_client_session_ptr session,
				unsigned int conn_index);
			tcp_client_session_ptr	create_session(
				xstring ip,
				xstring service_port,
				connection_handler_type connection_handler,
				onclose_handler_type onclose_handler,
				onrecv_handler_type onrecv_handler
				);

			tcp_client_session_ptr	mlb_connect_to(
				xstring ip,xstring service_port,
				connection_handler_type connection_handler,
				onclose_handler_type onclose_handler,
				onrecv_handler_type onrecv_handler);
			void					mlb_disconnect(const tcp_client_session_ptr& session);
			int						mlb_send(const tcp_client_session_ptr& session,const ::faith::string_buffer & data);
			int						inner_send(const tcp_client_session_ptr& session,const void *data_ptr,size_t data_len );
			int						mlb_send_multi(const tcp_client_session_ptr& session,const ::faith::string_buffer & data_queue);
			int						inner_send_multi(const tcp_client_session_ptr& session,const datablock_queue_type& data_queue);
			bool					mlb_remove_plugin(const tcp_client_session_ptr& session,int plugin_slot);
			void					init_options();
			void					apply_options();
			bool					check_options();
			void					create_buffer_pools();
			void					opc_report_connection_count_fun();
			void					opc_create_counter_fun();
		private:
			boost::recursive_mutex					m_mutex;
			bool									m_options_applied;
			tcp_session_option						m_session_option;
			send_buffer_pool_type*					m_send_buffer_pool;
			recv_buffer_pool_type*					m_recv_buffer_pool;
			scheduler_impl&							m_scheduler_impl;
		};
	}
}

#endif
