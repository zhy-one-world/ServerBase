/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:20
	file base:	options_container
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "options_container.hpp"
#include "scheduler.hpp"
//#include "faith_logger.hpp"

namespace faith
{
	namespace net
	{
		bool options_container::set_option(const boost::any& option_item,bool init_param/*=false*/)
		{
			if(option_item.empty())
				return false;
			options_map_it it=m_options_map.find(option_item);
			if(init_param)
			{
				if(it!=m_options_map.end())
				{
					return false;
				}

				m_options_map.insert(option_item);
				/*因Scheduler的构造函数里会调用到set_option函数,所以这里会有重入现象.gcc下的boost::call_once会出现wait死锁.故先注释掉.
				*/
				return true;
			}

			if(it==m_options_map.end())
			{
				return false;
			}

			m_options_map.erase(it);
			m_options_map.insert(option_item);

			//*it=option_item;
			return true;
		}

		bool options_container::get_option(boost::any& option_item)
		{
			if(option_item.empty())
				return false;

			options_map_it it=m_options_map.find(option_item);
			if(it==m_options_map.end())
			{
				return false;
			}

			option_item=*it;
			return true;
		}
	}
}
