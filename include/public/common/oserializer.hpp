/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:38
	file base:	oserializer
	file ext:	hpp
	author:		zhy

	purpose:
*********************************************************************/
#ifndef _OSERIALIZER_H_
#define _OSERIALIZER_H_

//#include <strstream>
#include "singleton.hpp"
#include "simple_binary_oarchive.hpp"

namespace faith
{
	class oserializer :
		public singleton<oserializer>
	{
		friend class singleton<oserializer>;

	public:
		template <class T>
		void save(const T& v, const char*& buffer, std::size_t& size);

		void reset();

		template <class T>
		oserializer& operator << (const T& v);

		void get_data(const char*& buffer, std::size_t& size);
	private:
		oserializer();
		simple_binary_oarchive 	m_oarchive;
	};

	inline oserializer::oserializer()
	{
	}

	inline void oserializer::reset()
	{
		m_oarchive.reset();
	}

	template <class T>
	inline void oserializer::save(const T& v, const char*& buffer, std::size_t& size)
	{
		reset();
		*this << v;
		get_data(buffer, size);
	}

	template <class T>
	inline oserializer& oserializer::operator << (const T& v)
	{
		m_oarchive& v;
		return *this;
	}

	inline void oserializer::get_data(const char*& buffer, std::size_t& size)
	{
		buffer = m_oarchive.str();
		size = m_oarchive.pcount();
	}
}

#endif
