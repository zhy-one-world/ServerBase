/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:30
	file base:	options_container
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _OPTIONS_CONTAINER_H_
#define _OPTIONS_CONTAINER_H_

#include <set>
#include <boost/any.hpp>

namespace faith 
{
	namespace net 
	{
		class options_container
		{
			class cmp_type_info
			{
			public:
				bool operator () (const boost::any & a,const boost::any & b) const
				{
					return a.type().before(b.type());
				}
			};
			typedef std::set<boost::any,cmp_type_info>	options_map;
			typedef options_map::iterator				options_map_it;
		public:
			bool	set_option(const boost::any& option_item,bool init_param=false);
			bool	get_option(boost::any& option_item);
			template<class option_type>
			bool	get_option(option_type& dest)
			{
				boost::any any_value=option_type();
				get_option(any_value);
				option_type *ptr=boost::any_cast<option_type>(&any_value);
				if(ptr==NULL) 
					return false;
				dest=*ptr;
				return true;
			}
		private:
			options_map	m_options_map;
		};
	}
}

#endif
