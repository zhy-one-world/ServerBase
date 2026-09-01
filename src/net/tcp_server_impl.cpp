/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:22
	file base:	tcp_server_impl
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "asio.hpp"
#include <boost/bind.hpp>
#include "scheduler.hpp"
#include "tcp_server_impl.hpp"
#include <rlog.hpp>
#include "mlb.hpp"
#include "persistence_id_generator.hpp"
#include "mem_pool.hpp"
#include <cstdlib>

using boost::asio::ip::tcp;

#pragma warning(disable:4355)
#pragma warning(disable:4503)

namespace faith
{
	namespace net 
	{
		static void call_serverstatus_handler(tcp_server::serverstatus_handler_type status_handler,boost::uint32_t instance_id,tcp_server::e_server_status_type status)
		{
			status_handler(status);
		}

		static void call_onconnected_handler(tcp_server::onconnected_handler_type onconnected_handler,boost::uint32_t instance_id,unsigned int connindex)
		{
			onconnected_handler(connindex);
		}

		static void call_onclose_handler(tcp_server::onclose_handler_type onclose_handler,boost::uint32_t instance_id,unsigned int connindex)
		{
			onclose_handler(connindex);
		}

		tcp_server_impl::~tcp_server_impl()
		{
			delete m_send_buffer_pool;
			delete m_recv_buffer_pool;
			common::persistence_id_generator::getInstance().return_id(_XTEXT("TCPServer"),m_instance_id);
		}

		tcp_server_impl::tcp_server_impl( 
			serverstatus_handler_type status_handler,
			onconnected_handler_type onconnected_handler,
			onclose_handler_type onclose_handler,
			recv_handler_type recv_handler,
			boost::asio::io_service &io_service,boost::asio::io_context::strand &strand,xstring ip,int tcp_port
			):
			m_io_service(io_service),
			m_strand(strand),
			m_acceptor(io_service),
			m_be_listening(false),
			m_conn_array(conn_array_size),
			m_options_applied(false),
			m_send_buffer_pool(NULL),
			m_recv_buffer_pool(NULL),
			m_scheduler_impl(*scheduler::getInstance().get_impl())
		{
			boost::asio::ip::address_v4	addr;

#if defined(FAITH_UNICODE)
			int temp_len = (ip.size() + 1)* 3; 
			char* temp = (char*)common::mem_pool::getInstance().alloc(temp_len);
			assert(temp);
			common::utility::_iconv_one("UCS-2LE",locale_charset(),(void*)ip.c_str(), (ip.size()+1)*sizeof(xchar), temp,temp_len);
			addr = boost::asio::ip::make_address_v4(temp);
			common::mem_pool::getInstance().free(temp,temp_len);
#else
			addr = boost::asio::ip::make_address_v4(ip);
#endif
			m_endpoint=tcp::endpoint( addr,tcp_port );

			m_instance_id = common::persistence_id_generator::getInstance().get_id(_XTEXT("TCPServer"));
			init_handlers(status_handler,onconnected_handler,onclose_handler,recv_handler);
			init_options();
			init_conn_index_lists();
		}

		tcp_server_impl::tcp_server_impl( 
			serverstatus_handler_type status_handler,
			onconnected_handler_type onconnected_handler,
			onclose_handler_type onclose_handler,
			recv_handler_type recv_handler,
			boost::asio::io_service &io_service,boost::asio::io_context::strand &strand,unsigned int tcp_port
			):
			m_io_service(io_service),
			m_strand(strand),
			m_acceptor( io_service ),
			m_be_listening( false ),
			m_conn_array(conn_array_size),
			m_options_applied(false),
			m_send_buffer_pool(NULL),
			m_recv_buffer_pool(NULL),
			m_scheduler_impl(*scheduler::getInstance().get_impl())
		{
			m_endpoint.port(tcp_port);

			m_instance_id = common::persistence_id_generator::getInstance().get_id(_XTEXT("TCPServer"));
			init_handlers(status_handler,onconnected_handler,onclose_handler,recv_handler);
			init_options();
			init_conn_index_lists();
		}

		void tcp_server_impl::init_conn_index_lists()
		{
			m_empty.clear();
			for (unsigned int i = 0; i < conn_array_size; ++i)
			{
				m_empty.push_back(i);
				m_conn_array[i].reset();
			}
		}

		void tcp_server_impl::call_onrecv_handler(tcp_server::onrecv_handler_type onrecv_handler, unsigned int connindex,const void *data_ptr,size_t data_len )
		{
			onrecv_handler(connindex,data_ptr,data_len);
		}

		tcp_server_session_ptr tcp_server_impl::get_session(unsigned int conn_index)
		{
			if (conn_index >= conn_array_size)
			{
				_RLOG_(MWARN, "tcp_server get_session invalid connindex:" << conn_index
					<< " array size:" << conn_array_size);
				return tcp_server_session_ptr();
			}
			return m_conn_array[conn_index];
		}

		void tcp_server_impl::release_session_index(unsigned int conn_index)
		{
			if (conn_index >= conn_array_size)
			{
				_RLOG_(MWARN, "tcp_server release_session_index invalid connindex:" << conn_index
					<< " array size:" << conn_array_size);
				return;
			}
			if (!m_conn_array[conn_index])
			{
				_RLOG_(MWARN, "tcp_server release_session_index session is null, connindex:" << conn_index);
				return;
			}
			m_conn_array[conn_index].reset();
			m_empty.push_back(conn_index);
		}

		void tcp_server_impl::listen()
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);

			tcp_server_session_ptr smart_ptr = create_session();
			if( smart_ptr == NULL )
			{
				_RLOG_(MERROR, "tcp_server listen create_session failed, use:"
					<< (conn_array_size - m_empty.size()) << " empty:" << m_empty.size());
				return;
			}
			m_acceptor.async_accept( smart_ptr->get_socket(),
				m_strand.wrap(boost::bind( &tcp_server_impl::handle_accept,this,smart_ptr,boost::asio::placeholders::error ) ));
		}

		bool tcp_server_impl::start( void )
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);

			apply_options();

			if( m_be_listening )
			{
				return false;
			}
			else
			{
				m_acceptor.open(m_endpoint.protocol());
				m_acceptor.set_option(tcp::acceptor::reuse_address(false));

				boost::system::error_code	error;

				m_acceptor.bind(m_endpoint,error);
				if(error)
				{
					return false;
				}

				m_acceptor.listen(boost::asio::socket_base::max_listen_connections,error);
				if(error)
				{
					return false;
				}

				m_be_listening = true;

				listen();
				return true;
			}
		}

		void tcp_server_impl::stop( bool wait_until_finished )
		{
			m_scheduler_impl.run_exclusive([this]()
			{
				boost::recursive_mutex::scoped_lock server_lock(m_mutex);
				if (m_be_listening)
				{
					m_be_listening = false;
					m_acceptor.close();
					for (unsigned int i = 0; i < conn_array_size; ++i)
					{
						tcp_server_session_ptr session = m_conn_array[i];
						if (session && session->been_opened())
						{
							session->close();
						}
					}
				}
			});
		}

		std::size_t	tcp_server_impl::get_conn_count( void )
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			return conn_array_size - m_empty.size();
		}

		xstring tcp_server_impl::get_ip_addr( unsigned int conn_index )
		{
			xstring ret;
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			tcp_server_session_ptr pSession = get_session(conn_index);
			if( pSession == NULL )
			{
				_RLOG_(MWARN, "tcp_server get_ip_addr session is null, connindex:" << conn_index);
			}
			else
			{
				const boost::asio::ip::tcp::endpoint& ep = pSession->get_remote_endpoint();

				xostringstream buf;
				buf << ep.address().to_string().c_str();
				ret = buf.str();
			}
			return ret;
		}

		unsigned short tcp_server_impl::get_ip_port( unsigned int conn_index )
		{
			unsigned short ret = 0;
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			tcp_server_session_ptr pSession = get_session(conn_index);
			if( pSession == NULL )
			{
				_RLOG_(MWARN, "tcp_server get_ip_port session is null, connindex:" << conn_index);
			}
			else
			{
				ret = pSession->get_remote_endpoint().port();
			}
			return ret;
		}

		unsigned int tcp_server_impl::get_session_thread_id( unsigned int conn_index )
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			tcp_server_session_ptr pSession = get_session(conn_index);
			if (pSession == NULL)
			{
				_RLOG_(MWARN, "tcp_server get_session_thread_id session is null, connindex:" << conn_index);
				return 0;
			}
			return pSession->get_thread_id();
		}

		void tcp_server_impl::handle_accept( tcp_server_session_ptr session_ptr,const boost::system::error_code& error )
		{
			if (nullptr == session_ptr)
			{
				_RLOG_(MERROR, "tcp_server handle_accept session_ptr is null, error:" << error.message());
				return;
			}
			if (!error)
			{	
				_RLOG_(MINFO, "tcp server accepted socket, connindex:"
					<< session_ptr->get_conn_index() << " session thread:"
					<< session_ptr->get_thread_id());
				m_scheduler_impl.run_exclusive([this, session_ptr]()
				{
					session_ptr->start();
					m_onconnected_handler(session_ptr->get_conn_index());
				});
				boost::recursive_mutex::scoped_lock server_lock(m_mutex);
				if (m_empty.empty())
				{
					_RLOG_(MWARN, "tcp_server handle_accept connection full, use:"
						<< (conn_array_size - m_empty.size()) << " array size:" << conn_array_size);
					return;
				}
				tcp_server_session_ptr new_session_ptr = create_session();
				if (new_session_ptr == NULL)
				{
					_RLOG_(MERROR, "tcp_server handle_accept create next session failed, use:"
						<< (conn_array_size - m_empty.size()) << " empty:" << m_empty.size());
					return;
				}
				m_acceptor.async_accept(new_session_ptr->get_socket(),
					m_strand.wrap(boost::bind(&tcp_server_impl::handle_accept, this, new_session_ptr, boost::asio::placeholders::error)));
			}
			else
			{
				session_ptr->close();
				{
					boost::recursive_mutex::scoped_lock server_lock(m_mutex);
					release_session_index(session_ptr->get_conn_index());
				}
				if(m_be_listening)
				{
					listen();
				}
			}
		}

		void tcp_server_impl::handle_session_close( unsigned int conn_index,tcp_server_session* session_ptr )
		{
		}

		int	tcp_server_impl::inner_send( unsigned int conn_index,const void *data_ptr,size_t data_len)
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			tcp_server_session_ptr pSession = get_session(conn_index);
			if( pSession == NULL || pSession->been_opened() == false)
			{
				_RLOG_(MWARN, "tcp_server inner_send session unavailable, connindex:" << conn_index
					<< " null:" << (pSession == NULL)
					<< " opened:" << (pSession && pSession->been_opened()));
				return 0;
			}
			else
			{
				return pSession->send( data_ptr,data_len );
			}	
		}
		int	tcp_server_impl::mlb_send( unsigned int conn_index,const common::string_buffer & data )
		{
			return inner_send(conn_index,data.c_str(),data.length());
		}

		namespace
		{
			static const xstring sz_send(_XTEXT("TCPServer::send"));
		}
		int tcp_server_impl::send( unsigned int conn_index,const void *data_ptr,size_t data_len )
		{
			return inner_send(conn_index,data_ptr,data_len);
		}

		int tcp_server_impl::inner_send_multi(unsigned int conn_index,const datablock_queue_type& data_queue)
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			tcp_server_session_ptr pSession = get_session(conn_index);
			if( pSession == NULL || pSession->been_opened() == false)
			{
				_RLOG_(MWARN, "tcp_server inner_send_multi session unavailable, connindex:" << conn_index
					<< " null:" << (pSession == NULL)
					<< " opened:" << (pSession && pSession->been_opened()));
				return 0;
			}
			else
			{
				return pSession->send_multi(data_queue);
			}	
		}

		int tcp_server_impl::mlb_send_multi(unsigned int conn_index,const common::string_buffer & data_queue)
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
			static const xstring sz_send_multi(_XTEXT("TCPServer::send_multi"));
		}
		int tcp_server_impl::send_multi(unsigned int conn_index,const datablock_queue_type& data_queue)
		{
			{
				size_t data_len=0;
				for(datablock_queue_type::const_iterator it = data_queue.begin();it!=data_queue.end();++it)
					data_len+=it->second;
			}
			return inner_send_multi(conn_index,data_queue);
		}

		bool tcp_server_impl::close( unsigned int conn_index )
		{
			m_scheduler_impl.post(
				boost::bind(&tcp_server_impl::close_on_main, this, conn_index),
				0);
			return true;
		}

		void tcp_server_impl::close_on_main(unsigned int conn_index)
		{
			m_scheduler_impl.run_exclusive([this, conn_index]()
			{
				boost::recursive_mutex::scoped_lock server_lock(m_mutex);
				tcp_server_session_ptr pSession = get_session(conn_index);
				if (pSession == NULL || !pSession->been_opened())
				{
					_RLOG_(MWARN, "tcp_server close_on_main session unavailable, connindex:" << conn_index
						<< " null:" << (pSession == NULL)
						<< " opened:" << (pSession && pSession->been_opened()));
					return;
				}

				pSession->close();
				const bool need_accept = m_empty.empty();
				finish_session_close(pSession, need_accept);
			});
		}

		tcp_server_session_ptr tcp_server_impl::create_session()
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			apply_options();

			if (m_empty.empty())
			{
				_RLOG_(MERROR, "tcp_server create_session empty list is empty, use:"
					<< (conn_array_size - m_empty.size()));
				return tcp_server_session_ptr();
			}
			if (m_send_buffer_pool == NULL || m_recv_buffer_pool == NULL)
			{
				_RLOG_(MERROR, "tcp_server create_session buffer pool is null, send:"
					<< (m_send_buffer_pool == NULL) << " recv:" << (m_recv_buffer_pool == NULL));
				return tcp_server_session_ptr();
			}

			const unsigned int conn_index = m_empty.front();
			m_empty.pop_front();

			const unsigned int thread_count = m_scheduler_impl.get_thread_count();
			const unsigned int worker_start_id = m_scheduler_impl.get_worker_thread_start_id();
			const unsigned int worker_count = thread_count > worker_start_id ? thread_count - worker_start_id : 0;
			const unsigned int thread_id = worker_count > 0 ? worker_start_id + (std::rand() % worker_count) : 0;

			tcp_server_session_ptr session_ptr = std::make_shared<tcp_server_session>(
				conn_index,
				thread_id,
				m_scheduler_impl.get_ioservice(thread_id),
				m_recv_handler,
				m_session_option,
				*m_send_buffer_pool,
				*m_recv_buffer_pool);
			if (session_ptr == NULL)
			{
				_RLOG_(MERROR, "tcp_server create_session make_shared failed, connindex:" << conn_index);
				m_empty.push_back(conn_index);
				return tcp_server_session_ptr();
			}
			session_ptr->set_conn_index(conn_index);
			session_ptr->set_close_handler(
				boost::bind(&tcp_server_impl::close, this, _1));
			m_conn_array[conn_index] = session_ptr;
			return session_ptr;
		}

		void tcp_server_impl::finish_session_close(tcp_server_session_ptr session, bool need_accept)
		{
			if (session == nullptr)
			{
				_RLOG_(MWARN, "tcp_server finish_session_close session is null, need_accept:" << need_accept);
				return;
			}
			const unsigned int conn_index = session->get_conn_index();
			if (m_onclose_handler)
			{
				m_onclose_handler(conn_index);
			}
			else
			{
				_RLOG_(MWARN, "tcp_server finish_session_close onclose_handler is null, connindex:" << conn_index);
			}
			release_session_index(conn_index);
			if (need_accept && m_be_listening)
			{
				boost::asio::post(m_strand, boost::bind(&tcp_server_impl::listen, this));
			}
		}

		void tcp_server_impl::clear_handlers()
		{
			m_status_handler = NULL;
			m_recv_handler = NULL;
			m_onclose_handler = NULL;
			m_onconnected_handler = NULL;
		}

		void tcp_server_impl::init_handlers(serverstatus_handler_type status_handler,onconnected_handler_type onconnected_handler,onclose_handler_type onclose_handler,recv_handler_type recv_handler)
		{
			m_status_handler = m_strand.wrap(boost::bind(call_serverstatus_handler,status_handler,m_instance_id,_1));
			m_onconnected_handler = boost::bind(call_onconnected_handler,onconnected_handler,m_instance_id,_1);
			m_recv_handler = boost::bind(&tcp_server_impl::call_onrecv_handler,this,recv_handler,_1,_2,_3);
			m_onclose_handler = boost::bind(call_onclose_handler,onclose_handler,m_instance_id,_1);
		}

		void tcp_server_impl::init_options()
		{
			options_container::set_option(tcp_server::options::connections_num_limit(8192),true);
			options_container::set_option(tcp_server::options::max_packet_size(8*1024),true);
			options_container::set_option(tcp_server::options::send_buffer_size(32*1024),true);
			options_container::set_option(tcp_server::options::recv_buffer_size(16*1024),true);
			options_container::set_option(tcp_server::options::delaysending_size_threshold(0),true);
		}
#define GET_OPTION(OBJ,OPTION)						\
	{												\
		tcp_server::options::OPTION opt;				\
		get_option(opt);							\
		OBJ.OPTION = opt.value;						\
	}

		void tcp_server_impl::apply_options()
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

		bool tcp_server_impl::set_option(const boost::any& option_item)
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

		bool tcp_server_impl::check_options()
		{
			tcp_session_option options;
			GET_OPTION(options,max_packet_size)
			GET_OPTION(options,send_buffer_size)
			GET_OPTION(options,recv_buffer_size)
			GET_OPTION(options,delaysending_size_threshold)
			return options.check_options();
		}

		void tcp_server_impl::create_buffer_pools()
		{
			unsigned int max_real_packet_size = m_session_option.max_packet_size + sizeof(tcp_pak_header);

			m_send_buffer_pool = new (std::nothrow) send_buffer_pool_type(m_session_option.send_buffer_size);
			if(m_send_buffer_pool == NULL)
			{
				_RLOG_(MERROR, "tcp_server create_buffer_pools send_buffer_pool is null, size:"
					<< m_session_option.send_buffer_size);
			}

			m_recv_buffer_pool = new (std::nothrow) recv_buffer_pool_type(m_session_option.recv_buffer_size+max_real_packet_size);
			if(m_recv_buffer_pool == NULL)
			{
				_RLOG_(MERROR, "tcp_server create_buffer_pools recv_buffer_pool is null, size:"
					<< (m_session_option.recv_buffer_size + max_real_packet_size));
			}
		}
	}
}
