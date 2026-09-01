/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:19
	file base:	delay_send_queue
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "delay_send_queue.hpp"
#include "delay_send_session.hpp"
#include "scheduler.hpp"
#include <boost/bind.hpp>

namespace faith
{
	namespace net
	{
		delay_send_queue::delay_send_queue():
			m_loop_counter(0),m_timer_index(0)
		{

		}

		void delay_send_queue::init()
		{
			m_timer_index = scheduler::getInstance().add_timer(timer_interval,boost::bind(&delay_send_queue::update,this,_1));
		}
		void delay_send_queue::release()
		{
		}

		void delay_send_queue::update(unsigned int timer_index)
		{
			++ m_loop_counter;
			boost::detail::lightweight_mutex::scoped_lock lock(m_mutex);
			for(delay_send_list::iterator it = m_session_list.begin();it!=m_session_list.end();++it)
			{
				delay_send_session * session = *it;
				session->send_delay_data();
			}
		}

		void delay_send_queue::add_session(delay_send_session * session)
		{
			session->set_send_time();
			boost::detail::lightweight_mutex::scoped_lock lock(m_mutex);
			m_session_list.push_back(session);
			session->m_it = --m_session_list.end();
		}

		void delay_send_queue::remove_session(delay_send_session * session)
		{
			boost::detail::lightweight_mutex::scoped_lock lock(m_mutex);
			if(session)
				m_session_list.remove(session);
		}
	}
}
