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
#include <boost/pool/object_pool.hpp>
#include "scheduler.hpp"
#include "tcp_server_impl.hpp"
#include <rlog.hpp>
//#include "faith_logger.hpp"
#include "mlb.hpp"
#include "persistence_id_generator.hpp"
#include "mem_pool.hpp"

using boost::asio::ip::tcp;

#pragma warning(disable:4355)
#pragma warning(disable:4503)

namespace faith
{
	namespace net 
	{
		//
		//	Implemention of TCPServer_impl
		//
		static void call_serverstatus_handler(tcp_server::serverstatus_handler_type status_handler,boost::uint32_t instance_id,tcp_server::e_server_status_type status)
		{
			status_handler(status);
		}

		static void recur_serverstatus_handler(tcp_server::serverstatus_handler_type status_handler,boost::uint32_t status)
		{
			status_handler(static_cast<tcp_server::e_server_status_type>(status));
		}

		static void call_onconnected_handler(tcp_server::onconnected_handler_type onconnected_handler,boost::uint32_t instance_id,unsigned int connindex)
		{
			onconnected_handler(connindex);
		}

		static void call_onclose_handler(tcp_server::onclose_handler_type onclose_handler,boost::uint32_t instance_id,unsigned int connindex)
		{
			onclose_handler(connindex);
		}

		static void recur_onrecv_handler(tcp_server::onrecv_handler_type onrecv_handler,unsigned int connindex,const xstring & data )
		{
			onrecv_handler(connindex,data.c_str(),data.length());
		}

		static bool call_plugin(plug_in plugin,boost::shared_ptr<int> slot,boost::uint32_t instance_id,unsigned int connindex,const void *data_ptr,size_t data_len )
		{
			bool ret=plugin(connindex,data_ptr,data_len);
			return ret;
		}

		static bool in_tcpserver_remove_plugin = false;
		static void plugin_removed_handler(boost::uint32_t instance_id,unsigned int connindex,int slot)
		{
		}

		static void recur_plugin(plug_in plugin,unsigned int connindex,const xstring & data )
		{
			plugin(connindex,data.c_str(),data.length());
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
			m_session_deallocator(m_strand.wrap(boost::bind(&tcp_server_impl::destroy_session,this,_1))),
			m_sessions_pool(),
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
			//opc_create_counter_fun();
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
			m_session_deallocator(m_strand.wrap(boost::bind(&tcp_server_impl::destroy_session,this,_1))),
			m_sessions_pool(),
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

		void tcp_server_impl::call_onrecv_handler(tcp_server::onrecv_handler_type onrecv_handler, unsigned int connindex,const void *data_ptr,size_t data_len )
		{
			onrecv_handler(connindex,data_ptr,data_len);
		}


		void tcp_server_impl::listen()
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);

			tcp_server_session_ptr smart_ptr = create_session();
			if( smart_ptr == NULL )
			{
				return;
			}
			m_acceptor.async_accept( smart_ptr->get_socket(),
				m_strand.wrap(boost::bind( &tcp_server_impl::handle_accept,this,smart_ptr,boost::asio::placeholders::error ) ));
		}

		//MLB_CLASS_FUNC_0(bool,TCPServer_impl,start)
		bool tcp_server_impl::start( void )
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);

			tcp_server::options::connections_num_limit connections_num_limit;
			get_option(connections_num_limit);
			//the extra one is the listening session
			m_conn_index_generator.set_max_count(connections_num_limit.value+1);
			if( m_be_listening )
			{
				return false;
			}
			else
			{
				// NOT allow the acceptor to reuse the address (i.e. SO_REUSEADDR)
				m_acceptor.open(m_endpoint.protocol());
				m_acceptor.set_option(tcp::acceptor::reuse_address(false));

				boost::system::error_code	error;

				//	bind endpoint
				m_acceptor.bind(m_endpoint,error);
				if(error)
				{
					return false;
				}

				//	start listen
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

		//MLB_CLASS_FUNC_1(void,TCPServer_impl,stop,
		//	bool,wait_until_finished)
		void tcp_server_impl::stop( bool wait_until_finished )
		{
			/*if this fuction is called in main thread,the call_close_handler may not acctual
				executed the session's close_handler ,instead of posted by service by strand.
				So we only do some clear action at first  and then wait.
			*/
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			if (m_be_listening)
			{
				m_be_listening = false;
				m_acceptor.close();
				for (int i = 0; i < m_conn_max_size; ++i)
				{
					if (m_conn_array[i]->been_opened())
					{
						m_conn_array[i]->close();
					}
				}
			}
		}

		//MLB_CLASS_FUNC_0(std::size_t,TCPServer_impl,get_conn_count)
		std::size_t	tcp_server_impl::get_conn_count( void )
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			return m_conn_size;
		}

		//MLB_CLASS_FUNC_1(xstring,TCPServer_impl,get_ip_addr,
		//	unsigned int,conn_index)
		xstring tcp_server_impl::get_ip_addr( unsigned int conn_index )
		{
			xstring ret;
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			if (conn_index < 0 || conn_index >= m_conn_max_size)
			{
				return ret;
			}
			tcp_server_session* pSession = m_conn_array[conn_index];
			if( pSession == NULL )
			{
			}
			else
			{
				const boost::asio::ip::tcp::endpoint& ep = pSession->get_remote_endpoint();

				/* TODO: 这里假设ip地址串中不包含中文等字符 */
				xostringstream buf;
				buf << ep.address().to_string().c_str();
				ret = buf.str();
			}
			return ret;
		}

		//MLB_CLASS_FUNC_1(unsigned short,TCPServer_impl,get_ip_port,
		//	unsigned int,conn_index)
		unsigned short tcp_server_impl::get_ip_port( unsigned int conn_index )
		{
			unsigned short ret;
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			if (conn_index < 0 || conn_index >= m_conn_max_size)
			{
				return 0;
			}
			tcp_server_session* pSession = m_conn_array[conn_index];
			if( pSession == NULL )
			{

			}
			else
			{
				ret = pSession->get_remote_endpoint().port();
			}
			return ret;
		}

		void tcp_server_impl::handle_accept( tcp_server_session_ptr session_ptr,const boost::system::error_code& error )
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			if (nullptr == session_ptr)
			{
				return;
			}
			//std::cout << "tcp_server_impl::handle_accept conn_index = " << session_ptr->get_conn_index() << " error = " << error << " m_conn_size ="<< m_conn_size<<std::endl;
			if (!error)
			{	
				_RLOG_(MINFO, "tcp server accepted socket, connindex:"
					<< session_ptr->get_conn_index() << " session thread:"
					<< session_ptr->get_thread_id());
				session_ptr->start();
				m_scheduler_impl.inner_post(boost::bind(m_onconnected_handler, session_ptr->get_conn_index()), session_ptr->get_thread_id());
				m_conn_size++;
				if (m_conn_size >= m_conn_max_size)
				{
					return;
				}
				tcp_server_session_ptr new_session_ptr = create_session();
				m_acceptor.async_accept(new_session_ptr->get_socket(),
					m_strand.wrap(boost::bind(&tcp_server_impl::handle_accept, this, new_session_ptr, boost::asio::placeholders::error)));
			}
			else
			{
				session_ptr->close();
				if(m_be_listening)
				{
					listen();
				}
			}
		}

		void tcp_server_impl::handle_session_close( unsigned int conn_index,tcp_server_session* session_ptr )
		{
			//std::cout << "tcp_server_impl::handle_session_close conn_index =" << conn_index << std::endl;
		}

		int	tcp_server_impl::inner_send( unsigned int conn_index,const void *data_ptr,size_t data_len)
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			if (conn_index < 0 || conn_index >= m_conn_max_size)
			{
				return 0;
			}
			tcp_server_session* pSession = m_conn_array[conn_index];
			if( pSession == NULL || pSession->been_opened() == false)
			{
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
			if (conn_index < 0 || conn_index >= m_conn_max_size)
			{
				return 0;
			}
			tcp_server_session* pSession = m_conn_array[conn_index];
			if( pSession == NULL || pSession->been_opened() == false)
			{
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
			//++(*m_send_instance);
			{
				size_t data_len=0;
				for(datablock_queue_type::const_iterator it = data_queue.begin();it!=data_queue.end();++it)
					data_len+=it->second;
				//(*m_send_bytes_instance) += data_len;
			}
			return inner_send_multi(conn_index,data_queue);
		}

		//MLB_CLASS_FUNC_1(bool,TCPServer_impl,close,
		//	unsigned int , conn_index )
		bool tcp_server_impl::close( unsigned int conn_index )
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			if (conn_index < 0 || conn_index >= m_conn_max_size)
			{
				return false;
			}
			tcp_server_session* pSession = m_conn_array[conn_index];
			if( pSession == NULL )
			{
				return false;
			}
			else
			{
				pSession->close();
				return true;
			}
		}

		tcp_server_session_ptr tcp_server_impl::create_session()
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			for (int i = 0; i < m_conn_max_size; ++i)
			{
				if (m_conn_array[i]->get_data_use() == false)
				{
					tcp_server_session_ptr smart_ptr = tcp_server_session_ptr(m_conn_array[i], m_session_deallocator);
					smart_ptr->set_data_use(true);
					return smart_ptr;
				}
			}
			return tcp_server_session_ptr();
		}

		void tcp_server_impl::destroy_session(tcp_server_session * session)
		{
			boost::recursive_mutex::scoped_lock server_lock(m_mutex);
			unsigned conn_index = session->get_conn_index();
			bool been_opened = session->been_opened();

			//std::cout << "tcp_server_impl::destroy_session conn_index =" << conn_index << " m_conn_size ="<< m_conn_size <<std::endl;
			session->close();
			if (been_opened)
			{
				m_conn_size--;
			}
			const bool need_accept = been_opened && m_conn_size == m_conn_max_size - 1;
			m_scheduler_impl.inner_post(boost::bind(&tcp_server_impl::finish_session_close, this, session, need_accept), session->get_thread_id());

			if (m_conn_size <= 0)
			{
				//m_scheduler_impl.inner_post(boost::bind(m_status_handler, tcp_server::e_ss_all_connection_closed));
				//clear_handlers();
			}
		}

		void tcp_server_impl::finish_session_close(tcp_server_session* session, bool need_accept)
		{
			if (session == nullptr)
			{
				return;
			}
			session->set_data_use(false);
			if (m_onclose_handler)
			{
				m_onclose_handler(session->get_conn_index());
			}
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
		void tcp_server_impl::init_client_server(unsigned int server_num)
		{
			m_conn_max_size = server_num;
			m_conn_array = new tcp_server_session*[m_conn_max_size];
			apply_options();
			const unsigned int thread_count = m_scheduler_impl.get_thread_count();
			const unsigned int worker_start_id = m_scheduler_impl.get_worker_thread_start_id();
			const unsigned int worker_count = thread_count > worker_start_id ? thread_count - worker_start_id : 0;
			for (int i = 0; i < m_conn_max_size; ++i)
			{
				const unsigned int thread_id = worker_count > 0 ? worker_start_id + (std::rand() % worker_count) : 0;
				m_conn_array[i] = new tcp_server_session(0, thread_id, m_scheduler_impl.get_ioservice(thread_id), m_recv_handler,
					m_session_option, *m_send_buffer_pool, *m_recv_buffer_pool);
				m_conn_array[i]->set_conn_index(i);
			}
			m_conn_size = 0;
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
//			GET_OPTION(m_session_option,tcp_nodelay)
			GET_OPTION(m_session_option,max_packet_size)
			GET_OPTION(m_session_option,send_buffer_size)
			GET_OPTION(m_session_option,recv_buffer_size)
			GET_OPTION(m_session_option,delaysending_size_threshold)
//			GET_OPTION(m_session_option,delaysending_time_threshold)
//			GET_OPTION(m_session_option,close_overflowed_session)
// 			if(m_session_option.delaysending_size_threshold > 0 && m_session_option.tcp_nodelay == false)
// 			{
// 				options_container::set_option(tcp_server::options::tcp_nodelay(true));
// 				m_session_option.tcp_nodelay = true;
// 			}

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

			}

			m_recv_buffer_pool = new (std::nothrow) recv_buffer_pool_type(m_session_option.recv_buffer_size+max_real_packet_size);
			if(m_recv_buffer_pool == NULL)
			{

			}
		}
	}
}
