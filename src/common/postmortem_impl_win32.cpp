/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   14:34
	file base:	postmortem_impl_win32
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifdef _MSC_VER

#include "xchar.hpp"
#include <tchar.h>
#include <iostream>
#include <windows.h>
#include "postmortem_impl_win32.hpp"

#define DUMP_TYPE_FULL		MiniDumpWithFullMemory
#define DUMP_TYPE_NORMAL	(MINIDUMP_TYPE)(MiniDumpNormal | MiniDumpWithDataSegs | MiniDumpWithHandleData)

namespace faith 
{

		static std::vector<postmortem::cb_t>	s_cbs;
		static std::vector<xchar*>				_g_cbs_desc_;
		struct _EXCEPTION_POINTERS*				g_stackoverflow_except = NULL;

		xstring			postmortem_impl::m_dumpfile_prefix;
		xstring			postmortem_impl::m_exec_afterdump;
		MINIDUMP_TYPE	postmortem_impl::m_dump_type;
		bool			postmortem_impl::m_use_dump_callback;

		typedef BOOL (WINAPI *mini_dump_write_dump)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
			CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
			CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
			CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
			);

		/*
		Dump信息的高级过滤.以下MiniDumpCallback和IsDataSectionNeeded函数实现了,只保存程序主模块的dump信息的功能.
		使用者可根据自己的需要增加或减少dump保存的信息.
		*/

		inline BOOL is_module_needed(const WCHAR* pModuleName)
		{
			if(pModuleName == 0)
			{
				return FALSE;
			}

			WCHAR szDbgHelpPath[_MAX_PATH] = L"";
			if (GetModuleFileNameW( NULL, szDbgHelpPath, _MAX_PATH ))
			{
				if(_wcsicmp(pModuleName, szDbgHelpPath) == 0)
					return TRUE;
			}
			return FALSE; 
		}

		inline BOOL CALLBACK mini_dump_callback(PVOID pParam, 
			const PMINIDUMP_CALLBACK_INPUT pInput, 
			PMINIDUMP_CALLBACK_OUTPUT pOutput)
		{
			if(pInput == 0 || pOutput == 0)
				return FALSE;

			switch(pInput->CallbackType)
			{
			case ModuleCallback: 
				if(pOutput->ModuleWriteFlags & ModuleWriteModule) 
					if(!is_module_needed(pInput->Module.FullPath)) 
						pOutput->ModuleWriteFlags &= (~ModuleWriteModule); 
			case IncludeModuleCallback:
			case IncludeThreadCallback:
			case ThreadCallback:
			case ThreadExCallback:
				return TRUE;
			default:;
			}
			return FALSE;
		}

		//	exception handler
		LONG WINAPI top_level_filter( struct _EXCEPTION_POINTERS *pExceptionInfo )
		{
			static LONG retval; retval = EXCEPTION_CONTINUE_SEARCH;
			static HWND hParent; hParent = NULL;						// find a better value for your app
			static SYSTEMTIME time;
			::GetLocalTime( &time );

			static HMODULE hDll; hDll = NULL;
			static TCHAR szDbgHelpPath[_MAX_PATH];

			//	首先在进程当前路径寻找 dbghelp.dll
			if (::GetModuleFileName( NULL, szDbgHelpPath, _MAX_PATH ))
			{
				static TCHAR *pSlash; pSlash = _tcsrchr( szDbgHelpPath, _T('\\') );
				if (pSlash)
				{
					_tcscpy_s( pSlash+1,_MAX_PATH-(pSlash-szDbgHelpPath)-1, _T("DBGHELP.DLL") );
					hDll = ::LoadLibrary( szDbgHelpPath );
				}
			}

			//	再使用系统的缺省路径寻找 dbghelp.dll
			if (hDll==NULL)
			{
				// load any version we can
				hDll = ::LoadLibrary( _T("DBGHELP.DLL") );
			}

			static LPCTSTR szResult; szResult = NULL;
			static TCHAR	szDumpPath[ _MAX_PATH ];
			static TCHAR	szScratch [ _MAX_PATH ];

			//	找到 dll
			if (hDll)
			{
				static mini_dump_write_dump pDump; pDump = (mini_dump_write_dump)::GetProcAddress( hDll, "MiniDumpWriteDump" );
				if (pDump)
				{
					// work out a good place for the dump file
					//			if (!GetTempPath( _MAX_PATH, szDumpPath ))
					//				_tcscpy( szDumpPath, "c:\\temp\\" );
					_stprintf_s( szDumpPath,_MAX_PATH,_T("%s_%02d%02d%02d_%02d-%02d-%02d.dmp"),
						postmortem_impl::m_dumpfile_prefix.c_str(),time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond );

					// don't ask user
					// ask the user if they want to save a dump file
// 					if ( Postmortem_impl::m_exec_afterdump.length() != 0 ||
// 						::MessageBox( NULL,_T("WARNING: An exception occured,save the dump file to debug it？"),_T("Postmortem Module"), MB_YESNO )==IDYES )
					{
						// create the file
						static HANDLE hFile; hFile = ::CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL, NULL );

						if (hFile!=INVALID_HANDLE_VALUE)
						{
							static _MINIDUMP_EXCEPTION_INFORMATION ExInfo;

							ExInfo.ThreadId = ::GetCurrentThreadId();
							ExInfo.ExceptionPointers = pExceptionInfo;
							ExInfo.ClientPointers = NULL;

							MINIDUMP_CALLBACK_INFORMATION mci;
							mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)mini_dump_callback;
							mci.CallbackParam       = 0;

							// write the dump
							BOOL bOK = pDump( ::GetCurrentProcess(), GetCurrentProcessId(), hFile, postmortem_impl::m_dump_type, &ExInfo, NULL, 
								postmortem_impl::m_use_dump_callback ? &mci : NULL );
							if (bOK)
							{
								_stprintf_s( szScratch,_MAX_PATH,_T("Saved dump file to '%s'"), szDumpPath );
								szResult = szScratch;
								retval = EXCEPTION_EXECUTE_HANDLER;
							}
							else
							{
								_stprintf_s( szScratch,_MAX_PATH,_T("Failed to save dump file to '%s' (error %d)"), szDumpPath, GetLastError() );
								szResult = szScratch;
							}
							::CloseHandle(hFile);
						}
						else
						{
							_stprintf_s( szScratch,_MAX_PATH,_T("Failed to create dump file '%s' (error %d)"), szDumpPath, GetLastError() );
							szResult = szScratch;
						}
					}
				}
				else
				{
					szResult = _T("the found DBGHELP.DLL has not MiniDumpWriteDump function");
				}
			}
			else
			{
				szResult = _T("DBGHELP.DLL not found");
			}

			if(postmortem_impl::m_exec_afterdump.length()==0)
			{
				//MessageBox( NULL, szResult, _T("FAITH Postmortem Module"), MB_OK );
			}
			else
			{
				static STARTUPINFO si;
				static PROCESS_INFORMATION pi;
				::ZeroMemory( &si, sizeof(si) );
				si.cb = sizeof(si);
				::ZeroMemory( &pi, sizeof(pi) );
				::CreateProcess(NULL,(xchar*)postmortem_impl::m_exec_afterdump.c_str(),NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
				::CloseHandle( pi.hProcess );
				::CloseHandle( pi.hThread );
			}

			return EXCEPTION_EXECUTE_HANDLER;
		}
		/*
#pragma warning(push)
#pragma warning(disable : 4715) //< "not all control path return a value"

		__declspec(naked) bool dump_stackoverflow()
		{
			if ( !g_stackoverflow_except || g_stackoverflow_except->ExceptionRecord->ExceptionCode != EXCEPTION_STACK_OVERFLOW )
			{
				//return EXCEPTION_CONTINUE_SEARCH;
				goto lb_return;
			}

			static LPBYTE lpPage,actPage,newstackbase,newstacktop,oldstackbase,oldstacktop;
			static SYSTEM_INFO si;
			static MEMORY_BASIC_INFORMATION mi;
			static DWORD allocsize,copysize;
			enum{ TEMP_ALLOC_STACK_SIZE = 1024*100, COPY_STACK_SIZE = 1024*10};

			// Get page size of system
			::GetSystemInfo(&si);

			// Find SP address
			__asm{ mov lpPage, esp };

			// Get allocation base of stack
			::VirtualQuery(lpPage, &mi, sizeof(mi));

			// Go to page beyond current page
			allocsize = ((TEMP_ALLOC_STACK_SIZE+si.dwPageSize-1) / si.dwPageSize) * si.dwPageSize; //< 至少分配TEMP_ALLOC_STACK_SIZE大小,但必须是pagesize的倍数
			lpPage = (LPBYTE)(mi.BaseAddress) - allocsize ;
			actPage = (LPBYTE)::VirtualAlloc(lpPage, allocsize ,MEM_COMMIT,PAGE_READWRITE );
			if (!actPage)
			{
				actPage = (LPBYTE)::VirtualAlloc(NULL, allocsize ,MEM_COMMIT,PAGE_READWRITE );
				__asm
				{
					mov oldstackbase, ebp
					mov oldstacktop, esp
				};
				copysize = (oldstackbase - oldstacktop) > (COPY_STACK_SIZE) ? COPY_STACK_SIZE : (oldstackbase-oldstacktop);
				newstackbase = actPage + allocsize;
				newstacktop = newstackbase - copysize;
				memcpy( newstacktop, oldstacktop, copysize );

				__asm
				{
					mov esp, newstacktop;
					mov ebp, newstackbase;
				}

				top_level_filter( g_stackoverflow_except );
				//UnhandledExceptionFilter( g_stackoverflow_except );

				__asm{
					mov ebp, oldstackbase;
					mov esp, oldstacktop;
				}
			}
			else
			{
				top_level_filter( g_stackoverflow_except );
				//UnhandledExceptionFilter( g_stackoverflow_except );
			}

			exit(-3105);

lb_return: ;
			//return true;
		}
#pragma warning(pop)
		//*/

		void postmortem_impl::register_extern_callback(postmortem::cb_t handler, xchar * desc)
		{
			s_cbs.push_back(handler);
			_g_cbs_desc_.push_back(desc);
		}

		/// 执行所有被注册的，用于非正常关闭时的处理
		static void call_cb()
		{
			for(unsigned int i = 0; i < s_cbs.size(); ++i)
			{
				__try
				{
					s_cbs[i]();

					//#define FAITH_DEBUG_ON_EXCEPTION
#ifdef FAITH_DEBUG_ON_EXCEPTION
					TCHAR szResult[1024];
					_stprintf_s( szResult, 1024, _T("'%s ' OK."), _g_cbs_desc_[i]) ;
					//MessageBox( NULL, szResult, _T("FAITH Postmortem Module"), MB_OK | MB_ICONINFORMATION);
#endif

				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					TCHAR szResult[1024];
					_stprintf_s( szResult, 1024, _T("on_exception callbacker execute fail. desc = '%s'"), _g_cbs_desc_[i]) ;
					//MessageBox( NULL, szResult, _T("FAITH Postmortem Module"), MB_OK | MB_ICONEXCLAMATION);
				}
			}
		}

		static LONG WINAPI handled_exception( struct _EXCEPTION_POINTERS *pExceptionInfo )
		{
			top_level_filter(pExceptionInfo);
			
			call_cb();

			return EXCEPTION_EXECUTE_HANDLER;
		}

		static LONG WINAPI handled_release( struct _EXCEPTION_POINTERS *pExceptionInfo )
		{
			return EXCEPTION_EXECUTE_HANDLER;
		}


		// Desc : 平时控制台关闭时，属非正常退出流程。
		static BOOL WINAPI common_console_handler(DWORD CEvent)
		{
			SetConsoleCtrlHandler((PHANDLER_ROUTINE)common_console_handler,FALSE);
			return TRUE;
		};

		bool postmortem_impl::init( xstring dumpfile_prefix,xstring exec_afterdump, int dump_type, bool use_dump_callback  )
		{
			m_dumpfile_prefix = dumpfile_prefix;
			m_exec_afterdump = exec_afterdump;
			m_use_dump_callback = use_dump_callback;

			if (dump_type == 1)
			{
				m_dump_type = DUMP_TYPE_FULL;
			}
			else if (dump_type == 2)
			{
				m_dump_type = DUMP_TYPE_NORMAL;
			}

			::SetUnhandledExceptionFilter( handled_exception );

			/// 设置控制台关闭时的处理句柄
			::SetConsoleCtrlHandler((PHANDLER_ROUTINE)common_console_handler,TRUE);

			return true;
		}

		void postmortem_impl::release()
		{
			::SetUnhandledExceptionFilter( handled_release );
}
}

#endif
