/********************************************************************
  created: 2014/08/09
  created: 9:8:2014 17:10
  file base: md5_crypto
  file ext: cpp
  author: YU REN
  
  purpose: 
*********************************************************************/
#include <md5/md5_crypto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../curl/curl_md5.h"

namespace faith
{
	namespace common
	{
		md5_crypto::md5_crypto()
		{
	
		}

		void md5_crypto::create_string_md5(unsigned char* output,const unsigned char* input)
		{
			Curl_md5it(output,input);
		}
	}
}
