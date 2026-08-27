#ifndef _MBTOWC_ANDROID_H_
#define _MBTOWC_ANDROID_H_

#include <stdlib.h>
	#ifdef __ANDROID__
		int wctomb(char *s, wchar_t wc);
		int mbtowc(wchar_t *pwc, const char *s, size_t n);
	#endif
#endif