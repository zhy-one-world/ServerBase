/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:21
	file base:	scheduler_impl
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "scheduler_impl.hpp"
#include "delay_send_queue.hpp"
#include "persistence_id_generator.hpp"
//#include "program_options.hpp"
#include "mlb_helper.hpp"
#include "mlb.hpp"
#include "postmortem.hpp"
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <boost/smart_ptr.hpp>
#include <typeinfo>
#include <rlog.hpp>

namespace faith 
{
	namespace net 
	{
		scheduler_impl::scheduler_impl(void) :
			m_id_container(scheduler_invalid_timer_index),
			m_is_running(false),
			m_main_thread_dispatch(false),
			m_stop_requested(false)
		{
			initialize_contexts(1);
			clear_data();
		}

		scheduler_impl::~scheduler_impl(void)
		{
			clear_data();
		}
		void scheduler_impl::clear_data()
		{
			boost::recursive_mutex::scoped_lock	lock(m_scheduler_mutex);
			m_is_running = false;
			for (int i = 0; i < timer_count_max; ++i)
			{
				m_timer_array[i].clear_data();
			}
		}

		namespace
		{
			thread_local unsigned int current_scheduler_thread_id = 0;
		}

		void scheduler_impl::initialize_contexts(unsigned int context_count)
		{
			while (m_io_services.size() < context_count)
			{
				io_service_ptr io_service(new io_service_type());
				m_io_services.push_back(io_service);
				m_strands.push_back(strand_ptr(new strand_type(*io_service)));
			}
		}

		unsigned int scheduler_impl::get_current_thread_id() const
		{
			return current_scheduler_thread_id;
		}

		unsigned int scheduler_impl::get_thread_count() const
		{
			return static_cast<unsigned int>(m_io_services.size());
		}

		unsigned int scheduler_impl::get_worker_thread_start_id() const
		{
			return m_main_thread_dispatch ? 1 : 0;
		}

		boost::asio::io_service& scheduler_impl::get_ioservice()
		{
			return get_ioservice(get_current_thread_id());
		}

		boost::asio::io_service& scheduler_impl::get_ioservice(unsigned int thread_id)
		{
			if (thread_id >= m_io_services.size())
			{
				thread_id = 0;
			}
			return *m_io_services[thread_id];
		}

		boost::asio::io_context::strand& scheduler_impl::get_strand()
		{
			return get_strand(get_current_thread_id());
		}

		boost::asio::io_context::strand& scheduler_impl::get_strand(unsigned int thread_id)
		{
			if (thread_id >= m_strands.size())
			{
				thread_id = 0;
			}
			return *m_strands[thread_id];
		}

		void scheduler_impl::init_options()
		{
			set_option(scheduler::options::thread_num(3),true);
			set_option(scheduler::options::thread_sleeptime(4),true);
		}		
		timer_info* scheduler_impl::get_timer(int index)
		{
			for (int i = 0; i < timer_count_max; ++i)
			{
				if (m_timer_array[i].timer_id == index)
				{
					return &(m_timer_array[i]);
				}
			}
			return nullptr;
		}
		unsigned int scheduler_impl::add_timer(unsigned int interval,timer_handler_type external_handler)
		{
			return add_timer(interval, get_current_thread_id(), external_handler);
		}

		unsigned int scheduler_impl::add_timer(unsigned int interval,unsigned int thread_id,timer_handler_type external_handler)
		{
			boost::recursive_mutex::scoped_lock	lock(m_scheduler_mutex);

			if (thread_id >= m_io_services.size())
			{
				return scheduler_invalid_timer_index;
			}

			unsigned int index = m_id_container.get_id();
			if(index==scheduler_invalid_timer_index)
			{
				return scheduler_invalid_timer_index;
			}
			int empty_index = -1;
			for (int i = 0; i < timer_count_max; ++i)
			{
				if (m_timer_array[i].timer_id == scheduler_invalid_timer_index)
				{
					empty_index = i;
					break;
				}
			}
			if (empty_index == -1)
			{
				return scheduler_invalid_timer_index;
			}
			boost::system::error_code ec;			

			timer_ptr timer_ptr(new boost::asio::deadline_timer(*m_io_services[thread_id]));
			timer_ptr->expires_from_now(boost::posix_time::milliseconds(interval),ec);
			if(ec)
			{
				m_id_container.return_id(index);
				return scheduler_invalid_timer_index;
			}
			timer_ptr->async_wait(m_strands[thread_id]->wrap(
					boost::bind(&scheduler_impl::timer_handler,this,boost::asio::placeholders::error,index,timer_ptr)));
			
			timer_info&	timer_info_ref = m_timer_array[empty_index];			
			timer_info_ref.interval=interval;
			timer_info_ref.thread_id=thread_id;
			timer_info_ref.external_handler=external_handler;
			timer_info_ref.ptr=timer_ptr;
			timer_info_ref.instance_id = common::persistence_id_generator::getInstance().get_id(_XTEXT("Scheduler.Timer"));
			timer_info_ref.timer_id = index;

			return index;
		}

		void scheduler_impl::remove_timer(int index)
		{
			boost::recursive_mutex::scoped_lock	lock(m_scheduler_mutex);
			timer_info* timer_info_ptr = get_timer(index);
			if (nullptr == timer_info_ptr)
			{
				return;
			}
			m_id_container.return_id(index);
			common::persistence_id_generator::getInstance().return_id(_XTEXT("Scheduler.Timer"), timer_info_ptr->instance_id);
			timer_info_ptr->clear_data();
		}

		void scheduler_impl::timer_handler(const boost::system::error_code& error,unsigned int index,timer_ptr timer)
		{
			boost::recursive_mutex::scoped_lock	lock(m_scheduler_mutex);

			if(error)
			{
				// maybe the timer has been removed by remove_timer
				remove_timer(index);
				return;
			}
			timer_info* timer_info_ptr = get_timer(index);
			if (nullptr == timer_info_ptr)
			{
				return;
			}
			//	call the user's timer-handler
			timer_info_ptr->external_handler(index);


			//	re-find the iterator, because user may remove the timer by calling remove_timer() 
			//	when external_handler was called within the above code
			if(timer_info_ptr->ptr.get()!=timer.get())
			{
				return;
			}
			//	continue async_wait
			boost::system::error_code ec;

			//(it_new->second).ptr->expires_at((it_new->second).ptr->expires_at()+boost::posix_time::milliseconds((it_new->second).interval),ec);
			timer_info_ptr->ptr->expires_from_now(boost::posix_time::milliseconds(timer_info_ptr->interval),ec);
			if(ec)
			{
				remove_timer(index);
				return;
			}

			timer_info_ptr->ptr->async_wait(m_strands[timer_info_ptr->thread_id]->wrap(
				boost::bind(&scheduler_impl::timer_handler,this,boost::asio::placeholders::error,index, timer_info_ptr->ptr)));
		}

		void scheduler_impl::startup(bool main_thread_dispatch)
		{
			boost::recursive_mutex::scoped_lock	lock(m_scheduler_mutex);

			if(m_is_running)
				return;

			using namespace common;

			delay_send_queue::getInstance().init();

			if(!m_thread_pool.empty())
			{// may be in the progress of shutdown
				return;
			}

			scheduler::options::thread_num thread_num;
			scheduler::getInstance().get_option(thread_num);
			
			if(thread_num.value==0)
			{
				thread_num.value = 1;
			}

			m_main_thread_dispatch = main_thread_dispatch;
			m_stop_requested = false;
			const unsigned int context_count = thread_num.value + (m_main_thread_dispatch ? 1 : 0);
			_RLOG_(MINFO, "scheduler startup, configured workers:" << thread_num.value
				<< " context count:" << context_count
				<< " main dispatch:" << m_main_thread_dispatch);
			initialize_contexts(context_count);
			for (io_service_pool::iterator i = m_io_services.begin(); i != m_io_services.end(); ++i)
			{
				(*i)->restart();
			}

			m_is_running=true;

			for(unsigned int i=0;i<thread_num.value;i++ )
			{
				const unsigned int thread_id = i + (m_main_thread_dispatch ? 1 : 0);
				boost::shared_ptr<boost::thread> new_thread(new boost::thread( boost::bind(&scheduler_impl::thread_func,this,thread_id) ));
				m_thread_pool.push_back(new_thread);
			}
			_RLOG_(MINFO, "scheduler startup completed, worker threads started:" << m_thread_pool.size());
		}

		bool scheduler_impl::shutdown()
		{
			boost::recursive_mutex::scoped_lock	lock(m_scheduler_mutex);

			if(!m_is_running)
			{
				for (io_service_pool::iterator i = m_io_services.begin(); i != m_io_services.end(); ++i)
				{
					(*i)->stop();
				}
				m_thread_pool.clear();
				return false;
			}
			else
			{
				if(in_working_threads())
				{
					return false;
				}

				//	stop services, clear all pending events
				for (io_service_pool::iterator i = m_io_services.begin(); i != m_io_services.end(); ++i)
				{
					(*i)->stop();
				}

				//	wait until all threads stopped
				m_is_running=false;

				// make a pool of thread pool
				thread_pool thread_pool = m_thread_pool;
				lock.unlock();
				// make sure we do not call join() for the current thread,
				// since this may yield "undefined behavior"
				{
					boost::this_thread::disable_interruption di;
					for (thread_pool::iterator i = m_thread_pool.begin();i != m_thread_pool.end(); ++i)
					{						
						(*i)->join();
					}
				}
				
				lock.lock();

				// clear the thread pool (also deletes thread objects)
				m_thread_pool.clear();
				//m_timermap.clear();

				//m_scheduler_has_stopped.notify_all();
				return true;
			}
		}

		void scheduler_impl::thread_func(unsigned int thread_id)
		{
			//FAITH_STACKOVERFLOW_CATCH_BEGIN();
			current_scheduler_thread_id = thread_id;
			thread_func_impl(thread_id);
			//FAITH_STACKOVERFLOW_CATCH_END();
		}
		void scheduler_impl::thread_func_impl(unsigned int thread_id)
		{
			boost::asio::io_service& io_service = get_ioservice(thread_id);
			do
			{
				try
				{
					boost::system::error_code ec;
					io_service.run();
					//std::cout << "ec = " << ec << std::endl;
				}
				catch (...)
				{
					boost::recursive_mutex::scoped_lock	lock(m_scheduler_mutex);

					static int exception_count = 0;

					++exception_count;					
					if (exception_count == 1)
					{
						//???????????????????????????????????throw??????????????????????????????????
					throw;
				}
				}
				if (m_is_running && !m_stop_requested && io_service.stopped())
				{
					io_service.restart();
				}
				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}while(m_is_running && !m_stop_requested);
		}

		void scheduler_impl::run_current_thread()
		{
			current_scheduler_thread_id = 0;
			thread_func_impl(0);
		}

		void scheduler_impl::request_stop()
		{
			boost::recursive_mutex::scoped_lock lock(m_scheduler_mutex);
			m_stop_requested = true;
			for (io_service_pool::iterator i = m_io_services.begin(); i != m_io_services.end(); ++i)
			{
				(*i)->stop();
			}
		}

		boost::xtime scheduler_impl::get_wakeuptime(unsigned int sleep_sec,unsigned int sleep_nsec)
		{
			const unsigned int NSEC_IN_SECOND = 1000000000;	// (10^9)

			boost::xtime wakeup_time;
			boost::xtime_get(&wakeup_time, boost::TIME_UTC_);
			wakeup_time.sec += sleep_sec;
			wakeup_time.nsec += sleep_nsec;
			if (static_cast<unsigned int>(wakeup_time.nsec) >= NSEC_IN_SECOND) {
				wakeup_time.sec++;
				wakeup_time.nsec -= NSEC_IN_SECOND;
			}
			return wakeup_time;
		}

		bool scheduler_impl::in_working_threads()
		{
			boost::recursive_mutex::scoped_lock	lock(m_scheduler_mutex);
			boost::thread::id current_thread_id = boost::this_thread ::get_id();
			for (thread_pool::iterator i = m_thread_pool.begin();i != m_thread_pool.end(); ++i)
			{
				if ((*i)->get_id() == current_thread_id)
				{
					return true;
				}
			}
			return false;
		}

		void scheduler_impl::post(post_handler_type handler)
		{
			post(handler, get_current_thread_id());
		}

		void scheduler_impl::post(post_handler_type handler, unsigned int thread_id)
		{
			boost::uint32_t instance_id = common::persistence_id_generator::getInstance().get_id(_XTEXT("Scheduler.Post"));
			//if(moonlightbox_enabled())
			//{
			//	common::mlb_file::callback_name name;
			//	name.class_name = _XTEXT("Scheduler");
			//	name.function_name = _XTEXT("onPost");
			//	name.instance_id = instance_id;
			//	get_moonlightbox()->register_handler<void(void)>(name,boost::bind(&Scheduler_impl::call_post,this,instance_id,handler));
			//}
			//if(!in_recurrence_mode())
			{
				if (thread_id >= m_strands.size())
				{
					thread_id = 0;
				}
				boost::asio::post(*m_strands[thread_id], boost::bind(&scheduler_impl::call_post,this,instance_id,handler));
			}			
		}

		void scheduler_impl::inner_post(post_handler_type handler)
		{
			inner_post(handler, get_current_thread_id());
		}

		void scheduler_impl::inner_post(post_handler_type handler, unsigned int thread_id)
		{
			if (thread_id >= m_strands.size())
			{
				thread_id = 0;
			}
			boost::asio::post(*m_strands[thread_id], handler);
		}

		void scheduler_impl::call_post(boost::uint32_t instance_id,post_handler_type handler)
		{
			handler();

			common::persistence_id_generator::getInstance().return_id(_XTEXT("Scheduler.Post"),instance_id);
		}


		bool scheduler_impl::set_option(const boost::any& option_item,bool init_param)
		{
			bool ret = options_container::set_option(option_item,init_param);

			return ret;
		}

		/* ???asio?????????unicode?? asio??????????????? */
		const xchar * scheduler_impl::asio_message(const std::string & msg)
		{
#if defined(FAITH_UNICODE)
			static wchar_t buf[2048];
			common::utility::_iconv_one(locale_charset(),"UCS-2LE",(void*)msg.c_str(),msg.size(),(void*)buf,sizeof(buf));
			return buf;
#else
			return msg.c_str();
#endif
		}
	}
}
