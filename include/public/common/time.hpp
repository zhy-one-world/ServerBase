/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:28
	file base:	time
	file ext:	hpp
	author:		zhy
	
	purpose:	
*********************************************************************/
#ifndef _TIMER_H_
#define _TIMER_H_
#include <boost/cstdint.hpp>


namespace faith
{
	namespace utility
	{
		//Return the the number of milliseconds since a certain time point.
		uint64_t get_tick_count();

		//Return the the number of milliseconds since a local time point.
		uint64_t get_local_tick_count();

		//Return the time as seconds elapsed since midnight, January 1, 1970, or -1 in the case of an error.
		uint64_t time();
	};
}

#endif
