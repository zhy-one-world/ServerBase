/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:05
	file base:	delay_send_queue
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _DELAY_SEND_QUEUE_H_
#define _DELAY_SEND_QUEUE_H_

#include <list>
#include <boost/cstdint.hpp>
#include <boost/detail/lightweight_mutex.hpp>
#include "singleton.hpp"
#include "fast_allocator.hpp"

namespace faith
{
	namespace net
	{
		class delay_send_session;
		typedef std::list<delay_send_session *,::faith::fast_allocator<delay_send_session *> >	delay_send_list;
		//@warning:受实现限制,该类型最多在994天后会产生回绕
		typedef boost::uint32_t																	delay_send_loop_counter_type;	//延时发送循环计数器类型

		class delay_send_queue : public ::faith::singleton<delay_send_queue>
		{
			friend class ::faith::singleton<delay_send_queue>;
		private:
			delay_send_queue();
		public:
			void								init();		//初始化,创建计时器
			void								release();		//清除,删除计时器
			void								add_session(delay_send_session * session);
			void								remove_session(delay_send_session * session);			
			delay_send_loop_counter_type		get_loop_counter() const
			{
				return m_loop_counter;
			}
		private:
			void								update(unsigned int timer_index);	
		public:
			static const unsigned int			timer_interval = 20;		//计时器间隔时间
		private:
			boost::detail::lightweight_mutex	m_mutex;			//对象访问锁
			delay_send_loop_counter_type		m_loop_counter;		//循环计数
			delay_send_list						m_session_list;		//sesson列表
			unsigned int						m_timer_index;		//计时器索引
		};
	}
}

#endif
