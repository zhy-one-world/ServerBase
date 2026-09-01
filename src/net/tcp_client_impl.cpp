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
#include "mlb.hpp"
#include "vmp_header.h"
#include <rlog.hpp>

#pragma warning(disable:4503)

namespace faith
{
	namespace net
	{
		namespace
		{
			void call_connection_handler(
				tcp_client::connection_handler_type connection_handler,
				tcp_client_session_ptr session,
				tcp_client::e_connect_info status,
				xstring info)
			{
				VMPBEGIN
				connection_handler(session,status,info);
				VMPEND
			}

			void call_onclose_handler(
				tcp_client::onclose_handler_type onclose_handler,
				tcp_client_session_ptr session)
			{
				VMPBEGIN
				onclose_handler(session);
				VMPEND
			}

			void call_onrecv_handler(
				tcp_client::onrecv_handler_type onrecv_handler,
				tcp_client_session_ptr session,
				const void *data_ptr,
				size_t data_len)
			{
				VMPBEGIN
				onrecv_handler(session,data_ptr,data_len);
				VMPEND
			}

			struct recv_handler_bridge
			{
				tcp_client::onrecv_handler_type user_handler;
				tcp_client_session_ptr session;

				void on_recv(unsigned int, const void* data_ptr, size_t data_len) const
				{
					if (session)
					{
						call_onrecv_handler(user_handler, session, data_ptr, data_len);
					}
				}
			};
		}

		tcp_client_impl::tcp_client_impl(void):
			m_options_applied(false),
			m_send_buffer_pool(NULL),
			m_recv_buffer_pool(NULL),
			m_scheduler_impl(*scheduler::getInstance().get_impl())
		{
			init_options();
			opc_create_counter_fun();
		}

		tcp_client_impl::~tcp_client_impl(void)
		{
			delete m_send_buffer_pool;
			delete m_recv_buffer_pool;
		}

		tcp_client_session_ptr tcp_client_impl::mlb_connect_to(
			xstring ip,xstring service_port,
			connection_handler_type connection_handler,
			onclose_handler_type onclose_handler,
			onrecv_handler_type onrecv_handler)
		{
			VMPBEGIN
			tcp_client_session_ptr session_ptr;
			boost::recursive_mutex::scoped_lock lock(m_mutex);

			apply_options();

			session_ptr = create_session(
				ip,
				service_port,
				boost::bind(call_connection_handler,connection_handler,_1,_2,_3),
				boost::bind(call_onclose_handler,onclose_handler,_1),
				onrecv_handler);
			VMPEND
			return session_ptr;
		}

		tcp_client_session_ptr tcp_client_impl::connect_to(
			xstring ip,xstring service_port,
			connection_handler_type connection_handler,
			onclose_handler_type onclose_handler,
			onrecv_handler_type onrecv_handler)
		{
			VMPBEGIN
			tcp_client_session_ptr ret = mlb_connect_to(
				ip,service_port,connection_handler,onclose_handler,onrecv_handler);
			VMPEND
			return ret;
		}

		void tcp_client_impl::disconnect(const tcp_client_session_ptr& session)
		{
			if (session == NULL)
			{
				return;
			}
			session->set_close_handler(NULL);
			session->close();
		}

		int	tcp_client_impl::inner_send(const tcp_client_session_ptr& session,const void *data_ptr,size_t data_len)
		{
			VMPBEGIN
			if (session == NULL)
			{
				return 0;
			}
			return session->send(data_ptr, data_len);
			VMPEND
			return 0;
		}

		int tcp_client_impl::mlb_send(const tcp_client_session_ptr& session,const common::string_buffer & data)
		{
			return inner_send(session,data.c_str(),data.length());
		}

		int tcp_client_impl::send(const tcp_client_session_ptr& session,const void *data_ptr,size_t data_len)
		{
			VMPBEGIN
			int ret = inner_send(session,data_ptr,data_len);
			VMPEND
			return ret;
		}

		int tcp_client_impl::inner_send_multi(const tcp_client_session_ptr& session,const datablock_queue_type& data_queue)
		{
			if (session == NULL)
			{
				return 0;
			}
			return session->send_multi(data_queue);
		}

		int tcp_client_impl::mlb_send_multi(const tcp_client_session_ptr& session,const common::string_buffer & data_queue)
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
			return inner_send_multi(session,datablocks);
		}

		int tcp_client_impl::send_multi(const tcp_client_session_ptr& session,const datablock_queue_type& data_queue)
		{
			return inner_send_multi(session,data_queue);
		}

		void tcp_client_impl::finish_session_close(
			onclose_handler_type onclose_handler,
			tcp_client_session_ptr session,
			unsigned int conn_index)
		{
			(void)conn_index;
			if (session == NULL)
			{
				return;
			}
			onclose_handler(session);
		}

		tcp_client_session_ptr tcp_client_impl::create_session(
			xstring ip,
			xstring service_port,
			connection_handler_type connection_handler,
			onclose_handler_type onclose_handler,
			onrecv_handler_type onrecv_handler
		)
		{
			VMPBEGIN
			if (m_send_buffer_pool == NULL || m_recv_buffer_pool == NULL)
			{
				_RLOG_(MERROR, "tcp_client create_session buffer pool is null, send:"
					<< (m_send_buffer_pool == NULL) << " recv:" << (m_recv_buffer_pool == NULL));
				return tcp_client_session_ptr();
			}

			boost::asio::io_service& io = scheduler::getInstance().get_impl()->get_ioservice();
			boost::asio::io_context::strand& strand = scheduler::getInstance().get_impl()->get_strand();

			const std::shared_ptr<recv_handler_bridge> recv_bridge = std::make_shared<recv_handler_bridge>();
			recv_bridge->user_handler = onrecv_handler;

			typedef boost::function<void(unsigned int,const void*,size_t)> internal_recv_handler_type;
			internal_recv_handler_type wrapped_recv = boost::bind(
				&recv_handler_bridge::on_recv,
				recv_bridge,
				_1,
				_2,
				_3);

			tcp_client_session_ptr session_ptr = std::make_shared<tcp_client_session>(
				tcp_client::invalid_conn_index,
				ip,
				service_port,
				io,
				strand,
				connection_handler,
				wrapped_recv,
				m_session_option,
				*m_send_buffer_pool,
				*m_recv_buffer_pool);
			if (session_ptr == NULL)
			{
				_RLOG_(MERROR, "tcp_client create_session make_shared failed");
				return tcp_client_session_ptr();
			}

			recv_bridge->session = session_ptr;
			session_ptr->set_close_handler(
				boost::bind(&tcp_client_impl::finish_session_close, this, onclose_handler, session_ptr, _1));
			session_ptr->start();
			VMPEND
			return session_ptr;
		}

		void tcp_client_impl::init_options()
		{
			options_container::set_option(tcp_client::options::max_packet_size(8*1024),true);
			options_container::set_option(tcp_client::options::send_buffer_size(32*1024),true);
			options_container::set_option(tcp_client::options::recv_buffer_size(16*1024),true);
			options_container::set_option(tcp_client::options::delaysending_size_threshold(0),true);
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
			GET_OPTION(m_session_option,max_packet_size)
			GET_OPTION(m_session_option,send_buffer_size)
			GET_OPTION(m_session_option,recv_buffer_size)
			GET_OPTION(m_session_option,delaysending_size_threshold)

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
#undef GET_OPTION
	}
}
