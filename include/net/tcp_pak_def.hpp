/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   18:56
	file base:	tcp_pak_def
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/

#ifndef _TCP_PAK_DEF_H_
#define _TCP_PAK_DEF_H_

#ifdef _SERVER
#include <boost/static_assert.hpp>
#endif
#include <cstdlib>
#include <string.h>

namespace faith 
{
	namespace net 
	{
		//	TCP Packet Define

#pragma pack(push,1)

		struct tcp_pak_header
		{
			char			flag[2];
			unsigned short	key;
			unsigned int	length;
			inline void reset()
			{
				//flag[0] = 'f';
				//flag[1] = 'l';
				//flag[2] = 'a';
				//flag[3] = 'g';

				// WARNING:
				//		only for little-endian processor now !!!
				*((unsigned int*)flag) = 0x67616c66;
				length = 0;
				key = 0;
			}

#ifdef _SERVER
			bool check()
			{
#ifdef _DEBUG
				char	str[5];

				memset( str,'\0',sizeof(str) );
				memcpy( str,&length,4 );

				if( str[0] == '0' && ( str[1] == 'x' || str[1] == 'X' ) )
				{
					char *ptr_stop(0);
					length = std::strtoul( str,&ptr_stop,16 );
					if( ptr_stop != &str[4] )
						return false;
				}

#endif//#ifdef _DEBUG

				// WARNING:
				//		only for little-endian processor now !!!
				return( *((unsigned int*)flag) == 0x67616c66 && length );
			}
#endif   
		};

#pragma pack(pop)

#ifdef _SERVER 
		BOOST_STATIC_ASSERT( sizeof(char)==1 );
		BOOST_STATIC_ASSERT( sizeof(int)==4 );
		BOOST_STATIC_ASSERT( sizeof(tcp_pak_header)==8 );
#else		
		static_assert(sizeof(char) == 1, "sizeof(char) == 1");
		static_assert(sizeof(int) == 4, "sizeof(int) == 4");
		static_assert(sizeof(tcp_pak_header) == 8, "sizeof(tcp_pak_header) == 8");
#endif

	}	// end of namespace net

}	// end of namespace faith

#endif//#define __FAITH_TCPPAKDEF_H_
