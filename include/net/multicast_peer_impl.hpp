/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:26
	file base:	multicast_peer_impl
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _MULTICAST_PEER_IMPL_H_
#define _MULTICAST_PEER_IMPL_H_

#pragma warning(disable:4099)

#include "asio.hpp"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>
#include "multicast_peer_buffer.hpp"
#include "xchar.hpp"

namespace faith
{
	namespace net
	{		
		typedef boost::function<bool(unsigned int ip, unsigned short port, const void * buffer, size_t size)> multicast_peerPlugIn;

		class multicast_peer_impl :	private boost::noncopyable
		{
		public:
			typedef boost::function<void( unsigned int, unsigned short, const void*, size_t)>	onrecv_handler_type;
			typedef boost::function<void(void)>													post_handler_type;
			typedef boost::function<void(post_handler_type)>									outer_post;
			typedef boost::circular_buffer<boost::asio::ip::udp::endpoint>						buffer_and_endpoints;
			typedef std::list<boost::shared_ptr<boost::thread> > ThreadPool;
			enum
			{
				max_pack_length = 1024,
			};
		public:
			multicast_peer_impl(	onrecv_handler_type recv_handle, int buffer_size = 4096 );
			~multicast_peer_impl(void);
		public:
			//start listen port and multicast channel
			void									start(const xstring& listen_addr, const xstring& multicast_addr, unsigned short listen_and_multicast_port);
			//stop listen port and multicast channel
			void									stop();
			//multicast send packet to this peer's channel
			bool									send(const void *data_ptr, size_t data_len);
			//warning! use it, if you fully understand the this.
			void									set_outer_receive_poster(boost::function<void(boost::function<void(void)>)> poster_handle);
		private:
			static void								start_service();
			static void								thread_func();
			void									read_packet();
			void									handle_read(const boost::system::error_code& error,size_t bytes_transferred);
			void									default_post(post_handler_type handle);
			void									callback_to_recv();
			bool									start_send();
			bool									send_a_packet();
			void									handle_write(const boost::system::error_code& error);
			void									mlb_start(const xstring& listen_addr, const xstring& multicast_addr, unsigned short listen_and_multicast_port);
			void									mlb_stop();
			bool									mlb_send(const xstring & data);
			bool									inner_send(const void *data_ptr, size_t data_len);
			void									call_onrecv_handler(multicast_peer_impl::onrecv_handler_type onrecv_handler, unsigned int ip, unsigned short port,const void *data_ptr,size_t data_len );
			void									recur_onrecv_handler(multicast_peer_impl::onrecv_handler_type onrecv_handler, unsigned int ip, unsigned short port,const xstring & data );
		private:
			static boost::asio::io_service			m_io_service;	//use one io_service at all multicast peer
			static boost::asio::strand				m_strand;		//use one m_strand at all multicast peer
			static bool								m_is_io_service_start;			
			static ThreadPool						m_thread_pool;
			boost::asio::ip::udp::endpoint			m_endpoint;
			boost::asio::ip::udp::socket			m_socket;
			bool									m_is_sending;
			bool									m_is_recving;
			onrecv_handler_type						receive_handler;
			outer_post								out_post_handler;
			// buffers
			boost::recursive_mutex					m_sendbuf_mutex;	//	for m_send_buf's res. protection
			boost::recursive_mutex					m_recvbuf_mutex;	//	for m_recv_buf's res. protection
			send_buffer         					m_send_bufs;
			receive_buffer       					m_recv_bufs;
			buffer_and_endpoints					m_endpoint_bufs;	//  for receive
			boost::uint32_t							m_instance_id;

		};
	}
}

#endif
