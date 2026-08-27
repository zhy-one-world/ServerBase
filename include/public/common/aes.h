///////////////////////////////
// http://mingcn.cnblogs.com //
//  xelz CopyRight (c) 2010  //
///////////////////////////////


#if !defined(AFX_AES_H__6BDD3760_BDE8_4C42_85EE_6F7A434B81C4__INCLUDED_)
#define AFX_AES_H__6BDD3760_BDE8_4C42_85EE_6F7A434B81C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "openssl/aes.h"
#include "string"
class AES  
{
public:
	static std::string aes_128_ecb_encrypt(const std::string& source, const std::string& key);
	static std::string aes_128_ecb_decrypt(const std::string& ciphertext, const std::string& key);
};

#endif // !defined(AFX_AES_H__6BDD3760_BDE8_4C42_85EE_6F7A434B81C4__INCLUDED_)
