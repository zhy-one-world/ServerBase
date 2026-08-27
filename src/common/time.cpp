/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:28
	file base:	time
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include <time.h>
#include "time.hpp"
#include "mlb.hpp"
#include <boost/date_time/posix_time/posix_time_types.hpp>

#if defined _MSC_VER
	#include <Windows.h>
	#include <mmsystem.h>
	#pragma comment(lib,"winmm.lib")
#endif


#ifdef CC_TARGET_OS_IPHONE
#include "mach/clock.h"
#include "mach/mach.h"
#endif

namespace faith
{
	namespace common
	{
		namespace utility
		{
			uint64_t get_tick_count()
			{
				boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
				boost::posix_time::time_duration time_from_epoch =
					//  boost::posix_time::microsec_clock::universal_time() - epoch;
					boost::posix_time::microsec_clock::universal_time() - epoch;
				//std::cout << time_from_epoch.total_milliseconds() << std::endl;
				return time_from_epoch.total_milliseconds();
				//return time_from_epoch.total_seconds();
				//return ret;
			}			
			uint64_t get_local_tick_count()
			{
				boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
				boost::posix_time::time_duration time_from_epoch =
					//  boost::posix_time::microsec_clock::universal_time() - epoch;
					boost::posix_time::microsec_clock::local_time() - epoch;
				//std::cout << time_from_epoch.total_milliseconds() << std::endl;
				return time_from_epoch.total_milliseconds();
				//return time_from_epoch.total_seconds();
				//return ret;
			}

			MLB_FUNC_0(uint64_t,time)
			{
				boost::int32_t ret;
				ret = static_cast<boost::int32_t>(::time(NULL));
				return ret;
			}
		}
	}
}
