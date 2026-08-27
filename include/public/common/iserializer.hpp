/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   22:06
	file base:	iserializer
	file ext:	hpp
	author:		zhy
	
	purpose:	
*********************************************************************/
#ifndef _ISERIALIZER_H_
#define _ISERIALIZER_H_

#include "singleton.hpp"
#include "simple_binary_iarchive.hpp"

namespace faith
{
	class iserializer :
		public singleton<iserializer>
	{
		friend class singleton<iserializer>;

	public:
		template <class T>
		void load(const char* buffer, std::size_t size, T& v);

		void set_data(const char* buffer, std::size_t size);

		template <class T>
		iserializer& operator >> (T& v);
	private:
		iserializer();

		simple_binary_iarchive 	m_iarchive;
	};

	inline iserializer::iserializer()
	{
	}

	template <class T>
	inline void iserializer::load(const char* buffer, std::size_t size, T& v)
	{
		m_iarchive.set_data(buffer, size);
		*this >> v;
	}

	inline void iserializer::set_data(const char* buffer, std::size_t size)
	{
		m_iarchive.set_data(buffer, size);
	}

	template <class T>
	inline iserializer& iserializer::operator >> (T& v)
	{
		m_iarchive& v;
		return *this;
	}
}

#endif
