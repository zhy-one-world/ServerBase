/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   22:02
	file base:	div
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _DIV_H_
#define _DIV_H_

namespace faith
{
	namespace common
	{
		template <const int DIVISOR,class T>
		T div(const T & v)
		{
			return v > 0?
				v / DIVISOR:
				(v - DIVISOR +1 )/DIVISOR;
		}

		template <class T>
		T div(const int DIVISOR,const T & v)
		{
			return v > 0?
				v / DIVISOR:
			(v - DIVISOR +1 )/DIVISOR;
		}
	}
}
#endif
