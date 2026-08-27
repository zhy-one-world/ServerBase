#ifndef OMP_COMMON_SIMPLE_BINARY_IARCHIVE_HPP
#define OMP_COMMON_SIMPLE_BINARY_IARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// simple_binary_iarchive.hpp

// (C) Copyright 2008 Zhang Yongbo
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// note special treatment of shared_ptr. This type needs a special
// structure associated with every archive.  We created a "mix-in"
// class to provide this functionality.  Since shared_ptr holds a
// special esteem in the boost library - we included it here by default.

#include <vector>
#include <set>
#include <map>

#include <boost/cstdint.hpp>
#include <boost/throw_exception.hpp>
#include <boost/archive/archive_exception.hpp>
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
	// via inhertance, derived from binary_iarchive_impl instead.  This will
	// preserve correct static polymorphism.
	class simple_binary_iarchive
	{
	public:
		typedef boost::mpl::bool_<true> is_loading;
		typedef boost::mpl::bool_<false> is_saving;

		inline simple_binary_iarchive() :
			m_buffer_begin(NULL),
			m_buffer_end(NULL),
			m_buffer_read(NULL)
		{
		}

		template<class T>
		inline simple_binary_iarchive& operator >> (const T& v)
		{
			(*this)& const_cast<T&>(v);
			return *this;
		}

		template<class T>
		inline typename boost::disable_if
			<
			boost::mpl::or_<boost::is_arithmetic<T>, boost::is_enum<T> >,
			simple_binary_iarchive&
			>::type operator & (T& v)
		{
			BOOST_MPL_ASSERT_NOT((boost::is_pointer<T>));
			boost::serialization::serialize(*this, v, 0);
			return *this;
		}

		template<class T>
		inline typename boost::enable_if
			<
			boost::mpl::or_<boost::is_arithmetic<T>, boost::is_enum<T> >,
			simple_binary_iarchive&
			>::type operator & (T& v)
		{
			v = *reinterpret_cast<const T*>(read_data(sizeof(v)));
			return *this;
		}

		template<class T, unsigned int N>
		inline simple_binary_iarchive& operator & (T(&v)[N])
		{
			for (unsigned int i = 0; i < N; ++i)
			{
				(*this)& v[i];
			}
			return *this;
		}

		inline simple_binary_iarchive& operator & (std::string& v)
		{
			boost::uint32_t length;
			(*this)& length;
			v.assign(static_cast<const char*>(read_data(length)), length);
			return *this;
		}
		inline simple_binary_iarchive& operator & (std::wstring& v)
		{
			boost::uint32_t length;
			(*this)& length;
			v.assign(static_cast<const wchar_t*>(read_data(length)), length / sizeof(wchar_t));
			return *this;
		}

		template<class U, class Allocator>
		inline simple_binary_iarchive& operator & (std::vector<U, Allocator>& v)
		{
			typedef std::vector<U, Allocator> CONTAINER;

			boost::uint32_t size;
			(*this)& size;
			v.clear();
			for (boost::uint32_t i = 0; i < size; ++i)
			{
				U item;
				(*this)& item;
				v.push_back(item);
			}
			return *this;
		}

		template<class Key, class Traits, class Allocator>
		inline simple_binary_iarchive& operator & (std::set<Key, Traits, Allocator>& v)
		{
			boost::uint32_t size;
			(*this)& size;
			v.clear();
			for (boost::uint32_t i = 0; i < size; ++i)
			{
				Key key;
				(*this)& key;
				v.insert(key);
			}
			return *this;
		}

		template<class Key, class Type, class Traits, class Allocator>
		inline simple_binary_iarchive& operator & (std::map<Key, Type, Traits, Allocator>& v)
		{
			boost::uint32_t size;
			(*this)& size;
			v.clear();
			for (boost::uint32_t i = 0; i < size; ++i)
			{
				std::pair<Key, Type> pair;
				(*this)& pair.first;
				(*this)& pair.second;
				v.insert(pair);
			}
			return *this;
		}

		inline void set_data(const void* buffer, size_t size)
		{
			m_buffer_begin = static_cast<const char*>(buffer);
			m_buffer_end = m_buffer_begin + size;
			m_buffer_read = m_buffer_begin;
		}

		inline void reset_object_address(
			const void* new_address,
			const void* old_address)
		{

		}
		inline const void* read_data(boost::uint32_t size)
		{
			const char* read = m_buffer_read + size;
			if (read > m_buffer_end)
			{
				boost::throw_exception(boost::archive::archive_exception(boost::archive::archive_exception::output_stream_error));
			}
			const void* ret = m_buffer_read;
			m_buffer_read = read;
			return ret;
		}
	private:
		const char* m_buffer_begin;	//ª∫¥Êø™ ºŒª÷√
		const char* m_buffer_end;	//ª∫¥ÊΩ· ¯Œª÷√
		const char* m_buffer_read;	//ª∫¥Ê∂¡Œª÷√
	};
} // namespace faith

#ifndef BOOST_SERIALIZATION_DEFAULT_TYPE_INFO   
#include <boost/serialization/extended_type_info_typeid.hpp>   
#endif

#endif // OMP_COMMON_SIMPLE_BINARY_IARCHIVE_HPP
