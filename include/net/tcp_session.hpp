/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:58
	file base:	tcp_session
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _TCP_SESSION_H_
#define _TCP_SESSION_H_

//#include "faith_logger.hpp"
#include "asio.hpp"
#include "tcp_pak_def.hpp"
#include "recv_buffer.hpp"
#include "scheduler_impl.hpp"
#include "tcp_session_option.hpp"
#include "send_buffer.hpp"
#include "plugin.hpp"
#include "datablock.hpp"
#include "delay_send_session.hpp"
#include "memory_pool.hpp"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/utility.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include <boost/detail/atomic_count.hpp>


#pragma warning(push)
#pragma warning(disable:4996)

namespace faith 
{
	namespace net 
	{
		typedef memory_pool	send_buffer_pool_type;
		typedef memory_pool	recv_buffer_pool_type;
		template < typename T >
		class tcp_session :
			private boost::noncopyable,
			private delay_send_session,
			public boost::enable_shared_from_this<T>
		{
			// handler
			typedef boost::function<void(unsigned int,const void*,size_t)>
				recv_handler_type;
			typedef boost::function<void(unsigned int,int)>
				plugin_removed_handler_type;

			recv_handler_type			m_recv_handler;				// invoke it while get packet
			plugin_removed_handler_type	m_plugin_removed_handler;	// invoke it while plugin removed

			boost::asio::ip::tcp::socket	m_socket;

			unsigned int			m_conn_index;			//	connection index
			bool					m_been_opened;			//	true if the socket had been opened
			bool					m_is_use;			//	true if the socket had been opened

			// buffers
			boost::recursive_mutex	m_sendbuf_mutex;		//	for m_send_buf's res. protection
			boost::recursive_mutex	m_recvbuf_mutex;		//	for m_recv_buf's res. protection
			
	
			typedef recv_buffer					recv_buffer_type;
			typedef send_buffer					send_buffer_type;
			recv_buffer_type 					m_recv_buf;
			send_buffer_type					m_send_buf;

			void handle_read(const boost::system::error_code& error,size_t bytes_transferred);
			void handle_write(const boost::system::error_code& error,size_t bytes_transferred);
			void on_packet_received(int packet_count);
			void read();

			//发送延时数据
			void send_delay_data();
			//异步发送数据
			void async_send( const void* data_ptr, size_t data_size );
			//由于时间到期发送数据
			bool send_data_by_time();
			//由于大小达到一定数值,发送数据
			bool send_data_by_size();

			boost::asio::ip::tcp::endpoint		m_local_endpoint;
			boost::asio::ip::tcp::endpoint		m_remote_endpoint;
			unsigned int						m_max_packet_size;
			tcp_session_option &					m_option;
			bool								m_reading;
			bool								m_sending;
		public:
			
			tcp_session(
				unsigned int connindex,
				boost::asio::io_service &io_service,
				recv_handler_type handler_recv,
				tcp_session_option & option,
				send_buffer_pool_type & send_buffer_pool,
				recv_buffer_pool_type & recv_buffer_pool
				);
			void	init_data();
			boost::asio::ip::tcp::socket&	get_socket()		{	return m_socket;	}
			inline unsigned int		get_conn_index() { return m_conn_index; }
			inline void				set_conn_index(unsigned int conn_index) { m_conn_index = conn_index; }
			inline bool				get_data_use() { return m_is_use; }
			inline void				set_data_use(bool is_use) { m_is_use = is_use; }

			bool	send( const void* data_ptr, size_t data_size );
			bool	send_multi(const datablock_queue_type& data_queue);
			void	close();
			bool	been_opened();

			const boost::asio::ip::tcp::endpoint& get_remote_endpoint()	{	return m_remote_endpoint;	}
		protected:
			~tcp_session();
			void	open();
		};		

		template < typename T >
		tcp_session<T>::tcp_session(
			unsigned int connindex,
			boost::asio::io_service &io_service,
			recv_handler_type handler_recv,
			tcp_session_option & option,
			send_buffer_pool_type & send_buffer_pool,
			recv_buffer_pool_type & recv_buffer_pool
			):
			m_conn_index(connindex),
			m_socket(io_service),
			m_recv_handler(handler_recv),
			m_recv_buf(option.recv_buffer_size,option.max_packet_size,recv_buffer_pool),
			m_been_opened(false),
			m_is_use(false),
			m_option(option),
			m_send_buf(option.send_buffer_size,option.delaysending_size_threshold,send_buffer_pool),
			m_reading(false),
			m_sending(false),
			delay_send_session(0/*option.delaysending_time_threshold/delay_send_queue::timer_interval*/)
		{
		}

		template < typename T >
		bool tcp_session<T>::been_opened()
		{
			return m_been_opened;
		}
		template < typename T >
		void tcp_session<T>::init_data()
		{
			m_been_opened = false;
			m_reading = false;
			m_sending = false;
		}
		template < typename T >
		void tcp_session<T>::open()
		{
			m_recv_buf.clear_data();
			m_send_buf.clear_data();
			m_been_opened = true;
			m_reading = false;
			m_sending = false;

			boost::system::error_code ec;
			m_local_endpoint = m_socket.local_endpoint(ec);
			m_remote_endpoint = m_socket.remote_endpoint(ec);
			
			//Socket option for disabling the Nagle algorithm.
			boost::asio::ip::tcp::no_delay option(false);
			m_socket.set_option(option,ec);

#ifdef FAITH_NET_RECVBUFFER_OMLB_ENABLED
			xostringstream recv_buf_logfile;

			recv_buf_logfile 
				<< _XTEXT("omlb/RecvBuffer/")
				<< _XTEXT("(") << m_local_endpoint.address() << _XTEXT(",") << m_local_endpoint.port() << _XTEXT(")")
				<< _XTEXT("-")
				<< _XTEXT("(") << m_remote_endpoint.address() << _XTEXT(",") << m_remote_endpoint.port() << _XTEXT(")")
				<< _XTEXT(".box");
			m_recv_buf.omlb_run(recv_buf_logfile.str(),::faith::MM_LOGGING);
			m_recv_buf.init(m_option.recv_buffer_size,m_option.max_packet_size);
#endif
			read();

 			////FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("tcp session open")<<	_XTEXT("\n\t")
 			//	<< _XTEXT("conn_index:") << get_conn_index() << _XTEXT("\n\t")
				//<< _XTEXT("m_send_buf total_size:") << m_send_buf.get_total_size() << _XTEXT("\n\t")
				//<< _XTEXT("m_send_buf buff_size:") << m_send_buf.get_buff_size() << _XTEXT("\n\t")
				//<< _XTEXT(" local ip addr:(") << m_local_endpoint.address().to_string().c_str() << _XTEXT(",") << m_local_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
				//<< _XTEXT(" remote ip addr:(") << m_remote_endpoint.address().to_string().c_str() << _XTEXT(",") << m_remote_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
				//);
		}

		template < typename T >
		void tcp_session<T>::close()
		{
			boost::system::error_code ec;
			if (m_been_opened == false)
			{
				return;
			}
			m_been_opened = false;
			//m_reading = false;
			//m_sending = false;
			if( !m_socket.is_open() )
				return;

			//FAITH_LOG_DEBUG(scheduler::getInstance().get_logger(),_XTEXT("TCPSession::close()") << _XTEXT("\n\t")
			//	<< _XTEXT("conn_index:") << get_conn_index() << _XTEXT("\n\t")
			//	<< _XTEXT(" local ip addr:(") << m_local_endpoint.address().to_string().c_str() << _XTEXT(",") << m_local_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
			//	<< _XTEXT(" remote ip addr:(") << m_remote_endpoint.address().to_string().c_str() << _XTEXT(",") << m_remote_endpoint.port() << _XTEXT(")") );

			//	for gracefull
			m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,ec);
			if( ec )
			{
				//FAITH_LOG_DEBUG(scheduler::getInstance().get_logger(),_XTEXT("TCPSession::close() error: ") << _asio_message(ec.message())
				//	<< _XTEXT(" file: ") << __XFILE__ << _XTEXT(" line: ") << __LINE__ );
			}

			//m_socket.cancel(ec);
			//if( ec )
			//{
			//	//FAITH_LOG_DEBUG(Scheduler::getInstance().get_logger(),_XTEXT("TCPSession::close() error: ") << _asio_message(ec.message())
			//		<< _XTEXT(" file: ") << __XFILE__ << _XTEXT(" line: ") << __LINE__ );
			//}			

			//	Cancel all asynchronous operations associated with the socket.			
			m_socket.close(ec);
			if( ec )
			{
				//FAITH_LOG_DEBUG(scheduler::getInstance().get_logger(),_XTEXT("TCPSession::close() error: ") << _asio_message(ec.message())
				//	<< _XTEXT(" file: ") << __XFILE__ << _XTEXT(" line: ") << __LINE__ );
			}
		}

		template < typename T >
		void tcp_session<T>::handle_read(const boost::system::error_code& error,size_t bytes_transferred)
		{
			if (m_been_opened == false)
			{
				return;
			}
			if(error)
			{				
				//std::cout << "tcp_session<T>::handle_read error = " << error << std::endl;
				////FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),__XFUNCTION__ << _XTEXT(" info: ") << _asio_message(error.message()) << _XTEXT(" file: ") << __XFILE__ << _XTEXT(" line: ") << __LINE__ );
				return;
			}

			int packet_count;
			{
				//	for m_recv_buf's res. protection
				//	deny potential re-entered behavior
				boost::recursive_mutex::scoped_lock recvbuf_lock(m_recvbuf_mutex);
				packet_count = m_recv_buf.push_data(bytes_transferred);
				m_reading = false;
				read();
			}
			if(packet_count == -1)
			{				
				std::cout << _XTEXT("TCPSession::handle_read() invalid packet-header received!") << _XTEXT("\n\t")
					<< _XTEXT("conn_index:") << get_conn_index() << _XTEXT("\n\t")
					<< _XTEXT(" local ip addr:(") << m_local_endpoint.address().to_string().c_str() << _XTEXT(",") << m_local_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
					<< _XTEXT(" remote ip addr:(") << m_remote_endpoint.address().to_string().c_str() << _XTEXT(",") << m_remote_endpoint.port() << _XTEXT(")") << std::endl;
				return;
			}
			if(packet_count > 0)
			{
				scheduler::getInstance().get_impl()->inner_post(
                    boost::bind(&T::on_packet_received, this->shared_from_this(), packet_count)
					);
			}
		}

		template < typename T >
		inline void tcp_session<T>::async_send( const void* data_ptr, size_t data_size )
		{
			if (m_been_opened == false)
			{
				m_send_buf.clear_data();
				return;
			}
			m_sending = true;
			set_send_time();
			try
			{
				boost::asio::async_write( 
					m_socket,
					boost::asio::buffer(data_ptr,data_size),
					boost::asio::transfer_at_least(1),
					boost::bind(&T::handle_write, this->shared_from_this(), boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred)
					);
			}
			catch (const boost::bad_weak_ptr &)
			{
				////FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),__XFUNCTION__ << _XTEXT(" failed. "));
				boost::recursive_mutex::scoped_lock sendbuf_lock(m_sendbuf_mutex);
				m_send_buf.pop(data_size);
				m_sending = false;
			}
		}

		template < typename T >
		inline bool tcp_session<T>::send_data_by_time()
		{
			//if(check_send_time())
			{
				unsigned int send_size;
				const void * send_data = m_send_buf.header_data(send_size);
				if(send_data)
				{
					async_send(send_data,send_size);
					return true;
				}
			}
			return false;
		}

		template < typename T >
		inline bool tcp_session<T>::send_data_by_size()
		{
			unsigned int send_size;
			const void * send_data = m_send_buf.nagle_data(send_size);
			if(send_data)
			{
				async_send(send_data,send_size);
				return true;
			}
			else
			{
				return false;
			}
		}

		template < typename T >
		void tcp_session<T>::handle_write(const boost::system::error_code& error,size_t bytes_transferred)
		{
			if (m_been_opened == false)
			{
				return;
			}
			if(error)
			{
				//std::cout << "tcp_session<T>::handle_writ error ="<< error << std::endl;
				////FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),__XFUNCTION__ << _XTEXT(" info: ") << _asio_message(error.message()) << _XTEXT(" file: ") << __XFILE__ << _XTEXT(" line: ") << __LINE__ );
				if (bytes_transferred > 0)
				{
					boost::recursive_mutex::scoped_lock sendbuf_lock(m_sendbuf_mutex);
					m_send_buf.pop(bytes_transferred);
				}
				m_sending = false;
				return;
			}
			
			boost::recursive_mutex::scoped_lock sendbuf_lock(m_sendbuf_mutex);

			m_send_buf.pop(bytes_transferred);
			m_sending = false;

			send_data_by_size();
		}

		template < typename T >
		bool tcp_session<T>::send( const void* data_ptr, size_t data_size )
		{
			if (m_been_opened == false)
			{
				return false;
			}
			if(data_size > m_option.max_packet_size)
			{
				//FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("data_size > m_option.max_packet_size")<<	_XTEXT("\n\t")
				//	<< _XTEXT("conn_index:") << get_conn_index() << _XTEXT("\n\t")
				//	<< _XTEXT(" local ip addr:(") << m_local_endpoint.address().to_string().c_str() << _XTEXT(",") << m_local_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
				//	<< _XTEXT(" remote ip addr:(") << m_remote_endpoint.address().to_string().c_str() << _XTEXT(",") << m_remote_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
				//	<< _XTEXT(" data_size:")<<data_size<< _XTEXT("\n\t")
				//	<< _XTEXT(" max_packet_size:") << m_option.max_packet_size << _XTEXT("\n\t")
				//	);
				return false;
			}

			tcp_pak_header	header;
			header.reset();
			header.length = data_size;

			//	for m_send_buf's res. protection
			boost::recursive_mutex::scoped_lock sendbuf_lock(m_sendbuf_mutex);
			//	in case of failure when
			//	below m_send_buf.push( body_info );
			if( !m_send_buf.can_push( sizeof(tcp_pak_header)+data_size ) )
			{
				//FAITH_LOG_INFO(scheduler::getInstance().get_logger(),_XTEXT("send buffer overflowed")<<	_XTEXT("\n\t")
				//	<< _XTEXT("conn_index:") << get_conn_index() << _XTEXT("\n\t")
				//	<< _XTEXT("data_size:") << data_size << _XTEXT("\n\t")
				//	<< _XTEXT("total_size:") << m_send_buf.get_total_size() << _XTEXT("\n\t")
				//	<< _XTEXT("buff_size:") << m_send_buf.get_buff_size() << _XTEXT("\n\t")
				//	<< _XTEXT(" local ip addr:(") << m_local_endpoint.address().to_string().c_str() << _XTEXT(",") << m_local_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
				//	<< _XTEXT(" remote ip addr:(") << m_remote_endpoint.address().to_string().c_str() << _XTEXT(",") << m_remote_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
				//	);

#ifdef _DEBUG_NET_BUFFER
				//FAITH_LOG_RELEASE(scheduler::getInstance().get_logger(),_XTEXT("TCPSession::send() can't push!") << _XTEXT("\n\t")
// 					<< _XTEXT("conn_index:") << get_conn_index() << _XTEXT("\n\t")
// 					<< _XTEXT(" local ip addr:(") << m_local_endpoint.address().to_string().c_str() << _XTEXT(",") << m_local_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
// 					<< _XTEXT(" remote ip addr:(") << m_remote_endpoint.address().to_string().c_str() << _XTEXT(",") << m_remote_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
// 					<< _XTEXT(" file: ") << __XFILE__ << _XTEXT(" line: ") << __LINE__ );
#else
				////FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("TCPSession::read() can't get buffer!") << _XTEXT("\n\t")
// 					<< _XTEXT("conn_index:") << get_conn_index() << _XTEXT("\n\t")
// 					<< _XTEXT(" local ip addr:(") << m_local_endpoint.address().to_string().c_str() << _XTEXT(",") << m_local_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
// 					<< _XTEXT(" remote ip addr:(") << m_remote_endpoint.address().to_string().c_str() << _XTEXT(",") << m_remote_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
// 					<< _XTEXT(" file: ") << __XFILE__ << _XTEXT(" line: ") << __LINE__ );
#endif
				//if(m_option.close_overflowed_session)
				//{
				//	//FAITH_LOG_DEBUG(Scheduler::getInstance().get_logger(),_XTEXT("close_overflowed_session conn_index:") << get_conn_index() );
				//	close();
				//}
				return false;
			}

			m_send_buf.push(&header,sizeof(header));
			m_send_buf.push(data_ptr,data_size);

			//	async. send loop has been started, just return
			if( m_sending )
				return true;

			send_data_by_size();
			return true;
		}

		template < typename T >
		bool tcp_session<T>::send_multi(const datablock_queue_type& data_queue)
		{
			//calculate the data size
			if (m_been_opened == false)
			{
				return false;
			}
			unsigned int data_size=0;
			{
				for(datablock_queue_type::const_iterator it = data_queue.begin();it!=data_queue.end();++it)
				{
					data_size += it->second;
				}
			}
			

			if(data_size > m_option.max_packet_size)
			{
				////FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("TCPSession::send_multi() data size:") << data_size << _XTEXT("larger than the max packet size:") << m_option.max_packet_size << _XTEXT("\n\t")
// 					<< _XTEXT("conn_index:") << get_conn_index() << _XTEXT("\n\t")
// 					<< _XTEXT(" local ip addr:(") << m_local_endpoint.address().to_string().c_str() << _XTEXT(",") << m_local_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
// 					<< _XTEXT(" remote ip addr:(") << m_remote_endpoint.address().to_string().c_str() << _XTEXT(",") << m_remote_endpoint.port() << _XTEXT(")") );
				return false;
			}

			tcp_pak_header	header;
			header.reset();
			header.length = data_size;

			//	for m_send_buf's res. protection
			boost::recursive_mutex::scoped_lock sendbuf_lock(m_sendbuf_mutex);
			//	in case of failure when
			//	below m_send_buf.push( body_info );
			if( !m_send_buf.can_push( sizeof(tcp_pak_header)+data_size ) )
			{
				////FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("TCPSession::send_multi() can't push!") << _XTEXT("\n\t")
// 					<< _XTEXT("conn_index:") << get_conn_index() << _XTEXT("\n\t")
// 					<< _XTEXT(" local ip addr:(") << m_local_endpoint.address().to_string().c_str() << _XTEXT(",") << m_local_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
// 					<< _XTEXT(" remote ip addr:(") << m_remote_endpoint.address().to_string().c_str() << _XTEXT(",") << m_remote_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
// 					<< _XTEXT(" file: ") << __XFILE__ << _XTEXT(" line: ") << __LINE__ );
				//if(m_option.close_overflowed_session)
				//{
				//	//FAITH_LOG_DEBUG(Scheduler::getInstance().get_logger(),_XTEXT("close_overflowed_session conn_index:") << get_conn_index() );
				//	close();
				//}
				return false;
			}

			m_send_buf.push(&header,sizeof(header));
			{
				for(datablock_queue_type::const_iterator it = data_queue.begin();it!=data_queue.end();++it)
				{
					m_send_buf.push(it->first,it->second);
				}
			}

			//	async. send loop has been started, just return
			if( m_sending )
				return true;

			send_data_by_size();
			return true;
		}

		template < typename T >
		void tcp_session<T>::send_delay_data()
		{
			boost::recursive_mutex::scoped_lock sendbuf_lock(m_sendbuf_mutex);
			if(m_sending)
			{
				return;
			}
			send_data_by_time();
		}

		template < typename T >
		void tcp_session<T>::on_packet_received(int packet_count)
		{
			if (m_been_opened == false)
			{
				return;
			}
			const void * packet_data;
			unsigned int packet_size;
			boost::recursive_mutex::scoped_lock recvbuf_lock(m_recvbuf_mutex);
			for(int i = 0;i< packet_count;++i)
			{
				packet_data = m_recv_buf.get_head_packet(packet_size);
				recvbuf_lock.unlock();
				m_recv_handler( m_conn_index,packet_data,packet_size);
				recvbuf_lock.lock();
				m_recv_buf.pop_head_packet();
			}
			read();
		}

		template < typename T >
		void tcp_session<T>::read()
		{
			if (m_been_opened == false)
			{
				m_recv_buf.clear_data();
				return;
			}
			if(m_reading)
			{
				return;
			}
			unsigned int size;
			void * buffer = m_recv_buf.get_free_buffer(size);
			m_reading = true;
			if (buffer && size > 0 )
			{
				m_socket.async_read_some(
					boost::asio::buffer(buffer,size),
					boost::bind( &T::handle_read, this->shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred )
					);
			}
			else
			{
				m_reading = false;
#ifdef _DEBUG_NET_BUFFER
				//FAITH_LOG_RELEASE(scheduler::getInstance().get_logger(),_XTEXT("TCPSession::read() can't get buffer!") << _XTEXT("\n\t")
// 					<< _XTEXT("conn_index:") << get_conn_index() << _XTEXT("\n\t")
// 					<< _XTEXT(" local ip addr:(") << m_local_endpoint.address().to_string().c_str() << _XTEXT(",") << m_local_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
// 					<< _XTEXT(" remote ip addr:(") << m_remote_endpoint.address().to_string().c_str() << _XTEXT(",") << m_remote_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
// 					<< _XTEXT(" file: ") << __XFILE__ << _XTEXT(" line: ") << __LINE__ );
#else
				////FAITH_LOG_ERROR(scheduler::getInstance().get_logger(),_XTEXT("TCPSession::read() can't get buffer!") << _XTEXT("\n\t")
// 					<< _XTEXT("conn_index:") << get_conn_index() << _XTEXT("\n\t")
// 					<< _XTEXT(" local ip addr:(") << m_local_endpoint.address().to_string().c_str() << _XTEXT(",") << m_local_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
// 					<< _XTEXT(" remote ip addr:(") << m_remote_endpoint.address().to_string().c_str() << _XTEXT(",") << m_remote_endpoint.port() << _XTEXT(")") << _XTEXT("\n\t")
// 					<< _XTEXT(" file: ") << __XFILE__ << _XTEXT(" line: ") << __LINE__ );
#endif

			}
		}

		template < typename T >
		tcp_session<T>::~tcp_session()
		{
			close();
		}
	}	// end of namespace net

}	// end of namespace faith

#pragma warning(pop)

#endif // end of #define __FAITH_TCPCONNECTION_HEADER__
