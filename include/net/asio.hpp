/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:03
	file base:	asio
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _ASIO_H_
#define _ASIO_H_

#include "mem_pool.hpp"
#include "common_ns_compat.hpp"

namespace boost 
{
	namespace asio 
	{
		template <typename handler>
		void* asio_handler_allocate(std::size_t size, handler* h)
		{
			return ::faith::mem_pool::getInstance().alloc(size);
		}

		template <typename handler>
		void asio_handler_deallocate(void* pointer, std::size_t size, handler* h)
		{
			::faith::mem_pool::getInstance().free(pointer,size);
		}
	}
}

#include <boost/asio.hpp>

// Boost.Asio 1.87+ removed io_service; keep a compatibility alias for legacy code.
namespace boost
{
	namespace asio
	{
		using io_service = io_context;
	}
}

#endif
