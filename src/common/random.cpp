/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:28
	file base:	random
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "random.hpp"
#include "mlb.hpp"
#include <stdlib.h>
#include <boost/function.hpp>
#include <boost/random.hpp>
#include <time.hpp>

namespace faith
{
	namespace common
	{
		namespace utility
		{
			boost::random::mt19937 rng(get_tick_count());
			boost::random::uniform_int_distribution<> random_len(0, 0x7FFFFFFF);
			MLB_FUNC_0(int,rand)
			{
				return random_len(rng);
			}
		}
	}
}
