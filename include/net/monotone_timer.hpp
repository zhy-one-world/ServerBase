/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:21
	file base:	monotone_timer
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _MONOTONE_TIMER_H_
#define _MONOTONE_TIMER_H_

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/asio/time_traits.hpp>
#include <boost/asio/basic_deadline_timer.hpp>
#include <time.hpp>

namespace boost
{
	namespace asio 
	{	
		namespace monotone_time
		{
			class mtime 
			{ 
			public: 
				mtime() {} 
				explicit mtime(const boost::posix_time::time_duration &d) :     m(d){} 
			public: 
				mtime operator+(const boost::posix_time::time_duration &d) const { return mtime(m + d); } 
				mtime operator-(const boost::posix_time::time_duration &d) const { return mtime(m - d); } 
				boost::posix_time::time_duration operator-(const mtime &t) const { return m - t.m; } 
			public: 
				boost::posix_time::time_duration m;
			};   // namespace monotone_time 
		}		

		template <> 
		struct time_traits<monotone_time::mtime> 
		{ 
			/// The time type. 
			typedef boost::asio::monotone_time::mtime time_type; 

			/// The duration type. 
			typedef boost::posix_time::time_duration duration_type; 

			/// Get the current time. 
			static time_type now()
			{
				return time_type(boost::posix_time::millisec(::faith::utility::get_tick_count())); 
			}
					
			/// Add a duration to a time. 
			static time_type add(const time_type& t, const duration_type& d) 
			{ 
				return t + d; 
			} 

			/// Subtract one time from another. 
			static duration_type subtract(const time_type& t1, const time_type& t2) 
			{ 
				return t1.m - t2.m; 
			} 

			/// Test whether one time is less than another. 
			static bool less_than(const time_type& t1, const time_type& t2) 
			{ 
				return t1.m < t2.m; 
			} 

			/// Convert to POSIX duration type. 
			static boost::posix_time::time_duration to_posix_duration( const duration_type& d) 
			{ 
				return d; 
			} 
		}; 

		/// Typedef for the typical usage of timer. 
		typedef boost::asio::basic_deadline_timer<boost::asio::monotone_time::mtime> monotone_timer; 
	}
}

#endif
