/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   22:07
	file base:	loop_int
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _LOOP_INT_H_
#define _LOOP_INT_H_

#include <boost/integer/integer_mask.hpp>

namespace boost
{
	namespace serialization
	{
		class access;
	}
}

namespace faith
{
	namespace common
	{
		template <class T>
		class loop_int
		{
			friend class boost::serialization::access;
			template<class Archive>
			inline void serialize(Archive & ar, const unsigned int version)
			{
				ar & m_val;
			}
		public:
			loop_int()
			{

			}

			loop_int(const T & v):
				m_val(v)
			{

			}

			loop_int(const loop_int & o):
				m_val(o.m_val)
			{

			}

			bool operator < (const loop_int & o) const
			{
				return o > (*this);
			}
			bool operator > (const loop_int & o) const
			{
				T d = m_val - o.m_val;
				return d > 0 && d <= MAX_DIFF;
			}

			loop_int & operator += (const T & v)
			{
				m_val += v;
				return *this;
			}

			loop_int & operator -= (const T & v)
			{
				m_val -= v;
				return *this;
			}

			loop_int & operator ++ ()
			{
				return (*this)+=1;
			}

			loop_int & operator -- ()
			{
				return (*this)-=1;
			}

			bool operator == (const loop_int & o) const
			{
				return m_val == o.m_val;
			}
			bool operator != (const loop_int & o) const
			{
				return !(*this == o);
			}
			operator T() const
			{
				return m_val;
			}
		private:
			enum
			{
				MAX_DIFF = boost::low_bits_mask_t<sizeof(T)*8-1>::sig_bits
			};
			T	m_val;
		};
	}
}
#endif
