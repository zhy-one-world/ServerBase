/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:16
	file base:	datablock
	file ext:	hpp
	author:		zhy
	
	purpose:	
*********************************************************************/
#ifndef _DATABLOCK_H_
#define _DATABLOCK_H_

#include <queue>
#include "fast_allocator.hpp"

namespace faith
{
	namespace net
	{
		typedef std::pair
		<
			const void *,	//data_ptr	数据指针
			size_t			//data_len	数据大小
		> datablock_type;

		typedef std::deque< datablock_type, fast_allocator<datablock_type> > datablock_queue_type;
	}//namespace net

}//namespace faith


#endif//#ifndef __OMP_NET_DATABLOCK_HEADER__
