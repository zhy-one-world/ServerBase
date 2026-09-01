#ifndef BASE64_1_H
#define BASE64_1_H

#include <boost/cstdint.hpp>

namespace faith
{
	class base64
	{
	public: 
		//result value must release by delete[] after no use.
		static char* encode(
			const void* pvSrcBinaryBuffer, 
			int iSrcSize, 
			int iMaxSizeToRead = 0);

		//stBytesWritten:传入目标buffer的大小，传出实际使用大小
		static bool decode(
			void* pvDestBinaryBuffer, 
			const char* pcSrcBase64Ascii,
			size_t& stBytesWritten);

	private:
		static const unsigned char ms_aucBase64Char[65];
		static const unsigned char ms_aucBase64ToBit6[128];
	};
}


#endif // BASE64_H



