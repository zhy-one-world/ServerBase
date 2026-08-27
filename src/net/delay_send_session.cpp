/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:19
	file base:	delay_send_session
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "delay_send_session.hpp"
#include "delay_send_queue.hpp"

namespace faith
{
	namespace net
	{
		delay_send_queue & delay_send_session::m_delay_send_queue = delay_send_queue::getInstance();

		delay_send_session::delay_send_session(unsigned int check_interval)
		{
			m_check_interval = check_interval>0?check_interval:1;
			m_delay_send_queue.add_session(this);
			m_next_send_time = 0;
		}

		delay_send_session::~delay_send_session()
		{
			m_delay_send_queue.remove_session(this);
		}
	}
}
