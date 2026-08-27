/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:22
	file base:	tcp_client_impl
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "asio.hpp"
#include <new>
#include <boost/bind.hpp>
#include "tcp_client_impl.hpp"
#include "scheduler.hpp"
//#include "faith_logger.hpp"
#include "mlb.hpp"
#include "vmp_header.h"

#pragma warning(disable:4503)

namespace faith
{
	namespace net
	{
		tcp_client_impl::tcp_client_impl(void):
			m_options_applied(false),
			m_send_buffer_pool(NULL),
			m_recv_buffer_pool(NULL),
			m_scheduler_impl(*scheduler::getInstance().get_impl())
		{
			init_options();
			opc_create_counter_fun();
			m_conn_index_generator.set_max_count(connection_limit);
		}

		tcp_client_impl::~tcp_client_impl(void)
		{
			delete m_send_buffer_pool;
			delete m_recv_buffer_pool;
		}

		static void call_connection_handler(tcp_client::connection_handler_type connection_handler,unsigned int connindex,tcp_client::e_connect_info status,xstring info)
		{
			VMPBEGIN

			connection_handler(connindex,status,info);

			VMPEND
		}
		static void recur_connection_handler(tcp_client::connection_handler_type connection_handler,unsigned int connindex,int status,xstring info)
		{
			connection_handler(connindex,static_cast<tcp_client::e_connect_info>(status),info);
		}

		static void call_onclose_handler(tcp_client::onclose_handler_type onclose_handler,unsigned int connindex)
		{
			VMPBEGIN

			onclose_handler(connindex);

			VMPEND
		}

		static void call_onrecv_handler(tcp_client::onrecv_handler_type onrecv_handler,unsigned int connindex,const void *data_ptr,size_t data_len )
		{
			VMPBEGIN

			onrecv_handler(connindex,data_ptr,data_len);

			VMPEND
		}

		static void recur_onrecv_handler(tcp_client::onrecv_handler_type onrecv_handler,unsigned int connindex,const xstring & data )
		{
			onrecv_handler(connindex,data.c_str(),data.length());
		}

		static void ondestroy_handler(unsigned int connindex)
		{

		}

		static bool call_plugin(plug_in plugin,boost::shared_ptr<int> slot,unsigned int connindex,const void *data_ptr,size_t data_len )
		{

			bool ret=plugin(connindex,data_ptr,data_len);

			return ret;
		}

		static bool in_tcpclient_remove_plugin = false;
		static void plugin_removed_handler(unsigned int connindex,int slot)
		{

		}
		static void recur_plugin(plug_in plugin,unsigned int connindex,const xstring & data )
		{
			plugin(connindex,data.c_str(),data.length());
		}

		unsigned int tcp_client_impl::mlb_connect_to(
			unsigned int conn_index,
			xstring ip,xstring service_port,
			connection_handler_type connection_handler,
			onclose_handler_type onclose_handler,
			onrecv_handler_type onrecv_handler)
		{
			VMPBEGIN
			unsigned int ret = tcp_client::invalid_conn_index;
			boost::recursive_mutex::scoped_lock lock(m_mutex);

			apply_options();

			// start connection
			ret = create_session(conn_index, ip,service_port,
				boost::bind(call_connection_handler,connection_handler,_1,_2,_3),
				boost::bind(call_onclose_handler,onclose_handler,_1),
				boost::bind(call_onrecv_handler,onrecv_handler,_1,_2,_3)
				);
			VMPEND
mlb_connect_to_end:
			return ret;
		}

		namespace
		{
			extern xchar sz_connect_to[] = _XTEXT("TCPClient::connect_to");
		}		
		unsigned int tcp_client_impl::connect_to(unsigned int conn_index, xstring ip,xstring service_port,
			connection_handler_type connection_handler,
			onclose_handler_type onclose_handler,
			onrecv_handler_type onrecv_handler)
		{
			VMPBEGIN
			unsigned int ret;

			ret = mlb_connect_to(conn_index, ip,service_port,connection_handler,onclose_handler,onrecv_handler);

			VMPEND
			return ret;
		}

		//MLB_CLASS_FUNC_1(void,TCPClient_impl,disconnect,
		//	unsigned int,conn_index
		//	)
		void tcp_client_impl::disconnect(unsigned int conn_index)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			if (conn_index < 0 || conn_index >= m_conn_max_size)
			{
				return;
			}
			tcp_client_session* pSession = m_conn_array[conn_index];
			if( pSession)
			{
				pSession->close();
			}
		}

		int	tcp_client_impl::inner_send( unsigned int conn_index,const void *data_ptr,size_t data_len)
		{
			VMPBEGIN
			if (conn_index < 0 || conn_index >= m_conn_max_size)
			{
				return 0;
			}
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			
			tcp_client_session* pSession = m_conn_array[conn_index];
			if( pSession == NULL )
			{
				return 0;
			}
			else
			{
				return pSession->send(data_ptr,data_len);
			}
			VMPEND
			return 0;
		}

		int tcp_client_impl::mlb_send(unsigned int conn_index,const common::string_buffer & data)
		{
			return inner_send(conn_index,data.c_str(),data.length());
		}

		namespace
		{
			static const xstring sz_send(_XTEXT("TCPClient::send"));
		}
		int tcp_client_impl::send(unsigned int conn_index,const void *data_ptr,size_t data_len)
		{
			VMPBEGIN
			int ret;

			ret = inner_send(conn_index,data_ptr,data_len);
			goto send_end;

			VMPEND
send_end:
			return ret;
		}

		int tcp_client_impl::inner_send_multi(unsigned int conn_index,const datablock_queue_type& data_queue)
		{
			if (conn_index < 0 || conn_index >= m_conn_max_size)
			{
				return 0;
			}
			boost::recursive_mutex::scoped_lock lock(m_mutex);

			tcp_client_session* pSession = m_conn_array[conn_index];
			if( pSession == NULL )
			{
				return 0;
			}
			else
			{
				return pSession->send_multi(data_queue);
			}
		}

		int tcp_client_impl::mlb_send_multi(unsigned int conn_index,const common::string_buffer & data_queue)
		{
			const char * str = data_queue.c_str();
			size_t size = *reinterpret_cast<const size_t *>(str);
			str += sizeof(size);
			
			datablock_queue_type datablocks;
			for(size_t i=0;i<size;++i)
			{
				datablock_type datablock;
				datablock.second = *reinterpret_cast<const size_t *>(str);
				str += sizeof(datablock.second);
				datablock.first = str;
				str += datablock.second;
				datablocks.push_back(datablock);
			}
			return inner_send_multi(conn_index,datablocks);
		}

		namespace
		{
			static const xstring sz_send_multi(_XTEXT("TCPClient::send_multi"));
		}

		int tcp_client_impl::send_multi(unsigned int conn_index,const datablock_queue_type& data_queue)
		{
			return inner_send_multi(conn_index,data_queue);
		}

		unsigned int tcp_client_impl::create_session(
			unsigned int conn_index,
			xstring ip,
			xstring service_port,
			connection_handler_type connection_handler,
			onclose_handler_type onclose_handler,
			onrecv_handler_type onrecv_handler
		)
		{
			VMPBEGIN
				if (conn_index >= m_conn_max_size || m_conn_array[conn_index])
				{
					return tcp_client::invalid_conn_index;
				}
				boost::asio::io_service& io=scheduler::getInstance().get_impl()->get_ioservice();
				boost::asio::io_context::strand& strand=scheduler::getInstance().get_impl()->get_strand();

				tcp_client_session* session_ptr = new/*(std::nothrow)*/ tcp_client_session(
					conn_index,
					ip,service_port,
					io,strand,
					connection_handler,
					onrecv_handler,
					m_session_option,
					*m_send_buffer_pool,
					*m_recv_buffer_pool);

				if(session_ptr==NULL)
				{
					return tcp_client::invalid_conn_index;
				}
				session_ptr->set_conn_index(conn_index);
				m_conn_array[conn_index] = session_ptr;
				tcp_client_session_ptr ret = tcp_client_session_ptr(session_ptr, strand.wrap(boost::bind(&tcp_client_impl::destroy_session, this, onclose_handler, _1)));
				ret->start();
			VMPEND
			return conn_index;
		}

		void tcp_client_impl::destroy_session(onclose_handler_type onclose_handler,tcp_client_session * session)
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			if (NULL == session)
			{
				return;
			}
			unsigned conn_index = session->get_conn_index();
			bool been_opened = session->been_opened();
			
			if(been_opened)
			{
				// invoke external handler
				onclose_handler(conn_index);
			}

			{
				delete session;
				session = NULL;
				// release resource
				m_conn_array[conn_index] = nullptr;
			}
			ondestroy_handler(conn_index);			
		}

		void tcp_client_impl::init_options()
		{
			options_container::set_option(tcp_client::options::max_packet_size(8*1024),true);
			options_container::set_option(tcp_client::options::send_buffer_size(32*1024),true);
			options_container::set_option(tcp_client::options::recv_buffer_size(16*1024),true);
			options_container::set_option(tcp_client::options::delaysending_size_threshold(0),true);
		}
		void tcp_client_impl::init_client_server(unsigned int server_num)
		{
			m_conn_max_size = server_num;
			m_conn_array = new tcp_client_session*[m_conn_max_size];
			apply_options();
			for (int i = 0; i < m_conn_max_size; ++i)
			{
				m_conn_array[i] = nullptr;
			}
			m_conn_size = 0;
		}
#define GET_OPTION(OBJ,OPTION)						\
	{												\
		tcp_client::options::OPTION opt;				\
		get_option(opt);							\
		OBJ.OPTION = opt.value;						\
	}
	
		void tcp_client_impl::apply_options()
		{
			if(m_options_applied)
			{
				return;
			}
//			GET_OPTION(m_session_option,tcp_nodelay)
			GET_OPTION(m_session_option,max_packet_size)
			GET_OPTION(m_session_option,send_buffer_size)
			GET_OPTION(m_session_option,recv_buffer_size)
			GET_OPTION(m_session_option,delaysending_size_threshold)
			//GET_OPTION(m_session_option,delaysending_time_threshold)
//			GET_OPTION(m_session_option,close_overflowed_session)
// 			if(m_session_option.delaysending_size_threshold > 0 && m_session_option.tcp_nodelay == false)
// 			{
// 				options_container::set_option(tcp_client::options::tcp_nodelay(true));
// 				m_session_option.tcp_nodelay = true;
// 			}

			create_buffer_pools();

			m_options_applied = true;
		}

		bool tcp_client_impl::set_option(const boost::any& option_item)
		{
			if(m_options_applied)
			{
				return false;
			}
			boost::any tmp(option_item);
			if(!get_option(tmp))
			{
				return false;
			}
			if(!options_container::set_option(option_item))
			{
				return false;
			}
			if(!check_options())
			{
				options_container::set_option(tmp);
				return false;
			}
			return true;
		}

		bool tcp_client_impl::check_options()
		{
			tcp_session_option options;
			GET_OPTION(options,max_packet_size)
			GET_OPTION(options,send_buffer_size)
			GET_OPTION(options,recv_buffer_size)
			GET_OPTION(options,delaysending_size_threshold)
			return options.check_options();
		}

		void tcp_client_impl::create_buffer_pools()
		{
			unsigned int max_real_packet_size = m_session_option.max_packet_size + sizeof(tcp_pak_header);

			m_send_buffer_pool = new (std::nothrow) send_buffer_pool_type(m_session_option.send_buffer_size);
			m_recv_buffer_pool = new (std::nothrow) recv_buffer_pool_type(m_session_option.recv_buffer_size+max_real_packet_size);
		}

		void tcp_client_impl::opc_create_counter_fun()
		{
		}

		void tcp_client_impl::opc_report_connection_count_fun()
		{
		}
	}
}
