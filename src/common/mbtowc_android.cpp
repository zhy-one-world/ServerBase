#include "mbtowc_android.h"

#ifdef __ANDROID__
	//int wctomb(char *s, wchar_t wc) { return wcrtomb(s,wc,NULL); }
	//int mbtowc(wchar_t *pwc, const char *s, size_t n) { return mbrtowc(pwc, s, n, NULL); }
#endif
