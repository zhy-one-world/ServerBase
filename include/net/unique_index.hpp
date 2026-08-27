/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:13
	file base:	unique_index
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _UNIQUE_INDEX_H_
#define _UNIQUE_INDEX_H_

#include <set>
#include <list>
#include <cassert>

namespace faith 
{
	namespace net 
	{
		template<class id_type,size_t size>
		struct unique_id_container
		{
		private:
			typedef std::set<id_type>		id_set;
			typedef std::list<id_type>		id_list;

		public:
			explicit unique_id_container( id_type invalid_id_value ) 
				:m_invalid_id(invalid_id_value)
			{
				assert(invalid_id_value >= size);
				for(int i=0;i<size;i++)
				{
					m_id_usable.push_back(i);
				}
			}
			void set_invalid_id(id_type invalid_id_value)
			{
				m_invalid_id = invalid_id_value;
			}
			id_type get_id()
			{
				if(m_id_usable.empty())
				{
					return m_invalid_id;
				}

				id_type index = m_id_usable.front();
				m_id_using.insert(index);
				m_id_usable.pop_front();
				return index;
			}
			void  return_id(id_type id)
			{
				size_t num=m_id_using.erase(id);
				assert(num==1);

				m_id_usable.push_back(id);
			}

		private:
			const id_type	m_invalid_id;
			id_set			m_id_using;
			id_list			m_id_usable;
		};
	}
}

#endif
