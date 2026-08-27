/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:39
	file base:	name
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _NAME_H_
#define _NAME_H_

#include <string>

#define OMP_NAME(n)	(#n)
namespace faith
{
	namespace common
	{
		typedef char 							nameelem_t;
		typedef std::basic_string<nameelem_t>	namestr_t;
		typedef nameelem_t*						nameiter_t;
		typedef const nameelem_t*				nameci_t;

		namespace utility
		{
			namestr_t xchar2name( std::wstring const& wstr );
			inline namestr_t const& xchar2name( std::string const& ansi_str) 	{ return ansi_str; }
			inline namestr_t& xchar2name( std::string& ansi_str)				{ return ansi_str; }
		}
	}
}

#endif // __NAME_HEADER_FILE__
