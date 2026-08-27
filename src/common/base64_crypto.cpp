/********************************************************************
  created: 2014/09/04
  created: 4:9:2014 20:36
  file base: base64_crypto
  file ext: cpp
  author: YU REN
  
  purpose: 
*********************************************************************/
#include <base64/base64_crypto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" 
{
#include <curl/curl.h>
#include "../curl/curl_base64.h"
}

namespace faith
{
	namespace common
	{
//		void base64_crypto::create_string_base64_encode(const char *inputbuff, size_t insize,char **outptr, size_t *outlen)
//		{
//			struct SessionHandle* data = NULL;
//			Curl_base64_encode(data,inputbuff,insize,outptr,outlen);
//		}

//		void base64_crypto::create_string_base64_decode(const char *src,unsigned char **outptr, size_t *outlen)
//		{
//			Curl_base64_decode(src,outptr,outlen);
//		}
	}
}
