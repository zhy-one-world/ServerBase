/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:32
	file base:	scheduler_impl
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _SHEDULER_IMPL_H_
#define _SHEDULER_IMPL_H_

#include "asio.hpp"
#include "monotone_timer.hpp"
#include <list>
#include <map>
#include <string>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/detail/atomic_count.hpp>
#include "scheduler.hpp"
#include "unique_index.hpp"
#include "options_container.hpp"

namespace faith 
{
	namespace net 
	{
		const unsigned int timer_count_max = 100;
		enum
		{
			scheduler_timer_num_limit = scheduler::scheduler_timer_num_limit,
			scheduler_invalid_timer_index = scheduler::scheduler_invalid_timer_index,
		};
		typedef std::list<boost::shared_ptr<boost::thread> >								thread_pool;
		typedef boost::asio::io_service													io_service_type;
		typedef boost::asio::io_context::strand											strand_type;
		typedef boost::shared_ptr<io_service_type>										io_service_ptr;
		typedef boost::shared_ptr<strand_type>											strand_ptr;
		typedef std::vector<io_service_ptr>												io_service_pool;
		typedef std::vector<strand_ptr>													strand_pool;
		typedef unique_id_container<unsigned int, scheduler_timer_num_limit>				index_container;
		typedef boost::shared_ptr<boost::asio::deadline_timer>								timer_ptr;
		struct timer_info
		{
			unsigned int		interval;
			unsigned int		thread_id;
			timer_ptr			ptr;
			timer_handler_type	external_handler;
			boost::uint32_t		instance_id;
			boost::uint32_t		timer_id;
			timer_info()
			{
				clear_data();
			}
			void clear_data()
			{
				memset(this, 0, sizeof(timer_info));
				timer_id = scheduler_invalid_timer_index;
			}
		};
		class scheduler_impl : public options_container
		{
		public:
			scheduler_impl(void);
			~scheduler_impl(void);
		public:
			void							clear_data();
		public:
			// ONLY for internal class use
			boost::asio::io_context::strand&			get_strand();
			boost::asio::io_context::strand&			get_strand(unsigned int thread_id);
			boost::asio::io_service&				get_ioservice();
			boost::asio::io_service&				get_ioservice(unsigned int thread_id);
			unsigned int							get_current_thread_id() const;
			unsigned int							get_thread_count() const;
			unsigned int							get_worker_thread_start_id() const;
			const xchar*					asio_message(const std::string & msg);
			void							init_options();
			bool							set_option(const boost::any& option_item,bool init_param=false);
			void							startup(bool main_thread_dispatch = false);
			bool							shutdown();					

			unsigned int					add_timer(unsigned int interval,timer_handler_type handler);
			unsigned int					add_timer(unsigned int interval,unsigned int thread_id,timer_handler_type handler);
			void							remove_timer(int index);
			void							post(post_handler_type handler);
			void							post(post_handler_type handler,unsigned int thread_id);
			void							inner_post(post_handler_type handler);
			void							inner_post(post_handler_type handler,unsigned int thread_id);
			void							run_current_thread();
			void							request_stop();
		private:
			void							initialize_contexts(unsigned int context_count);
			timer_info*						get_timer(int index);
			//	param:	index of timer
			void							timer_handler(const boost::system::error_code& error,unsigned int index,timer_ptr timer);
			void							thread_func(unsigned int thread_id);
			void							thread_func_impl(unsigned int thread_id);
			boost::xtime					get_wakeuptime(unsigned int sleep_sec,unsigned int sleep_nsec);
			bool							in_working_threads();
			void							call_post(boost::uint32_t instance_id,post_handler_type handler);
			//void recurer_thread_func();
		private:
			//	timer's container
			timer_info						m_timer_array[timer_count_max];
			//	WARNING:	below 2 member's declaration-order can't be changed
			io_service_pool					m_io_services;
			strand_pool						m_strands;
			bool							m_is_running;
			bool							m_main_thread_dispatch;
			bool							m_stop_requested;
			boost::recursive_mutex			m_scheduler_mutex;
			thread_pool						m_thread_pool;
			index_container					m_id_container;
			boost::system::error_code		m_ec;
		};

		#define _asio_message scheduler::getInstance().get_impl()->asio_message

	}
}

#endif
