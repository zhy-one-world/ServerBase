/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:20
	file base:	scheduler
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "scheduler_impl.hpp"
#include "scheduler.hpp"

namespace faith 
{
	namespace net 
	{
		scheduler::scheduler(void)
		{
			m_impl_ptr = new scheduler_impl();
			if (m_impl_ptr)
				m_impl_ptr->init_options();
		}

		scheduler::~scheduler(void)
		{
			delete m_impl_ptr;
		}

		bool scheduler::set_option(const boost::any& option_item)
		{
			if (!m_impl_ptr)
				return false;
			return m_impl_ptr->set_option(option_item);
		}

		bool scheduler::get_option(boost::any& option_item)
		{
			if (!m_impl_ptr)
				return false;
			return m_impl_ptr->get_option(option_item);
		}

		scheduler& scheduler::getInstance(void)
		{
			static scheduler sheduler_instance;
			return sheduler_instance;
		}

		void scheduler::startup()
		{
			if (m_impl_ptr)
				m_impl_ptr->startup();
		}

		void scheduler::shutdown()
		{
			if (m_impl_ptr)
			{
				m_impl_ptr->shutdown();
			}		
		}

		unsigned int scheduler::add_timer(unsigned int interval,timer_handler_type handler)
		{
			if (!m_impl_ptr)
				return scheduler_invalid_timer_index;
			return m_impl_ptr->add_timer(interval,handler);
		}

		void scheduler::remove_timer(int index)
		{
			if (m_impl_ptr)
				m_impl_ptr->remove_timer(index);
		}

		void scheduler::post(post_handler_type handler)
		{
			if (m_impl_ptr)
				m_impl_ptr->post(handler);
		}

		void scheduler::post_raw(post_handler_type handler)
		{
			if (m_impl_ptr)
				m_impl_ptr->inner_post(handler);
		}
	}
}
