/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:23
	file base:	tcp_session_option
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "tcp_session_option.hpp"
#include "tcp_pak_def.hpp"

namespace faith
{
	namespace net
	{
		bool tcp_session_option::check_options() const
		{
			unsigned int max_real_packet_size = max_packet_size + sizeof(tcp_pak_header);
			if(send_buffer_size < max_real_packet_size)
			{
				return false;
			}

			if(recv_buffer_size < max_real_packet_size)
			{
				return false;
			}

			if(delaysending_size_threshold >= send_buffer_size/2 )
			{
				return false;
			}

			return true;
		}
	}
}
