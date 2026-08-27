/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   20:18
	file base:	file_system
	file ext:	hpp
	author:		zhy

	purpose:
*********************************************************************/
#ifndef _FILE_H_
#define _FILE_H_

#include "xchar.hpp"
#include <boost/filesystem.hpp>
#include "mlb.hpp"

namespace faith
{
	namespace utility
	{
		namespace filesystem
		{
			static const xchar	list_dir_sep[] = _XTEXT("|");

			xstring				list_dir(const xstring& path);
			bool				is_directory(const xstring& path);
			bool				exists(const xstring& path);

			class file_obj
			{
			public:
				file_obj();
				~file_obj();
			public:
				bool			open(const xchar* filename, const xchar* mode);
				xstring		read(unsigned long dwBufLen);
				xstring		read_all();
				unsigned long	write(const xstring& pData);
				int				flush();
				void			close();
				unsigned long	length();
				int				seek(long offset, int origin);
				unsigned long	tell();
			private:
				bool			mlb_open(const xchar* filename, const xchar* mode);
				xstring		mlb_read(unsigned long dwBufLen);
				xstring		mlb_read_all();
				unsigned long	mlb_write(const xstring& pData);
				int				mlb_flush();
				unsigned long	mlb_length();
				void			mlb_close();
				int				mlb_seek(long offset, int origin);
				unsigned long	mlb_tell();
			private:
				FILE* m_fp;
				unsigned long	m_length;
			};
		}
	}
}

#endif
