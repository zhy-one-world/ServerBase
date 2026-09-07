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
		namespace
		{
			void call_serverstatus_handler(tcp_server::serverstatus_handler_type status_handler,boost::uint32_t instance_id,tcp_server::e_server_status_type status)
			{
				(void)instance_id;
				status_handler(status);
			}

			void call_onconnected_handler(tcp_server::onconnected_handler_type onconnected_handler,boost::uint32_t instance_id,tcp_server_session_ptr session)
			{
				(void)instance_id;
				onconnected_handler(session);
			}

			void call_onclose_handler(tcp_server::onclose_handler_type onclose_handler,boost::uint32_t instance_id,tcp_server_session_ptr session)
			{
				(void)instance_id;
				onclose_handler(session);
			}

			void call_onrecv_handler(tcp_server::onrecv_handler_type onrecv_handler,tcp_server_session_ptr session,const void *data_ptr,size_t data_len)
			{
				onrecv_handler(session,data_ptr,data_len);
			}

			struct recv_handler_bridge
			{
				tcp_server::onrecv_handler_type user_handler;
				tcp_server_session_ptr session;

				void on_recv(const void* data_ptr, size_t data_len) const
				{
					if (session)
					{
						call_onrecv_handler(user_handler, session, data_ptr, data_len);
					}
				}
			};
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
			m_conn_count(0),
			m_connections_limit(8192),
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
			m_conn_count(0),
			m_connections_limit(8192),
			m_options_applied(false),
			m_send_buffer_pool(NULL),
			m_recv_buffer_pool(NULL),
			m_scheduler_impl(*scheduler::getInstance().get_impl())
		{
			m_endpoint.port(tcp_port);

			m_instance_id = common::persistence_id_generator::getInstance().get_id(_XTEXT("TCPServer"));
			init_handlers(status_handler,onconnected_handler,onclose_handler,recv_handler);
			init_options();
		}

		bool tcp_server_impl::is_session_capacity_full() const
		{
			return m_conn_count >= m_connections_limit;
		}

		void tcp_server_impl::release_session_count()
		{
			if (m_conn_count > 0)
			{
				--m_conn_count;
			}
		}

		void tcp_server_impl::listen()
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);

			tcp_server_session_ptr smart_ptr = create_session();
			if( smart_ptr == NULL )
			{
				_RLOG_(MERROR, "tcp_server listen create_session failed, use:"
					<< m_conn_count << " limit:" << m_connections_limit);
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

		std::size_t	tcp_server_impl::get_conn_count( void )
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			return m_conn_count;
		}

		xstring tcp_server_impl::get_ip_addr( const tcp_server_session_ptr& session )
		{
			xstring ret;
			if( session == NULL )
			{
				_RLOG_(MWARN, "tcp_server get_ip_addr session is null");
			}
			else
			{
				const boost::asio::ip::tcp::endpoint& ep = session->get_remote_endpoint();

				xostringstream buf;
				buf << ep.address().to_string().c_str();
				ret = buf.str();
			}
			return ret;
		}

		unsigned short tcp_server_impl::get_ip_port( const tcp_server_session_ptr& session )
		{
			unsigned short ret = 0;
			if( session == NULL )
			{
				_RLOG_(MWARN, "tcp_server get_ip_port session is null");
			}
			else
			{
				ret = session->get_remote_endpoint().port();
			}
			return ret;
		}

		unsigned int tcp_server_impl::get_session_thread_id( const tcp_server_session_ptr& session )
		{
			if (session == NULL)
			{
				_RLOG_(MWARN, "tcp_server get_session_thread_id session is null");
				return 0;
			}
			return session->get_thread_id();
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
				_RLOG_(MINFO, "tcp server accepted socket, session thread:"
					<< session_ptr->get_thread_id());
				m_scheduler_impl.run_exclusive([this, session_ptr]()
				{
					session_ptr->start();
					m_onconnected_handler(session_ptr);
				});
				boost::recursive_mutex::scoped_lock server_lock(m_mutex);
				if (is_session_capacity_full())
				{
					_RLOG_(MWARN, "tcp_server handle_accept connection full, use:"
						<< m_conn_count << " limit:" << m_connections_limit);
					return;
				}
				tcp_server_session_ptr new_session_ptr = create_session();
				if (new_session_ptr == NULL)
				{
					_RLOG_(MERROR, "tcp_server handle_accept create next session failed, use:"
						<< m_conn_count << " limit:" << m_connections_limit);
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
					release_session_count();
				}
				if(m_be_listening)
				{
					listen();
				}
			}
		}

		void tcp_server_impl::handle_session_close(tcp_server_session_ptr session)
		{
			if (session == nullptr)
			{
				_RLOG_(MWARN, "tcp_server handle_session_close session is null");
				return;
			}
			m_scheduler_impl.post(
				boost::bind(&tcp_server_impl::close_on_main, this, session),
				0);
		}

		int	tcp_server_impl::inner_send( const tcp_server_session_ptr& session,const void *data_ptr,size_t data_len)
		{
			if( session == NULL || session->been_opened() == false)
			{
				_RLOG_(MWARN, "tcp_server inner_send session unavailable"
					<< " null:" << (session == NULL)
					<< " opened:" << (session && session->been_opened()));
				return 0;
			}
			else
			{
				return session->send( data_ptr,data_len );
			}	
		}

		int tcp_server_impl::send( const tcp_server_session_ptr& session,const void *data_ptr,size_t data_len )
		{
			return inner_send(session,data_ptr,data_len);
		}

		int tcp_server_impl::inner_send_multi(const tcp_server_session_ptr& session,const datablock_queue_type& data_queue)
		{
			if( session == NULL || session->been_opened() == false)
			{
				_RLOG_(MWARN, "tcp_server inner_send_multi session unavailable"
					<< " null:" << (session == NULL)
					<< " opened:" << (session && session->been_opened()));
				return 0;
			}
			else
			{
				return session->send_multi(data_queue);
			}	
		}

		int tcp_server_impl::send_multi(const tcp_server_session_ptr& session,const datablock_queue_type& data_queue)
		{
			{
				size_t data_len=0;
				for(datablock_queue_type::const_iterator it = data_queue.begin();it!=data_queue.end();++it)
					data_len+=it->second;
				(void)data_len;
			}
			return inner_send_multi(session,data_queue);
		}

		bool tcp_server_impl::close( const tcp_server_session_ptr& session )
		{
			if (session == nullptr)
			{
				_RLOG_(MWARN, "tcp_server close session unavailable");
				return false;
			}
			m_scheduler_impl.post(
				boost::bind(&tcp_server_impl::close_on_main, this, session),
				0);
			return true;
		}

		void tcp_server_impl::close_on_main(tcp_server_session_ptr session)
		{
			m_scheduler_impl.run_exclusive([this, session]()
			{
				boost::recursive_mutex::scoped_lock server_lock(m_mutex);
				if (session == nullptr || !session->been_opened())
				{
					_RLOG_(MWARN, "tcp_server close_on_main session unavailable"
						<< " null:" << (session == nullptr)
						<< " opened:" << (session && session->been_opened()));
					return;
				}

				session->close();
				const bool need_accept = is_session_capacity_full();
				finish_session_close(session, need_accept);
			});
		}

		tcp_server_session_ptr tcp_server_impl::create_session()
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			apply_options();

			if (is_session_capacity_full())
			{
				_RLOG_(MERROR, "tcp_server create_session capacity full, use:"
					<< m_conn_count << " limit:" << m_connections_limit);
				return tcp_server_session_ptr();
			}
			if (m_send_buffer_pool == NULL || m_recv_buffer_pool == NULL)
			{
				_RLOG_(MERROR, "tcp_server create_session buffer pool is null, send:"
					<< (m_send_buffer_pool == NULL) << " recv:" << (m_recv_buffer_pool == NULL));
				return tcp_server_session_ptr();
			}

			const unsigned int thread_count = m_scheduler_impl.get_thread_count();
			const unsigned int worker_start_id = m_scheduler_impl.get_worker_thread_start_id();
			const unsigned int worker_count = thread_count > worker_start_id ? thread_count - worker_start_id : 0;
			const unsigned int thread_id = worker_count > 0 ? worker_start_id + (std::rand() % worker_count) : 0;

			const std::shared_ptr<recv_handler_bridge> recv_bridge = std::make_shared<recv_handler_bridge>();
			recv_bridge->user_handler = m_recv_handler;

			typedef boost::function<void(const void*,size_t)> internal_recv_handler_type;
			internal_recv_handler_type wrapped_recv = boost::bind(
				&recv_handler_bridge::on_recv,
				recv_bridge,
				_1,
				_2);

			tcp_server_session_ptr session_ptr = std::make_shared<tcp_server_session>(
				thread_id,
				m_scheduler_impl.get_ioservice(thread_id),
				wrapped_recv,
				m_session_option,
				*m_send_buffer_pool,
				*m_recv_buffer_pool);
			if (session_ptr == NULL)
			{
				_RLOG_(MERROR, "tcp_server create_session make_shared failed");
				return tcp_server_session_ptr();
			}
			recv_bridge->session = session_ptr;
			session_ptr->set_close_handler(
				boost::bind(&tcp_server_impl::handle_session_close, this, session_ptr));
			++m_conn_count;
			return session_ptr;
		}

		void tcp_server_impl::finish_session_close(tcp_server_session_ptr session, bool need_accept)
		{
			if (session == nullptr)
			{
				_RLOG_(MWARN, "tcp_server finish_session_close session is null, need_accept:" << need_accept);
				return;
			}
			if (m_onclose_handler)
			{
				m_onclose_handler(session);
			}
			else
			{
				_RLOG_(MWARN, "tcp_server finish_session_close onclose_handler is null");
			}
			release_session_count();
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
			m_recv_handler = recv_handler;
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
			{
				tcp_server::options::connections_num_limit opt;
				get_option(opt);
				m_connections_limit = opt.value;
			}

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
