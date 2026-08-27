/********************************************************************
  created: 2014/09/04
  created: 4:9:2014 20:36
  file base: base64_crypto
  file ext: h
  author: YU REN
  
  purpose: 
*********************************************************************/
#ifndef _COMMON_BASE64_CRYPTO_H_
#define _COMMON_BASE64_CRYPTO_H_


namespace faith
{
	namespace common
	{
		class base64_crypto
		{
		public:
			base64_crypto();
			base64_crypto(const char* md5src);
			base64_crypto(unsigned long* md5src);
			base64_crypto	operator+(base64_crypto adder);
			bool			operator==(base64_crypto cmper);

			~base64_crypto() {};
		public:
			//void		create_string_base64_encode(const char *inputbuff, size_t insize,char **outptr, size_t *outlen);
			//void		create_string_base64_decode(const char *src,unsigned char **outptr, size_t *outlen);
		};
	}
}

#endif
