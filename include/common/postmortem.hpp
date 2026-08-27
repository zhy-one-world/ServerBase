/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   14:36
	file base:	postmortem
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _POSTMORTEM_MODULE_H_
#define _POSTMORTEM_MODULE_H_

#include "singleton.hpp"
#include "xchar.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>

namespace faith 
{
		class postmortem_impl;
		//
		//	Postmortem class, Singleton class
		//	
		//	Warning:
		//		无法在一个调试器内调试此模块功能，因为异常会被调试器捕获
		//
		class postmortem : public singleton<postmortem>
		{
		public:
			friend class singleton<postmortem>;		
			typedef boost::function<void(void)> cb_t;
		public:
			postmortem();
			~postmortem();
		public:
			//	Name:	Init
			//	Param:
			//			dumpfile_prefix		dump 文件名前缀
			//			exec_afterdump		dump 后执行命令
			//			dump_type			dump 类型,生成信息的内容
			//			use_dump_callback	使用回调函数过滤无用的模块,目前只保存主模块的dump信息.
			bool								init( xstring dumpfile_prefix,xstring exec_afterdump, int dump_type = 1, bool use_dump_callback = true);
			void								release(	);			
			void								register_extern_callback( cb_t handler, xchar * desc );	// Desc:	注册异常后回调处理

		private:
			boost::scoped_ptr<postmortem_impl>	m_impl_ptr;
		};

#ifdef _WIN32
		/// stack overflow 专用
		extern struct _EXCEPTION_POINTERS * g_stackoverflow_except;
		bool dump_stackoverflow();

#	define OMP_STACKOVERFLOW_CATCH_BEGIN() \
		__try{

#	define OMP_STACKOVERFLOW_CATCH_END() \
		}__except( ::faith::common::g_stackoverflow_except = GetExceptionInformation()	\
					, (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW )					\
						&& ::faith::common::dump_stackoverflow() ) {}

#else // no implementation for other platforms
#	define OMP_STACKOVERFLOW_CATCH_BEGIN()  
#	define OMP_STACKOVERFLOW_CATCH_END()  

#endif
}

#endif
