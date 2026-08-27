/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:35
	file base:	xchar
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "xchar.hpp"
#include <string.h>
#include "mem_pool.hpp"
//#include "hash_container.hpp"

bool load_textdomain(const xchar* localcode, const xchar* domain, const xchar* dirname)
{
	XSETLOCALE(LC_ALL,localcode);
	return true;
}

bool set_textdomain(const xchar* domain)
{
	return true;
}
