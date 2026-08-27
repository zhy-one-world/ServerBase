/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   14:31
	file base:	postmortem_impl_win32
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _POSTMORTEM_WIN32_H_
#define _POSTMORTEM_WIN32_H_

//#ifdef WIN32

#include <windows.h>
#include <dbghelp.h>
#include <vector>
#include "xchar.hpp"
#include "postmortem.hpp"

namespace faith {
		class postmortem_impl
		{
		public:
			postmortem_impl( void )	{};
			~postmortem_impl( void )	{};
		public:
			bool					init( xstring dumpfile_prefix,xstring exec_afterdump, int dump_type, bool use_dump_callback );
			void					release(	);
			void					register_extern_callback( postmortem::cb_t handler, xchar * desc );
		public:
			static xstring			m_dumpfile_prefix;
			static xstring			m_exec_afterdump;
			static MINIDUMP_TYPE	m_dump_type;
			static bool				m_use_dump_callback;

		};
}

#endif

//#endif
