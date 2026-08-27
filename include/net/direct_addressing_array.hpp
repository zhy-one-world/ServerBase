/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:07
	file base:	direct_addressing_array
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _DIRECT_ADDRESSING_H_
#define _DIRECT_ADDRESSING_H_

#include <boost/integer.hpp>
#include <boost/cstdint.hpp>
#include <utility>
#include <map>

namespace faith
{
	namespace net
	{
		template <class t>
		class direct_addressing_array
		{
		public:
			direct_addressing_array() : m_size(0)
			{
				memset(m_data,0,sizeof(m_data));
			}
		public:
			bool empty() const
			{
				return m_size == 0;
			}

			size_t size() const
			{
				return m_size;
			}

			t * find(boost::uint16_t index) const
			{
				return m_data[index];
			}

			bool add(boost::uint16_t index,t * obj)
			{
				if(m_data[index])
				{
					return false;
				}
				else
				{
					m_list.push_back(obj);
					m_data[index] = obj;
					++ m_size;
					return true;
				}
			}

			bool remove(boost::uint16_t index)
			{
				if(m_data[index])
				{
					m_list.remove(m_data[index]);
					m_data[index] = NULL;					
					-- m_size;
					return true;
				}
				else
				{
					return false;
				}
			}

			template <typename Handler>
			void traverse(Handler handler)
			{
				for(typename std::list<t *>::const_iterator it = m_list.begin();it!=m_list.end();)
				{
					t * obj = *it++;
					handler(obj);
				}
			}
		private:
			t *				m_data[boost::integer_traits<boost::uint16_t>::const_max+1];
			std::list<t *>	m_list;
			size_t			m_size;
		};
	}
}

#endif
