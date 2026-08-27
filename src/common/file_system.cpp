/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:18
	file base:	file_system
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "file_system.hpp"
#include <stdio.h>

// Microsoft compiler security
namespace faith
{
		namespace utility
		{
			//namespace filesystem
			//{

			//	MLB_FUNC_1(xstring,list_dir,const xstring &,path)
			//	{
			//		xstring ret;

			//		boost::filesystem::directory_iterator end_itr; 
			//		for( boost::filesystem::directory_iterator itr( path ); itr != end_itr; ++itr )
			//		{
			//			ret += itr->path().leaf().generic_string() + list_dir_sep;
			//		}
			//		ret.erase(ret.size()-1);

			//		return ret;
			//	}

			//	MLB_FUNC_1(bool,is_directory,const xstring & ,path)
			//	{
			//		return boost::filesystem::is_directory(path);
			//	}

			//	MLB_FUNC_1(bool,exists,const xstring & ,path)
			//	{
			//		return boost::filesystem::exists(path);
			//	}

			//	file_obj::file_obj():
			//		m_fp(NULL),
			//		m_length(0)
			//	{

			//	}

			//	file_obj::~file_obj()
			//	{
			//		close();
			//	}

			//	MLB_CLASS_FUNC_2(bool, file_obj, open, const xchar*, filename, const xchar*, mode)
			//	{
			//		if (m_fp)
			//		{
			//			close();
			//		}
			//		#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
			//			errno_t err = XFOPEN_S( &m_fp, filename, mode );
			//		#else
			//			m_fp =  XFOPEN( filename, mode );
			//		#endif
			//			if ( m_fp ) 
			//			{ 
			//				return true;
			//			}
			//			return false;
			//	}

			//	MLB_CLASS_FUNC_1(xstring, file_obj, read, unsigned long, dwBufLen)
			//	{
			//		xchar* buf  = new xchar[dwBufLen];
			//		int len = fread(buf, 1, dwBufLen, m_fp);
			//		xstring ret(buf,len);
			//		delete [] buf;	
			//		return ret;
			//	}

			//	MLB_CLASS_FUNC_0(xstring, file_obj, read_all)
			//	{
			//		xstring ret = "";
			//		unsigned long len = mlb_length();
			//		if (len == 0)
			//		{
			//			return ret;
			//		}	
			//		return mlb_read(len);
			//	}

			//	MLB_CLASS_FUNC_1(unsigned long, file_obj, write, const xstring &, pData)
			//	{
			//		return fwrite(pData.c_str(), 1, pData.length(), m_fp);
			//	}

			//	MLB_CLASS_FUNC_0(int, file_obj, flush)
			//	{
			//		return fflush(m_fp);
			//	}

			//	MLB_CLASS_FUNC_0(void, file_obj, close)
			//	{
			//		if(m_fp != NULL)
			//		{
			//			fclose(m_fp);
			//			m_fp = NULL;
			//		}
			//	}

			//	MLB_CLASS_FUNC_0(unsigned long, file_obj, length)
			//	{
			//		//get file's length
			//		fseek(m_fp, 0, SEEK_END);
			//		m_length = ftell( m_fp );
			//		fseek(m_fp, 0, SEEK_SET);
			//		return m_length;
			//	}

			//	MLB_CLASS_FUNC_2(int, file_obj, seek, long, offset, int, origin)
			//	{
			//		return fseek(m_fp, offset, origin);
			//	}

			//	MLB_CLASS_FUNC_0(unsigned long,file_obj,tell)
			//	{
			//		return ftell( m_fp );
			//	}
			//}
		}
}
