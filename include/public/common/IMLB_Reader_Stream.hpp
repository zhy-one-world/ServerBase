#ifndef __IMLB_READER_STREAM_H__
#define __IMLB_READER_STREAM_H__

#include "xchar.hpp"
#include "MlbParam.hpp"
#include "name.hpp"
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/cstdint.hpp>


///
/// 提取月光宝盒流前的事件。 hotbakcup中，通过实现它，来请求proxy代理
///
#define TEXT1(z, n, text) text(n)
#define DEF_FN(N)	\
	virtual void before_read(xchar* class_name, xchar* function_name, boost::uint64_t instance_id BOOST_PP_ENUM_TRAILING_PARAMS(N, MlbParam arg)){};


namespace faith
{
	namespace common
	{

		///
		/// 月光宝盒流重定向接口，将月光宝盒流 从文件或网络中提前
		///
		class IMLB_Reader_Stream
		{
		public:
			virtual bool Open(const xchar* szFileName)=0;
			BOOST_PP_REPEAT(10, TEXT1, DEF_FN)


			/// 当前月光宝盒操作的流数据的类型，如 callback/calling/sumcheck 等
			virtual void SetWorkStatus(int status){};
			virtual int GetWorkStatus(){ return 0; };

			virtual size_t Read(xchar* pBuf, size_t dwBufLen)=0;
			virtual void Close(void)=0;
			virtual void Release(void)=0;
			virtual size_t Peek(xchar* pBuf, size_t dwBufLen){ return 0; }
		};
	}
}

#undef TEXT1
#undef DEF_FN

#endif

