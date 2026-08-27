/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   14:28
	file base:	work_flow
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include <boost/bind.hpp>
#include "work_flow.hpp"

namespace faith
{
	namespace common
	{

		work_flow::work_flow(void) :
			m_started(false),
			m_terminated(false),
			m_sync(false)
		{
		}

		work_flow::~work_flow(void)
		{
			if(has_thread())
			{
				terminate();
				wait();
			}
		}

		void work_flow::wait(void)
		{
			if(has_thread())
			{
				m_thread.join();
				boost::thread empty;
				m_thread.swap(empty);
			}
		}

		bool work_flow::start(bool sync)
		{
			if(m_started)
			{
				return  true;
			}

			m_started = true;
			m_terminated = false;
			m_sync = sync;
			if(!m_sync)
			{
				boost::thread work(boost::bind(&work_flow::execute,this));
				m_thread.swap(work);
			}
			

			return true;
		}

		bool work_flow::is_terminated(void)
		{
			return m_terminated;
		}

		bool work_flow::is_started(void)
		{
			return m_started;
		}

		void work_flow::terminate(void)
		{
			m_started = false;
			m_terminated = true;
			on_terminated();
		}

		bool work_flow::is_sync() const
		{
			return m_sync;
		}

		bool work_flow::has_thread() const
		{
			boost::thread empty;
			return m_thread!=empty;
		}
	}
}
