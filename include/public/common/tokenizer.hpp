/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:37
	file base:	tokenizer
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "xchar.hpp"
#include <boost/tokenizer.hpp>
#include <string>

namespace faith
{
	namespace common
	{
#ifdef OMP_UNICODE
	typedef boost::char_separator<wchar_t> xchar_separator;
	typedef boost::tokenizer
	<
		xchar_separator, 
    	std::wstring::const_iterator,
    	std::wstring
	> xtokenizer;
#else
	typedef boost::char_separator<char> xchar_separator;
	typedef boost::tokenizer
	<
		xchar_separator, 
    	std::string::const_iterator,
    	std::string
	> xtokenizer;
#endif
	}
}
#endif
