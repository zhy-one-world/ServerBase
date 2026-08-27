/********************************************************************
  created: 2014/08/09
  created: 9:8:2014 17:09
  file base: md5_crypto
  file ext: h
  author: zhy

  purpose:
*********************************************************************/
#ifndef _COMMON_MD5_CRYPTO_H_
#define _COMMON_MD5_CRYPTO_H_

#include <string>

namespace faith
{
	// 		#define CONTEXT_TOTAL_LEN		2
	// 		#define CONTEXT_STATE_LEN		4
	// 		#define CONTEXT_BUFFER_LEN		64
	// 		#define DIGEST_LEN				16
	// 		#define MD5_STRING_SUB_BEGIN	8
	// 		#define	MD5_STRING_SUB_NUMS		16

	class md5_crypto
	{
		// 			struct md5_context
		// 			{
		// 				unsigned long int	total[CONTEXT_TOTAL_LEN];
		// 				unsigned long int	state[CONTEXT_STATE_LEN];
		// 				unsigned char		buffer[CONTEXT_BUFFER_LEN];
		// 			};
	public:
		md5_crypto();
		md5_crypto(const char* md5src);
		md5_crypto(unsigned long* md5src);
		md5_crypto	operator+(md5_crypto adder);
		bool		operator==(md5_crypto cmper);
	public:
		~md5_crypto() {};
	public:
		void		create_string_md5(unsigned char* output, const unsigned char* input);
	};
}

#endif
