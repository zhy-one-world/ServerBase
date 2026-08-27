#ifndef _FAITH_COMMON_NS_COMPAT_H_
#define _FAITH_COMMON_NS_COMPAT_H_

#include "mem_pool.hpp"
#include "singleton.hpp"
#include "unique_id_generator.hpp"
#include "string_buffer.hpp"
#include "fast_allocator.hpp"
#include "time.hpp"
#include "persistence_id_generator.hpp"
#include "postmortem.hpp"

namespace faith
{
	namespace common
	{
		using ::faith::mem_pool;
		using ::faith::persistence_id_generator;
		using ::faith::postmortem;

		template <class T>
		using singleton = ::faith::singleton<T>;

		template <class T>
		using unique_id_generator = ::faith::unique_id_generator<T>;

		template <class T>
		using basic_string_buffer = ::faith::basic_string_buffer<T>;

		using string_buffer = ::faith::string_buffer;
		using wstring_buffer = ::faith::wstring_buffer;

		template <class T, unsigned int N = 1>
		using fast_allocator = ::faith::fast_allocator<T, N>;

		namespace utility
		{
			using ::faith::utility::get_tick_count;
			using ::faith::utility::get_local_tick_count;
			using ::faith::utility::time;
		}
	}
}

#endif
