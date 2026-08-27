/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:24
	file base:	persistence_id_generator
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "persistence_id_generator.hpp"

namespace faith
{
	namespace common
	{
		persistence_id_generator::persistence_id_generator()
		{

		}

		persistence_id_generator::~persistence_id_generator()
		{
// 			for(id_generator_map::const_iterator it = m_id_generator_map.begin();it!=m_id_generator_map.end();++it)
// 			{
// 				delete it->second;
// 			}
		}

		boost::uint32_t persistence_id_generator::get_id(const xchar *category)
		{
			id_generator_map::iterator it = m_id_generator_map.find(category);
			
			if(it!=m_id_generator_map.end())
			{
				return it->second.get_id();
			}
			else
			{
				
				//m_id_generator_map.insert(std::make_pair(category,id_generator(persistence_id_generator::s_invalid_id)));
				m_id_generator_map[category] = id_generator(persistence_id_generator::s_invalid_id);
				m_id_generator_map[category].set_max_count(0xFFFFFFFF);
				return m_id_generator_map[category].get_id();
			}			
		}

		void persistence_id_generator::return_id(const xchar *category,boost::uint32_t id)
		{
			id_generator_map::iterator it = m_id_generator_map.find(category);
			if(it!=m_id_generator_map.end())
			{
				it->second.return_id(id);
				if(it->second.get_size()==0)
				{
					//delete it->second;
					it->second.reset(persistence_id_generator::s_invalid_id, 0xFFFFFFFF);
				}
			}
		}
	}
}
