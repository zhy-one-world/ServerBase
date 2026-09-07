/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:09
	file base:	tcp_server_impl
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _TCP_SERVER_IMPL_H_
#define _TCP_SERVER_IMPL_H_

#include "asio.hpp"
#include "unique_id_generator.hpp"
#include "string_buffer.hpp"
#include "tcp_server_session.hpp"
#include "direct_addressing_array.hpp"
#include "options_container.hpp"
#include "tcp_server.hpp"
#include "plugin.hpp"
#include "datablock.hpp"
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/thread/condition.hpp>
#include <boost/function.hpp>
#include <boost/detail/atomic_count.hpp>
#include <string>
#include <utility>
#include <iostream>
#include <memory>
#include "hash_container.hpp"
#include "xchar.hpp"

namespace faith 
{
	namespace net 
	{
		typedef std::shared_ptr<tcp_server_session>						tcp_server_session_ptr;
		typedef boost::function<void(tcp_server::e_server_status_type)>		serverstatus_handler_type;
		typedef boost::function<void(tcp_server_session_ptr)>				onclose_handler_type;
		typedef boost::function<void(tcp_server_session_ptr)>				onconnected_handler_type;
		typedef boost::function<void(tcp_server_session_ptr,const void*,size_t)>	recv_handler_type;

		class scheduler_impl;

		//	asynchronous TCP server implemention
		class tcp_server_impl : public options_container, private boost::noncopyable
		{
		public:
			explicit tcp_server_impl( 
				serverstatus_handler_type status_handler,
				onconnected_handler_type onconnected_handler,
				onclose_handler_type onclose_handler,
				recv_handler_type recv_handler,
				boost::asio::io_service &io_service,
				boost::asio::io_context::strand &strand,
				xstring ip,
				int tcp_port );
			explicit tcp_server_impl( 
				serverstatus_handler_type status_handler,
				onconnected_handler_type onconnected_handler,
				onclose_handler_type onclose_handler,
				recv_handler_type recv_handler,
				boost::asio::io_service &io_service,
				boost::asio::io_context::strand &strand,
				unsigned int tcp_port );
			virtual ~tcp_server_impl();
		public:
			std::size_t								get_conn_count( void );
			xstring									get_ip_addr( const tcp_server_session_ptr& session );
			unsigned short							get_ip_port( const tcp_server_session_ptr& session );
			unsigned int							get_session_thread_id( const tcp_server_session_ptr& session );
			bool									start( void );
			int										send( const tcp_server_session_ptr& session,const void *data_ptr,size_t data_len );
			int										send_multi(const tcp_server_session_ptr& session,const datablock_queue_type& data_queue);
			bool									close( const tcp_server_session_ptr& session );
			bool									set_option(const boost::any& option_item);
		private:
			void									init_handlers(serverstatus_handler_type status_handler,onconnected_handler_type onconnected_handler,onclose_handler_type onclose_handler,recv_handler_type recv_handler);
			void									clear_handlers();
			void									init_options();
			void									apply_options();
			bool									check_options();
			void									create_buffer_pools();
			bool									is_session_capacity_full() const;
			void									handle_accept( tcp_server_session_ptr session_ptr,const boost::system::error_code& error );
			void									handle_session_close(tcp_server_session_ptr session);
			void									finish_session_close(tcp_server_session_ptr session, bool need_accept);
			void									close_on_main(tcp_server_session_ptr session);
			void									release_session_count();
			void									listen();
			tcp_server_session_ptr					create_session();
			int										inner_send( const tcp_server_session_ptr& session,const void *data_ptr,size_t data_len );
			int										inner_send_multi(const tcp_server_session_ptr& session,const datablock_queue_type& data_queue);
		private:
			boost::asio::io_service &				m_io_service;
			boost::asio::io_context::strand &		m_strand;
			boost::asio::ip::tcp::acceptor			m_acceptor;
			boost::asio::ip::tcp::endpoint			m_endpoint;
			serverstatus_handler_type				m_status_handler;
			recv_handler_type						m_recv_handler;
			onclose_handler_type					m_onclose_handler;
			onconnected_handler_type				m_onconnected_handler;
			unsigned int							m_conn_count;
			unsigned int							m_connections_limit;
			boost::recursive_mutex					m_mutex;				// for thread safe
			bool									m_be_listening;
			boost::uint32_t							m_instance_id;
			bool									m_options_applied;
			tcp_session_option						m_session_option;
			send_buffer_pool_type*					m_send_buffer_pool;
			recv_buffer_pool_type*					m_recv_buffer_pool;			
			scheduler_impl &						m_scheduler_impl;
		};
	}
}

#endif
