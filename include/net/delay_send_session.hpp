/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:06
	file base:	delay_send_session
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _DELAY_SEND_SESSION_H_
#define _DELAY_SEND_SESSION_H_

#include "delay_send_queue.hpp"

namespace faith
{
	namespace net
	{
		//延时发送的Sesson接口
		class delay_send_session
		{
			friend class delay_send_queue;
		public:
			delay_send_session(unsigned int check_interval);
			virtual ~delay_send_session();
		public:	
			//发送延时数据
			virtual void					send_delay_data() = 0;
		protected:
			void							unregister_from_delay_queue()
			{
				m_delay_send_queue.remove_session(this);
			}
			bool							check_send_time()
			{
				return m_delay_send_queue.get_loop_counter() >= m_next_send_time;
			}

			void							set_send_time()
			{
				m_next_send_time = m_delay_send_queue.get_loop_counter()+m_check_interval;
			}
		private:
			unsigned int					m_check_interval;	//检查间隔
			delay_send_loop_counter_type	m_next_send_time;	//下一次可以发送时间
			delay_send_list::iterator		m_it;				//在list中的位置
			static delay_send_queue &		m_delay_send_queue;	//为优化性能设置的缓存指针
		};
	}
}

#endif
