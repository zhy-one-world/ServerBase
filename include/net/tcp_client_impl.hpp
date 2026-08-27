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

#include <map>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include "tcp_client_session.hpp"
#include "conn_index_generator.hpp"
#include "direct_addressing_array.hpp"
#include "tcp_client.hpp"
#include "plugin.hpp"
#include "datablock.hpp"
#include "xchar.hpp"
#include "string_buffer.hpp"

namespace faith
{
	namespace net
	{		
		typedef boost::shared_ptr<tcp_client_session>	tcp_client_session_ptr;
		class tcp_client_impl : public options_container, private boost::noncopyable
		{
			enum
			{
				connection_limit=8192
			};
			typedef boost::function<void(unsigned int)>										onclose_handler_type;
			typedef boost::function<void(unsigned int,const void*,size_t)>					onrecv_handler_type;
			typedef boost::function<void(unsigned int,tcp_client::e_connect_info,xstring)>	connection_handler_type;
			typedef net::direct_addressing_array<tcp_client_session>					conn_array;
		public:
			tcp_client_impl();
			~tcp_client_impl();
		public:
			unsigned int			connect_to(unsigned int conn_index,
				xstring ip,xstring service_port,
				connection_handler_type connection_handler,
				onclose_handler_type onclose_handler,
				onrecv_handler_type onrecv_handler);

			void					disconnect(unsigned int conn_index);
			int						send(unsigned int conn_index,const void *data_ptr,size_t data_len);
			int						send_multi(unsigned int conn_index,const datablock_queue_type& data_queue);
			bool					set_option(const boost::any& option_item);
			void					init_client_server(unsigned int server_num);
		private:
			unsigned int create_session(
				unsigned int conn_index,
				xstring ip,
				xstring service_port,
				connection_handler_type connection_handler,
				onclose_handler_type onclose_handler,
				onrecv_handler_type onrecv_handler
				);
			void					destroy_session(onclose_handler_type onclose_handler,tcp_client_session * session);

			unsigned int			mlb_connect_to(
				unsigned int conn_index,
				xstring ip,xstring service_port,
				connection_handler_type connection_handler,
				onclose_handler_type onclose_handler,
				onrecv_handler_type onrecv_handler);
			void					mlb_disconnect(unsigned int conn_index);
			int						mlb_send(unsigned int conn_index,const ::faith::string_buffer & data);
			int						inner_send( unsigned int conn_index,const void *data_ptr,size_t data_len );
			int						mlb_send_multi(unsigned int conn_index,const ::faith::string_buffer & data_queue);
			int						inner_send_multi(unsigned int conn_index,const datablock_queue_type& data_queue);
			bool					mlb_remove_plugin(unsigned int conn_index,int plugin_slot);
			void					init_options();
			void					apply_options();
			bool					check_options();
			void					create_buffer_pools();
			void					opc_report_connection_count_fun();
			void					opc_create_counter_fun();
		private:
			boost::recursive_mutex	m_mutex;
			tcp_client_session**	m_conn_array;
			int						m_conn_size;
			int						m_conn_max_size;
			conn_index_generator	m_conn_index_generator;
			bool					m_options_applied;
			tcp_session_option		m_session_option;
			send_buffer_pool_type*	m_send_buffer_pool;
			recv_buffer_pool_type*	m_recv_buffer_pool;			
			scheduler_impl&			m_scheduler_impl;	// 性能优化,去除getInstance
		};
	}
}

#endif
