/********************************************************************
	created:	2014/08/19
	created:	19:8:2014   15:03
	file base:	tinyxml_wrap
	file ext:	cpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#include "tinyxml_wrap.hpp"
#include "file_system.hpp"
#include "mlb.hpp"
#include "mem_pool.hpp"
#include <sstream>
#include <fstream>

#define ENCODING	"UCS-2LE"

namespace faith
{
		char * get_char(const wchar_t * val )
		{
			static char char_buf[1024];
			int		val_len = wcslen(val)+1;
			//assert(val_len < 1024/sizeof(wchar_t));
			//utility::_iconv_one(ENCODING,locale_charset(),(void*)val,val_len*sizeof(wchar_t),(void*)char_buf,sizeof(char_buf));
			memcpy(char_buf, val, val_len*sizeof(wchar_t)< sizeof(char_buf) ? val_len*sizeof(wchar_t): sizeof(char_buf) );
			return char_buf;
		}

		inline void get_xstring(const char* val,xstring & outstr)
		{
			#if defined(FAITH_UNICODE)
				if (val == NULL)
				{
					outstr.clear();
					return;
				}
				int		val_len = strlen(val)+1;				
				wchar_t * wchar_buf = static_cast<wchar_t *>(mem_pool::getInstance().alloc(sizeof(wchar_t)*val_len));
				utility::_iconv_one(locale_charset(),ENCODING,(void*)val,val_len,(void*)wchar_buf,sizeof(wchar_t)*val_len);
				outstr = wchar_buf;
				mem_pool::getInstance().free(wchar_buf,sizeof(wchar_t)*val_len);
			#else
				outstr = val;
			#endif
		}

		bool tixmldocument_wrap::load_file( xstring filename )
		{
			utility::filesystem::file_obj file;
			if(file.open(filename.c_str(),_XTEXT("r")))
			{
				xstring buf = file.read_all();
				impl.Parse(buf.c_str(),0,TIXML_ENCODING_LEGACY);
				return !(impl.Error());
			}
			else
			{
				return false;
			}
		}

		bool tixmldocument_wrap::loadfile_without_mlb( const xchar * filename )
		{
			#if defined(FAITH_UNICODE)
				return impl.LoadFile(get_char(filename));
			#else
				return impl.LoadFile(filename);
			#endif
		}

		void tixmldocument_wrap::parse( const xchar* p )
		{
			#if defined(FAITH_UNICODE)
				impl.Parse(get_char(p),0,TIXML_ENCODING_LEGACY);
			#else
				impl.Parse(p,0,TIXML_ENCODING_LEGACY);
			#endif
		}		

		const xstring tixmlelement_wrap::attribute( const xchar*  name ) const
		{
			xstring ret;
			#if defined(FAITH_UNICODE)
				get_xstring(impl->Attribute(get_char(name)),ret);
			#else
				const char* val = impl->Attribute(name);
				ret = val == NULL ? "" : val;
			#endif
			return ret;
		}

		const xstring tixmlelement_wrap::attribute( const xchar*  name, int* i ) const
		{
			xstring ret;
			#if defined(FAITH_UNICODE)
				get_xstring(impl->Attribute(get_char(name),i),ret);
			#else
				ret = impl->Attribute(name,i);
			#endif
			return ret;
		}

		const xstring tixmlelement_wrap::attribute( const xchar*  name, double* d ) const
		{
			xstring ret;
			#if defined(FAITH_UNICODE)
				get_xstring(impl->Attribute(get_char(name),d),ret);
			#else
				ret = impl->Attribute(name,d);
			#endif
			return ret;
		}

		const xstring tixmlelement_wrap::value() const
		{
			xstring ret;
			get_xstring(impl->Value(),ret);
			return ret;
		}

		const xstring tixmlelement_wrap::get_text() const
		{		
			xstring ret;
			get_xstring(impl->GetText(),ret);
			return ret;
		}

		const char* tixmlelement_wrap::get_text_ansi() const
		{
			return impl->GetText();
		}
}
