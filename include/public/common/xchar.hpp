/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:34
	file base:	xchar
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _XCHAR_H_
#define _XCHAR_H_


//#pragma warning(disable : 4819)
//#pragma warning(disable : 4800)
//#pragma warning(disable : 4996)
//#pragma warning(disable : 4244)
//#pragma warning(disable : 4503)
//#pragma warning(disable : 4275)
//#pragma warning(disable : 4996)

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif

#include <string>
#include <sstream>
#include <iostream>

/* type */
typedef char						xchar;
typedef std::string					xstring;
typedef std::ostream				xostream;
typedef std::streambuf				xstreambuf;
typedef std::stringbuf				xstringbuf;
typedef std::istream				xistream;
typedef std::ostringstream			xostringstream;
typedef std::stringstream			xstringstream;
typedef std::ifstream				xifstream;

#define _XTEXT(x)					x

#define __XFILE__					__FILE__
#define __XDATE__					__DATE__
#define __XTIME__					__TIME__
#define __XFUNCTION__				__FUNCTION__

#ifdef _MSC_VER                                                 
	#define _XMAIN					main
#endif

#define XPRINTF						printf
#define XPRINTF_S					printf_s
#define SXPRINTF(X,Y,Z,...)			sprintf((X),(Z),__VA_ARGS__)

#if defined(_MSC_VER)
	#define SXPRINTF_S				sprintf_s
#elif defined(__GNUC__)
	#define SXPRINTF_S				sprintf
#endif

#if defined(_MSC_VER)
	#define SNXPRINTF				_snprintf
#elif defined(__GNUC__)
	#define SNXPRINTF				snprintf
#endif

#define SXSCANF						sscanf
#define SXSCANF_S					sscanf_s

#define ICONV_ONE					_iconv_one
#define XTOI						atoi
#define ANSI2XSTR(S)				(S)
#define XSTR2ANSI(S)				(S)

#define XSTRLEN						strlen
#define XSTRNLEN					strnlen
#define	XSTRCMP						strcmp
#define	XSTRNCPY					strncpy
#define XSTRFTIME					strftime
#define XSTRCSPN					strcspn
#define XSTRCHR						strchr
#define XSTRSTR						strstr

#define XTIME32						_ctime32	//TODO: to supply linux function

#define XMKDIR						_mkdir
#define XFOPEN						fopen
#define XFOPEN_S					fopen_s
#define XFGETS						fgets

#define XSETLOCALE					setlocale

#define ELEMENTOF(ARRAY)			sizeof(ARRAY)/sizeof(ARRAY[0])

#endif