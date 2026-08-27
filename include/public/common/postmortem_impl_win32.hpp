#ifndef __OMP_COMMON_POSTMORTEM_WIN32_FILE__
#define __OMP_COMMON_POSTMORTEM_WIN32_FILE__

#ifdef WIN32

#include "xchar.hpp"

namespace faith {

		class Postmortem_impl
		{
		public:

			static xstring		m_dumpfile_prefix;
			static xstring		m_exec_afterdump;
		
			Postmortem_impl(void)	{};
			~Postmortem_impl(void)	{};

			bool init( xstring dumpfile_prefix,xstring exec_afterdump );
			void release();
		};

	}// end of namespace faith

#endif//#ifdef WIN32

#endif//#define __OMP_COMMON_POSTMORTEM_WIN32_FILE__