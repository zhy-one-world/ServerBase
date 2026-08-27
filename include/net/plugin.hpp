/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   19:15
	file base:	plugin
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _PLUGIN_H_
#define _PLUGIN_H_
#include <boost/function.hpp>

namespace faith 
{
	namespace net 
	{
		enum
		{
			e_plugin_slot_first = -1,
			e_plugin_slot_last = 100001,
			e_plugin_slot_invalid = 0x7FFFFFFF
		};
		typedef boost::function<bool(unsigned int index,const void * buffer,size_t size)> plug_in;
	}
}

#endif
