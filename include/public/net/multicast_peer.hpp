/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:15
	file base:	multicast_peer
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _MULTICAST_PEER_H_
#define _MULTICAST_PEER_H_

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#ifdef __ANDROID__
#include "xchar.hpp"
#else
#include "../common/xchar.hpp"
#endif

//#include "config.hpp"

namespace faith 
{
	namespace net 
	{
		class multicast_peer_impl;

		class multicast_peer:
			private boost::noncopyable
		{
		public:
			typedef boost::function<void( unsigned int, unsigned short, const void*, size_t)>
				onrecv_handler_type;
			typedef boost::function<void(void)> 
				post_handler_type;
			typedef boost::function<void(post_handler_type)>
				outer_post_type;

		private:
			boost::scoped_ptr<multicast_peer_impl>	impl_ptr;

		public:
			multicast_peer(	onrecv_handler_type recv_handle, int buffer_size = 4096 );
			~multicast_peer(void);			
		public:
			//start listen port and multicast channel
			void start(const xstring& listen_addr, const xstring& multicast_addr, unsigned short listen_and_multicast_port);

			//stop listen port and multicast channel
			void stop();

			//multicast send packet to this peer's channel
			bool send(const void *data_ptr, size_t data_len);

			//warning! use it, if you fully understand the this.
			void set_outer_receive_poster(boost::function<void(boost::function<void(void)>)> poster_handle);
		};
	}	// end of namespace net
}	// end of namespace faith

#endif // end of #define __OMP_MULTICAST_PEER_HEADER__
