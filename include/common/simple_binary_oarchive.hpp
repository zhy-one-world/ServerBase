#ifndef OMP_COMMON_SIMPLE_BINARY_OARCHIVE_HPP
#define OMP_COMMON_SIMPLE_BINARY_OARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// simple_binary_oarchive.hpp

// (C) Copyright 2008 Zhang Yongbo
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <set>
#include <map>

#include <boost/cstdint.hpp>
#pragma warning( push )
#pragma warning( disable : 4103 )
#include <boost/serialization/void_cast.hpp>
#pragma warning( pop )
#include <boost/mpl/assert.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/serialization/serialization.hpp>

namespace faith
{

	// do not derive from this class.  If you want to extend this functionality
	// via inhertance, derived from binary_oarchive_impl instead.  This will
	// preserve correct static polymorphism.
	class  simple_binary_oarchive
	{
		static const int mem_block_size = 4 * 1024;  //一次增长的内存分配数量
	public:
		typedef boost::mpl::bool_<false> is_loading;
		typedef boost::mpl::bool_<true> is_saving;

		inline simple_binary_oarchive()
		{
			const int init_size = mem_block_size;
			m_buffer_begin = new char[init_size];
			m_buffer_end = m_buffer_begin + init_size;
			m_buffer_write = m_buffer_begin;
		}

		inline ~simple_binary_oarchive()
		{
			if (m_buffer_begin)
			{
				delete[] m_buffer_begin;
			}
		}


		template<class T>
		inline simple_binary_oarchive& operator << (const T& v)
		{
			(*this)& v;
			return *this;
		}

		template<class T>
		inline typename boost::disable_if
			<
			boost::mpl::or_<boost::is_arithmetic<T>, boost::is_enum<T> >,
			simple_binary_oarchive&
			>::type operator & (const T& v)
		{
			BOOST_MPL_ASSERT_NOT((boost::is_pointer<T>));
			boost::serialization::serialize(*this, const_cast<T&>(v), 0);
			return *this;
		}

		template<class T>
		inline typename boost::enable_if
			<
			boost::mpl::or_<boost::is_arithmetic<T>, boost::is_enum<T> >,
			simple_binary_oarchive&
			>::type operator & (const T& v)
		{
			char* end = m_buffer_write + sizeof(T);
			if (end > m_buffer_end)
			{//需要重新分配内存
				size_t old_data_size = m_buffer_write - m_buffer_begin;
				size_t new_buffer_size = ((end - m_buffer_begin - 1) / mem_block_size + 1) * mem_block_size;
				char* new_buffer = new char[new_buffer_size];
				memcpy(new_buffer, m_buffer_begin, old_data_size);
				*reinterpret_cast<T*>(new_buffer + old_data_size) = v;
				delete[] m_buffer_begin;
				m_buffer_begin = new_buffer;
				m_buffer_end = new_buffer + new_buffer_size;
				m_buffer_write = new_buffer + old_data_size + sizeof(T);
			}
			else
			{
				*reinterpret_cast<T*>(m_buffer_write) = v;
				m_buffer_write = end;
			}
			return *this;
		}

		template<class T, unsigned int N>
		inline simple_binary_oarchive& operator & (const T(&v)[N])
		{
			for (unsigned int i = 0; i < N; ++i)
			{
				(*this)& v[i];
			}
			return *this;
		}

		inline simple_binary_oarchive& operator & (const std::string& v)
		{
			size_t length = v.length();
			(*this)& length;
			save_binary(v.c_str(), length);
			return *this;
		}
		inline simple_binary_oarchive& operator & (const std::wstring& v)
		{
			size_t length = v.length() * sizeof(wchar_t);
			(*this)& length;
			save_binary(reinterpret_cast<const char*>(v.c_str()), length);
			return *this;
		}

		template<class U, class Allocator>
		inline simple_binary_oarchive& operator & (const std::vector<U, Allocator>& v)
		{
			typedef std::vector<U, Allocator> CONTAINER;

			size_t size = v.size();
			(*this)& size;
			for (typename CONTAINER::const_iterator it = v.begin(); it != v.end(); ++it)
			{
				(*this)& (*it);
			}
			return *this;
		}

		template<class Key, class Traits, class Allocator>
		inline simple_binary_oarchive& operator & (const std::set<Key, Traits, Allocator>& v)
		{
			typedef std::set<Key, Traits, Allocator> CONTAINER;

			boost::uint32_t size = v.size();
			(*this)& size;
			for (typename CONTAINER::const_iterator it = v.begin(); it != v.end(); ++it)
			{
				(*this)& (*it);
			}
			return *this;
		}

		template<class Key, class Type, class Traits, class Allocator>
		inline simple_binary_oarchive& operator & (const std::map<Key, Type, Traits, Allocator>& v)
		{
			typedef std::map<Key, Type, Traits, Allocator> CONTAINER;

			boost::uint32_t size = v.size();
			(*this)& size;
			for (typename CONTAINER::const_iterator it = v.begin(); it != v.end(); ++it)
			{
				(*this)& it->first;
				(*this)& it->second;
			}
			return *this;
		}

		//查询接口
		inline const char* str() const
		{
			return m_buffer_begin;
		}

		inline size_t pcount() const
		{
			return m_buffer_write - m_buffer_begin;
		}

		inline void reset()
		{
			m_buffer_write = m_buffer_begin;
		}
		inline void save_binary(const void* address, std::size_t count)
		{
			char* end = m_buffer_write + count;
			if (end > m_buffer_end)
			{//需要重新分配内存
				size_t old_data_size = m_buffer_write - m_buffer_begin;
				size_t new_buffer_size = ((end - m_buffer_begin - 1) / mem_block_size + 1) * mem_block_size;
				char* new_buffer = new char[new_buffer_size];
				memcpy(new_buffer, m_buffer_begin, old_data_size);
				memcpy(new_buffer + old_data_size, address, count);
				delete[] m_buffer_begin;
				m_buffer_begin = new_buffer;
				m_buffer_end = new_buffer + new_buffer_size;
				m_buffer_write = new_buffer + old_data_size + count;
			}
			else
			{
				memcpy(m_buffer_write, address, count);
				m_buffer_write = end;
			}
		}
	private:
		char* m_buffer_begin;	//缓冲区开始位置
		char* m_buffer_end;	//缓冲区结束位置
		char* m_buffer_write;	//缓冲区写位置
	};
} // namespace faith

#ifndef BOOST_SERIALIZATION_DEFAULT_TYPE_INFO   
#include <boost/serialization/extended_type_info_typeid.hpp>   
#endif

#endif // OMP_COMMON_SIMPLE_BINARY_OARCHIVE_HPP
