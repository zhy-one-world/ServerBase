/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:37
	file base:	unique_id_generator
	file ext:	hpp
	author:		zhy
	
	purpose:	
*********************************************************************/
#ifndef _UNIQUE_ID_GENERATOR_H_
#define _UNIQUE_ID_GENERATOR_H_

#include <set>
#include <cassert>

namespace faith 
{
	template<class id_type>
	struct unique_id_generator
	{
	private:
		typedef std::set<id_type>		id_set;

	public:
		unique_id_generator( ):
		m_invalid_id(0),
			m_next_id(1),
			m_max_count(0),
			m_min_id(0),
			m_max_id(0)
		{
		}

		explicit unique_id_generator( id_type invalid_id_value ):
			m_invalid_id(invalid_id_value),
			m_next_id(1),
			m_max_count(0),
			m_min_id(0),
			m_max_id(0)
		{
		}

		explicit unique_id_generator( const unique_id_generator& r_copy  )
		{
			m_invalid_id	= r_copy.m_invalid_id;
			m_id_using		= r_copy.m_id_using;
			m_next_id		= r_copy.m_next_id;
			m_max_count		= r_copy.m_max_count;
			m_min_id		= r_copy.m_min_id;
			m_max_id		= r_copy.m_max_id;
		}

		void reset(id_type invalid_id_value, id_type max_count)
		{
			m_invalid_id	= invalid_id_value;
			m_id_using.clear();
			m_next_id		= 1;
			m_max_count		= max_count;
			m_min_id		= 0;
			m_max_id		= 0;
		}

		id_type get_id()
		{
			if(m_id_using.size() >= m_max_count)
			{
				return m_invalid_id;
			}
			for(;;)
			{
				if(m_next_id!=m_invalid_id && m_id_using.insert(m_next_id).second)
				{
					break;
				}
				else
				{
					if (m_max_id != 0)
					{
						if (m_next_id<m_max_id)
							++m_next_id;
						else
							m_next_id = m_min_id;
					}
					else
					{
						++m_next_id;
					}
				}
			}
			return m_next_id++;;
		}
		void  return_id(id_type id)
		{
			size_t num=m_id_using.erase(id);
			//assert(num==1);
		}
		void set_max_count(id_type max_count)
		{
			m_max_count = max_count;
		}

		id_type get_size() const
		{
			return static_cast<id_type>(m_id_using.size());
		}

		void add_reserved_id(id_type id)
		{
			m_id_using.insert(id);
		}

		void set_range(id_type min_id, id_type max_id)
		{
			if (min_id >= max_id)
				return;
			m_min_id = min_id;
			m_max_id = max_id;
			m_next_id = m_min_id;
		}

	private:
		id_type			m_invalid_id;
		id_set			m_id_using;
		id_type			m_next_id;
		id_type			m_max_count;
		id_type			m_min_id;
		id_type			m_max_id;
	};
}	// end namespace faith

#endif
