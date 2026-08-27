/********************************************************************
	created:	2014/05/04
	created:	4:5:2014   22:09
	file base:	mlb
	file ext:	hpp
	author:		lucifer~yu
	
	purpose:	
*********************************************************************/
#ifndef _MLB_H_
#define _MLB_H_

#include <boost/bind.hpp>
#include "mlb_helper.hpp"

#define MLB_FUNC_0(RESULT,NAME)	RESULT NAME()

#define MLB_FUNC_1(RESULT,NAME,											\
			ARG1_TYPE,ARG1_NAME											\
			)															\
	extern const xchar sz_mlb_##NAME[] = _XTEXT(#NAME);							\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME											\
			);															\
	mlb_helper<sz_mlb_##NAME,RESULT (						\
			ARG1_TYPE													\
			) >  NAME##_gettler(s_##NAME);								\
	RESULT NAME (														\
			ARG1_TYPE ARG1_NAME											\
			)															\
	{																	\
		return NAME##_gettler.operator()								\
			(															\
			ARG1_NAME													\
			);															\
	}																	\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME											\
			)															\

#define MLB_FUNC_2(RESULT,NAME,											\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME											\
			)															\
	extern const xchar sz_mlb_##NAME[] = _XTEXT(#NAME);							\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME											\
			);															\
	mlb_helper<sz_mlb_##NAME,RESULT (						\
			ARG1_TYPE,													\
			ARG2_TYPE													\
			) >  NAME##_gettler(s_##NAME);								\
	RESULT NAME (														\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME											\
			)															\
	{																	\
		return NAME##_gettler.operator()								\
			(															\
			ARG1_NAME,													\
			ARG2_NAME													\
			);															\
	}																	\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME											\
			)															\

#define MLB_FUNC_3(RESULT,NAME,											\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME											\
			)															\
	extern const xchar sz_mlb_##NAME[] = _XTEXT(#NAME);							\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME											\
			);															\
	mlb_helper<sz_mlb_##NAME,RESULT (						\
			ARG1_TYPE,													\
			ARG2_TYPE,													\
			ARG3_TYPE													\
			) >  NAME##_gettler(s_##NAME);								\
	RESULT NAME (														\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME											\
			)															\
	{																	\
		return NAME##_gettler.operator()								\
			(															\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME													\
			);															\
	}																	\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME											\
			)															\

#define MLB_FUNC_4(RESULT,NAME,											\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME,										\
			ARG4_TYPE,ARG4_NAME											\
			)															\
	extern const xchar sz_mlb_##NAME[] = _XTEXT(#NAME);							\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME											\
			);															\
	mlb_helper<sz_mlb_##NAME,RESULT (						\
			ARG1_TYPE,													\
			ARG2_TYPE,													\
			ARG3_TYPE,													\
			ARG4_TYPE													\
			) >  NAME##_gettler(s_##NAME);								\
	RESULT NAME (														\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME											\
			)															\
	{																	\
		return NAME##_gettler.operator()								\
			(															\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME,													\
			ARG4_NAME													\
			);															\
	}																	\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME											\
			)															\

#define MLB_FUNC_5(RESULT,NAME,											\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME,										\
			ARG4_TYPE,ARG4_NAME,										\
			ARG5_TYPE,ARG5_NAME											\
			)															\
	extern const xchar sz_mlb_##NAME[] = _XTEXT(#NAME);							\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME											\
			);															\
	mlb_helper<sz_mlb_##NAME,RESULT (						\
			ARG1_TYPE,													\
			ARG2_TYPE,													\
			ARG3_TYPE,													\
			ARG4_TYPE,													\
			ARG5_TYPE													\
			) >  NAME##_gettler(s_##NAME);								\
	RESULT NAME (														\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME											\
			)															\
	{																	\
		return NAME##_gettler.operator()								\
			(															\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME,													\
			ARG4_NAME,													\
			ARG5_NAME													\
			);															\
	}																	\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME											\
			)															\

#define MLB_FUNC_6(RESULT,NAME,											\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME,										\
			ARG4_TYPE,ARG4_NAME,										\
			ARG5_TYPE,ARG5_NAME,										\
			ARG6_TYPE,ARG6_NAME											\
			)															\
	extern const xchar sz_mlb_##NAME[] = _XTEXT(#NAME);							\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME											\
			);															\
	mlb_helper<sz_mlb_##NAME,RESULT (						\
			ARG1_TYPE,													\
			ARG2_TYPE,													\
			ARG3_TYPE,													\
			ARG4_TYPE,													\
			ARG5_TYPE,													\
			ARG6_TYPE													\
			) >  NAME##_gettler(s_##NAME);								\
	RESULT NAME (														\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME											\
			)															\
	{																	\
		return NAME##_gettler.operator()								\
			(															\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME,													\
			ARG4_NAME,													\
			ARG5_NAME,													\
			ARG6_NAME													\
			);															\
	}																	\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME											\
			)															\

#define MLB_FUNC_7(RESULT,NAME,											\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME,										\
			ARG4_TYPE,ARG4_NAME,										\
			ARG5_TYPE,ARG5_NAME,										\
			ARG6_TYPE,ARG6_NAME,										\
			ARG7_TYPE,ARG7_NAME											\
			)															\
	extern const xchar sz_mlb_##NAME[] = _XTEXT(#NAME);							\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME											\
			);															\
	mlb_helper<sz_mlb_##NAME,RESULT (						\
			ARG1_TYPE,													\
			ARG2_TYPE,													\
			ARG3_TYPE,													\
			ARG4_TYPE,													\
			ARG5_TYPE,													\
			ARG6_TYPE,													\
			ARG7_TYPE													\
			) >  NAME##_gettler(s_##NAME);								\
	RESULT NAME (														\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME											\
			)															\
	{																	\
		return NAME##_gettler.operator()								\
			(															\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME,													\
			ARG4_NAME,													\
			ARG5_NAME,													\
			ARG6_NAME,													\
			ARG7_NAME													\
			);															\
	}																	\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME											\
			)															\

#define MLB_FUNC_8(RESULT,NAME,											\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME,										\
			ARG4_TYPE,ARG4_NAME,										\
			ARG5_TYPE,ARG5_NAME,										\
			ARG6_TYPE,ARG6_NAME,										\
			ARG7_TYPE,ARG7_NAME,										\
			ARG8_TYPE,ARG8_NAME											\
			)															\
	extern const xchar sz_mlb_##NAME[] = _XTEXT(#NAME);							\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME,										\
			ARG8_TYPE ARG8_NAME											\
			);															\
	mlb_helper<sz_mlb_##NAME,RESULT (						\
			ARG1_TYPE,													\
			ARG2_TYPE,													\
			ARG3_TYPE,													\
			ARG4_TYPE,													\
			ARG5_TYPE,													\
			ARG6_TYPE,													\
			ARG7_TYPE,													\
			ARG8_TYPE													\
			) >  NAME##_gettler(s_##NAME);								\
	RESULT NAME (														\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME,										\
			ARG8_TYPE ARG8_NAME											\
			)															\
	{																	\
		return NAME##_gettler.operator()								\
			(															\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME,													\
			ARG4_NAME,													\
			ARG5_NAME,													\
			ARG6_NAME,													\
			ARG7_NAME,													\
			ARG8_NAME													\
			);															\
	}																	\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME,										\
			ARG8_TYPE ARG8_NAME											\
			)															\

#define MLB_FUNC_9(RESULT,NAME,											\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME,										\
			ARG4_TYPE,ARG4_NAME,										\
			ARG5_TYPE,ARG5_NAME,										\
			ARG6_TYPE,ARG6_NAME,										\
			ARG7_TYPE,ARG7_NAME,										\
			ARG8_TYPE,ARG8_NAME,										\
			ARG9_TYPE,ARG9_NAME											\
			)															\
	extern const xchar sz_mlb_##NAME[] = _XTEXT(#NAME);							\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME,										\
			ARG8_TYPE ARG8_NAME,										\
			ARG9_TYPE ARG9_NAME											\
			);															\
	mlb_helper<sz_mlb_##NAME,RESULT (						\
			ARG1_TYPE,													\
			ARG2_TYPE,													\
			ARG3_TYPE,													\
			ARG4_TYPE,													\
			ARG5_TYPE,													\
			ARG6_TYPE,													\
			ARG7_TYPE,													\
			ARG8_TYPE,													\
			ARG9_TYPE													\
			) >  NAME##_gettler(s_##NAME);								\
	RESULT NAME (														\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME,										\
			ARG8_TYPE ARG8_NAME,										\
			ARG9_TYPE ARG9_NAME											\
			)															\
	{																	\
		return NAME##_gettler.operator()								\
			(															\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME,													\
			ARG4_NAME,													\
			ARG5_NAME,													\
			ARG6_NAME,													\
			ARG7_NAME,													\
			ARG8_NAME,													\
			ARG9_NAME													\
			);															\
	}																	\
	static RESULT s_##NAME(												\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME,										\
			ARG8_TYPE ARG8_NAME,										\
			ARG9_TYPE ARG9_NAME											\
			)															\

#define MLB_CLASS_FUNC_0(RESULT,CLASS,NAME)								\
	namespace															\
	{																	\
		extern xchar sz_##NAME[] = _XTEXT(#CLASS) _XTEXT("::") _XTEXT(#NAME);						\
	}																	\
	RESULT CLASS::NAME()												\
	{																	\
		mlb_helper											\
		<																\
			sz_##NAME,													\
			RESULT ()													\
		> helper														\
		(																\
			boost::bind													\
			(															\
				&CLASS::mlb_##NAME,this									\
			)															\
		);																\
		return helper();												\
	}																	\
	RESULT CLASS::mlb_##NAME()											\


#define MLB_CLASS_FUNC_1(RESULT,CLASS,NAME,								\
			ARG1_TYPE,ARG1_NAME											\
			)															\
	namespace															\
	{																	\
		extern xchar sz_##NAME[] = _XTEXT(#CLASS) _XTEXT("::") _XTEXT(#NAME);						\
	}																	\
	RESULT CLASS::NAME(													\
			ARG1_TYPE ARG1_NAME											\
			)															\
	{																	\
		mlb_helper											\
		<																\
			sz_##NAME,													\
			RESULT														\
			(															\
				ARG1_TYPE												\
			)															\
		> helper														\
		(																\
			boost::bind													\
			(															\
				&CLASS::mlb_##NAME,this,								\
				_1														\
			)															\
		);																\
		return helper													\
		(																\
			ARG1_NAME													\
		);																\
	}																	\
	RESULT CLASS::mlb_##NAME(											\
			ARG1_TYPE ARG1_NAME											\
			)															\

#define MLB_CLASS_FUNC_2(RESULT,CLASS,NAME,								\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME											\
			)															\
	namespace															\
	{																	\
		extern xchar sz_##NAME[] = _XTEXT(#CLASS) _XTEXT("::") _XTEXT(#NAME);						\
	}																	\
	RESULT CLASS::NAME(													\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME											\
			)															\
	{																	\
		mlb_helper											\
		<																\
			sz_##NAME,													\
			RESULT														\
			(															\
				ARG1_TYPE,												\
				ARG2_TYPE												\
			)															\
		> helper														\
		(																\
			boost::bind													\
			(															\
				&CLASS::mlb_##NAME,this,								\
				_1,_2													\
			)															\
		);																\
		return helper													\
		(																\
			ARG1_NAME,													\
			ARG2_NAME													\
		);																\
	}																	\
	RESULT CLASS::mlb_##NAME(											\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME											\
			)															\

#define MLB_CLASS_FUNC_3(RESULT,CLASS,NAME,								\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME											\
			)															\
	namespace															\
	{																	\
		extern xchar sz_##NAME[] = _XTEXT(#CLASS) _XTEXT("::") _XTEXT(#NAME);						\
	}																	\
	RESULT CLASS::NAME(													\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME											\
			)															\
	{																	\
		mlb_helper											\
		<																\
			sz_##NAME,													\
			RESULT														\
			(															\
				ARG1_TYPE,												\
				ARG2_TYPE,												\
				ARG3_TYPE												\
			)															\
		> helper														\
		(																\
			boost::bind													\
			(															\
				&CLASS::mlb_##NAME,this,								\
				_1,_2,_3												\
			)															\
		);																\
		return helper													\
		(																\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME													\
		);																\
	}																	\
	RESULT CLASS::mlb_##NAME(											\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME											\
			)															\

#define MLB_CLASS_FUNC_4(RESULT,CLASS,NAME,								\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME,										\
			ARG4_TYPE,ARG4_NAME											\
			)															\
	namespace															\
	{																	\
		extern xchar sz_##NAME[] = _XTEXT(#CLASS) _XTEXT("::") _XTEXT(#NAME);						\
	}																	\
	RESULT CLASS::NAME(													\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME											\
			)															\
	{																	\
		mlb_helper											\
		<																\
			sz_##NAME,													\
			RESULT														\
			(															\
				ARG1_TYPE,												\
				ARG2_TYPE,												\
				ARG3_TYPE,												\
				ARG4_TYPE												\
			)															\
		> helper														\
		(																\
			boost::bind													\
			(															\
				&CLASS::mlb_##NAME,this,								\
				_1,_2,_3,_4												\
			)															\
		);																\
		return helper													\
		(																\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME,													\
			ARG4_NAME													\
		);																\
	}																	\
	RESULT CLASS::mlb_##NAME(											\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME											\
			)															\

#define MLB_CLASS_FUNC_5(RESULT,CLASS,NAME,								\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME,										\
			ARG4_TYPE,ARG4_NAME,										\
			ARG5_TYPE,ARG5_NAME											\
			)															\
	namespace															\
	{																	\
		extern xchar sz_##NAME[] = _XTEXT(#CLASS) _XTEXT("::") _XTEXT(#NAME);						\
	}																	\
	RESULT CLASS::NAME(													\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME											\
			)															\
	{																	\
		mlb_helper											\
		<																\
			sz_##NAME,													\
			RESULT														\
			(															\
				ARG1_TYPE,												\
				ARG2_TYPE,												\
				ARG3_TYPE,												\
				ARG4_TYPE,												\
				ARG5_TYPE												\
			)															\
		> helper														\
		(																\
			boost::bind													\
			(															\
				&CLASS::mlb_##NAME,this,								\
				_1,_2,_3,_4,_5											\
			)															\
		);																\
		return helper													\
		(																\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME,													\
			ARG4_NAME,													\
			ARG5_NAME													\
		);																\
	}																	\
	RESULT CLASS::mlb_##NAME(											\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME											\
			)															\

#define MLB_CLASS_FUNC_6(RESULT,CLASS,NAME,								\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME,										\
			ARG4_TYPE,ARG4_NAME,										\
			ARG5_TYPE,ARG5_NAME,										\
			ARG6_TYPE,ARG6_NAME											\
			)															\
	namespace															\
	{																	\
		extern xchar sz_##NAME[] = _XTEXT(#CLASS) _XTEXT("::") _XTEXT(#NAME);						\
	}																	\
	RESULT CLASS::NAME(													\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME											\
			)															\
	{																	\
		mlb_helper											\
		<																\
			sz_##NAME,													\
			RESULT														\
			(															\
				ARG1_TYPE,												\
				ARG2_TYPE,												\
				ARG3_TYPE,												\
				ARG4_TYPE,												\
				ARG5_TYPE,												\
				ARG6_TYPE												\
			)															\
		> helper														\
		(																\
			boost::bind													\
			(															\
				&CLASS::mlb_##NAME,this,								\
				_1,_2,_3,_4,_5,_6										\
			)															\
		);																\
		return helper													\
		(																\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME,													\
			ARG4_NAME,													\
			ARG5_NAME,													\
			ARG6_NAME													\
		);																\
	}																	\
	RESULT CLASS::mlb_##NAME(											\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME											\
			)															\

#define MLB_CLASS_FUNC_7(RESULT,CLASS,NAME,								\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME,										\
			ARG4_TYPE,ARG4_NAME,										\
			ARG5_TYPE,ARG5_NAME,										\
			ARG6_TYPE,ARG6_NAME,										\
			ARG7_TYPE,ARG7_NAME											\
			)															\
	namespace															\
	{																	\
		extern xchar sz_##NAME[] = _XTEXT(#CLASS) _XTEXT("::") _XTEXT(#NAME);						\
	}																	\
	RESULT CLASS::NAME(													\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME											\
			)															\
	{																	\
		mlb_helper											\
		<																\
			sz_##NAME,													\
			RESULT														\
			(															\
				ARG1_TYPE,												\
				ARG2_TYPE,												\
				ARG3_TYPE,												\
				ARG4_TYPE,												\
				ARG5_TYPE,												\
				ARG6_TYPE,												\
				ARG7_TYPE												\
			)															\
		> helper														\
		(																\
			boost::bind													\
			(															\
				&CLASS::mlb_##NAME,this,								\
				_1,_2,_3,_4,_5,_6,_7									\
			)															\
		);																\
		return helper													\
		(																\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME,													\
			ARG4_NAME,													\
			ARG5_NAME,													\
			ARG6_NAME,													\
			ARG7_NAME													\
		);																\
	}																	\
	RESULT CLASS::mlb_##NAME(											\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME											\
			)															\

#define MLB_CLASS_FUNC_8(RESULT,CLASS,NAME,								\
			ARG1_TYPE,ARG1_NAME,										\
			ARG2_TYPE,ARG2_NAME,										\
			ARG3_TYPE,ARG3_NAME,										\
			ARG4_TYPE,ARG4_NAME,										\
			ARG5_TYPE,ARG5_NAME,										\
			ARG6_TYPE,ARG6_NAME,										\
			ARG7_TYPE,ARG7_NAME,										\
			ARG8_TYPE,ARG8_NAME											\
			)															\
	namespace															\
	{																	\
		extern xchar sz_##NAME[] = _XTEXT(#CLASS) _XTEXT("::") _XTEXT(#NAME);						\
	}																	\
	RESULT CLASS::NAME(													\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME,										\
			ARG8_TYPE ARG8_NAME											\
			)															\
	{																	\
		mlb_helper											\
		<																\
			sz_##NAME,													\
			RESULT														\
			(															\
				ARG1_TYPE,												\
				ARG2_TYPE,												\
				ARG3_TYPE,												\
				ARG4_TYPE,												\
				ARG5_TYPE,												\
				ARG6_TYPE,												\
				ARG7_TYPE,												\
				ARG8_TYPE												\
			)															\
		> helper														\
		(																\
			boost::bind													\
			(															\
				&CLASS::mlb_##NAME,this,								\
				_1,_2,_3,_4,_5,_6,_7,_8									\
			)															\
		);																\
		return helper													\
		(																\
			ARG1_NAME,													\
			ARG2_NAME,													\
			ARG3_NAME,													\
			ARG4_NAME,													\
			ARG5_NAME,													\
			ARG6_NAME,													\
			ARG7_NAME,													\
			ARG8_NAME													\
		);																\
	}																	\
	RESULT CLASS::mlb_##NAME(											\
			ARG1_TYPE ARG1_NAME,										\
			ARG2_TYPE ARG2_NAME,										\
			ARG3_TYPE ARG3_NAME,										\
			ARG4_TYPE ARG4_NAME,										\
			ARG5_TYPE ARG5_NAME,										\
			ARG6_TYPE ARG6_NAME,										\
			ARG7_TYPE ARG7_NAME,										\
			ARG8_TYPE ARG8_NAME											\
			)															\

#endif
