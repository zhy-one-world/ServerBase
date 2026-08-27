/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:13
	file base:	tcp_session_option
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _TCP_SESSION_OPTION_H_
#define _TCP_SESSION_OPTION_H_

#include <boost/pool/pool.hpp>

namespace faith
{
	namespace net
	{
		struct tcp_session_option
		{
			bool			check_options() const;
			unsigned int	max_packet_size;
			unsigned int	send_buffer_size;
			unsigned int	recv_buffer_size;
			unsigned int	delaysending_size_threshold;			
		};
	}
}

#endif
