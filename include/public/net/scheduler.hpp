/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:35
	file base:	Scheduler
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _SHEDULER_H_
#define _SHEDULER_H_

#include <boost/function.hpp>
#include <boost/any.hpp>
#include "xchar.hpp"

namespace faith 
{
	namespace net 
	{
		class scheduler_impl;
		typedef boost::function<void(unsigned int)>	timer_handler_type;
		typedef boost::function<void(void)> post_handler_type;
		typedef boost::function<void (
			const xchar* class_name, 
			const xchar* function_name, 
			boost::uint32_t instance_it, 
			time_t time, 
			boost::uint64_t index)> recurer_handler_type;
		class scheduler
		{
		public:
			enum
			{
				scheduler_timer_num_limit = 64,
				scheduler_invalid_timer_index = 0x0FFFFFFF,
			};

			struct options
			{
				template<int check_sum,class value_type>
				struct option_item_type
				{
					typedef value_type type_;
					value_type	value;

					option_item_type(value_type v):value(v)	{};
					option_item_type()	{};
				};
				typedef option_item_type<0,unsigned int>							thread_num;						//workerthread_num
				typedef option_item_type<1,unsigned int>							thread_sleeptime;				//workerthread_idle_time
			};
		private:
			scheduler(void);
		public:
			~scheduler(void);
		public:
			static scheduler&					getInstance(void);
			bool								set_option(const boost::any& option_item);
			bool								get_option(boost::any& option_item);
			template<class option_type>
			bool								get_option(option_type& dest)
			{
				boost::any any_value=option_type();
				get_option(any_value);
				option_type *ptr=boost::any_cast<option_type>(&any_value);
				if(ptr==NULL) 
					return false;
				dest=*ptr;
				return true;
			}
			unsigned int						add_timer(unsigned int interval,timer_handler_type handler);
			unsigned int						add_timer(unsigned int interval,unsigned int thread_id,timer_handler_type handler);
			void								remove_timer(int index);
			void								post(post_handler_type handler);
			void								post(post_handler_type handler,unsigned int thread_id);
			void								post_raw(post_handler_type handler);
			void								post_raw(post_handler_type handler,unsigned int thread_id);
			void								startup(bool main_thread_dispatch = false);
			void								shutdown();
			void								run_current_thread();
			void								run_exclusive(post_handler_type handler);
			void								request_stop();
			unsigned int						get_current_thread_id() const;
		public:
			scheduler_impl*						get_impl()	{	return m_impl_ptr;	};
		private:
			scheduler_impl*	m_impl_ptr;
		};
	}
}

#endif
