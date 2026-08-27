/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:38
	file base:	string_buffer
	file ext:	hpp
	author:		zhy

	purpose:
*********************************************************************/
#ifndef _STRING_BUFFER_H_
#define _STRING_BUFFER_H_

#include <stdlib.h>
#include <boost/cstdint.hpp>
#include <algorithm>

#undef min

namespace faith
{
	template<class T>
	class basic_string_buffer
	{
	public:
		basic_string_buffer();
		~basic_string_buffer();
		basic_string_buffer(basic_string_buffer<T> const& other);
		basic_string_buffer(const std::basic_string<T>& str);
		basic_string_buffer(const T* str);
		basic_string_buffer(const T* str, size_t length);
		const T* c_str() const;
		size_t length() const;

		basic_string_buffer<T>& operator = (basic_string_buffer<T> const& other);
		basic_string_buffer<T>& operator = (std::basic_string<T> const& str);
		basic_string_buffer<T>& operator = (const T* str);

		bool operator == (const basic_string_buffer& o) const;
		bool operator != (const basic_string_buffer& o) const;
		bool operator < (const basic_string_buffer& o) const;
		bool operator > (const basic_string_buffer& o) const;
		bool operator <= (const basic_string_buffer& o) const;
		bool operator >= (const basic_string_buffer& o) const;
		void assign(const T* str, size_t length);
		operator std::basic_string<T>() const;
	private:
		size_t				m_length;	//the count of characters in buffer
		const T* m_str;		//the pointer to the string
	};

	template <class T>
	inline basic_string_buffer<T>::~basic_string_buffer()
	{
	}

	template <class T>
	inline basic_string_buffer<T>::basic_string_buffer()
		:m_length(0)
		, m_str(NULL)
	{
	}

	template <class T>
	inline basic_string_buffer<T>::basic_string_buffer(basic_string_buffer<T> const& other)
		:m_length(0)
		, m_str(NULL)
	{
		assign(other.c_str(), other.length());
	}

	template <class T>
	basic_string_buffer<T>& basic_string_buffer<T>::operator = (basic_string_buffer<T> const& other)
	{
		assign(other.c_str(), other.length());
		return *this;
	}

	template <class T>
	inline basic_string_buffer<T>::basic_string_buffer(const T* str)
		:m_length(0)
		, m_str(NULL)
	{
		assign(str, std::char_traits<T>::length(str));
	}

	template <class T>
	basic_string_buffer<T>& basic_string_buffer<T>::operator = (const T* str)
	{
		assign(str, std::char_traits<T>::length(str));
		return *this;
	}

	template <class T>
	inline basic_string_buffer<T>::basic_string_buffer(const T* str, size_t length)
		:m_length(0)
		, m_str(NULL)
	{
		assign(str, length);
	}

	template <class T>
	inline basic_string_buffer<T>::basic_string_buffer(std::basic_string<T> const& str)
		:m_length(0)
		, m_str(NULL)
	{
		assign(str.c_str(), str.length());
	}

	template <class T>
	basic_string_buffer<T>& basic_string_buffer<T>::operator = (std::basic_string<T> const& str)
	{
		assign(str.c_str(), str.length());
		return *this;
	}

	template <class T>
	inline void basic_string_buffer<T>::assign(const T* str, size_t length)
	{
		m_str = str;
		m_length = length;
	}

	template <class T>
	inline const T* basic_string_buffer<T>::c_str() const
	{
		return m_str;
	}

	template <class T>
	inline size_t basic_string_buffer<T>::length() const
	{
		return m_length;
	}

	template <class T>
	inline basic_string_buffer<T>::operator std::basic_string<T>() const
	{
		std::basic_string<T> str(m_str, m_length);
		return str;
	}
	template <class T>
	inline bool basic_string_buffer<T>::operator == (const basic_string_buffer& o) const
	{
		if (m_length == o.m_length)
		{
			return memcmp(m_str, o.m_str, m_length * sizeof(T)) == 0;
		}
		else
		{
			return false;
		}
	}

	template <class T>
	inline bool basic_string_buffer<T>::operator != (const basic_string_buffer& o) const
	{
		return !(*this == o);
	}

	template <class T>
	inline bool basic_string_buffer<T>::operator < (const basic_string_buffer& o) const
	{
		size_t min_length = std::min(m_length, o.m_length);
		int cmp = memcmp(m_str, o.m_str, min_length * sizeof(T));
		if (cmp != 0)
		{
			return cmp < 0;
		}
		else
		{
			return m_length < o.m_length;
		}
	}

	template <class T>
	inline bool basic_string_buffer<T>::operator > (const basic_string_buffer& o) const
	{
		boost::uint32_t min_length = std::min(m_length, o.m_length);
		int cmp = memcmp(m_str, o.m_str, min_length * sizeof(T));
		if (cmp != 0)
		{
			return cmp > 0;
		}
		else
		{
			return m_length > o.m_length;
		}
	}

	template <class T>
	inline bool basic_string_buffer<T>::operator <= (const basic_string_buffer& o) const
	{
		return o > *this;
	}

	template <class T>
	inline bool basic_string_buffer<T>::operator >= (const basic_string_buffer& o) const
	{
		return o < *this;
	}

	typedef basic_string_buffer<char>		string_buffer;
	typedef basic_string_buffer<wchar_t>	wstring_buffer;

#ifdef OMP_UNICODE
	typedef wstring_buffer	xstring_buffer;
#else
	typedef string_buffer	xstring_buffer;
#endif
}

//support for serialization
#include "simple_binary_iarchive.hpp"
#include "simple_binary_oarchive.hpp"

namespace boost
{
	namespace serialization
	{
		template<class T>
		inline void serialize(faith::simple_binary_iarchive& ar, faith::basic_string_buffer<T>& v, const unsigned int version)
		{
			boost::uint32_t length;
			ar& length;
			const void* data = ar.read_data((length + 1) * sizeof(T));
			v.assign(reinterpret_cast<const T*>(data), length);
		}

		template<class T>
		inline void serialize(faith::simple_binary_oarchive& ar, faith::basic_string_buffer<T>& v, const unsigned int version)
		{
			ar& v.length();
			ar.save_binary(v.c_str(), v.length() * sizeof(T));
			T tail = 0;
			ar& tail;
		}
	}
}
#endif
