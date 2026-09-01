/*base_64.h文件*/
#pragma once
/**
* Base64 编码/解码
* @author liruixing
*/
namespace faith {
	class Base64 {
	private:
		static std::string _base64_table;
		static const char base64_pad = '=';
	public:
		/**
		* 这里必须是unsigned类型，否则编码中文的时候出错
		*/
		static std::string Encode(const unsigned char* str, int bytes);
		static std::string Decode(const char* str, int bytes);
	};
}