#if !defined(BOOST_PP_IS_ITERATING)

#ifndef _MLB_GETTER_H_
#define _MLB_GETTER_H_

#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/function.hpp>
#include "xchar.hpp"

namespace faith
{

	/*
		help the moonlightbox function calling.
		1) in logging mode , calling the relative functor,and log the calling.
		2) in recurence mode , fetch the result from the log file.
		3) in normal mode, only calling the relative functor.
	*/
	template <const xchar* Name, class sig>
	class mlb_helper;
}

#define BOOST_PP_ITERATION_PARAMS_1 (3, (0, 9, "mlb_helper.hpp"))
#include BOOST_PP_ITERATE()

#endif //end of _MLB_GETTER_H_

#else

#define N BOOST_PP_ITERATION()
#define ARG(z,n,text) BOOST_PP_CAT(T,n) BOOST_PP_CAT(arg,n)

namespace faith
{

	template <const xchar* Name, class R BOOST_PP_ENUM_TRAILING_PARAMS(N, class T) >
	class mlb_helper<Name, R(BOOST_PP_ENUM_PARAMS(N, T)) >
	{
	public:
		typedef boost::function<R(BOOST_PP_ENUM_PARAMS(N, T))> Handler;
		mlb_helper(Handler handler) :
			m_handler(handler)
		{
		}

		R operator () (BOOST_PP_ENUM(N, ARG, ""))
		{
			R ret;
			// 				if(faith::common::mlb_mode::getInstance().in_recurrence_mode())
			// 				{
			// 					faith::common::MlbParam result;
			// 					faith::common::mlb_mode::getInstance().get_recurer().fetch_calling_result(m_name,result BOOST_PP_ENUM_TRAILING_PARAMS(N,arg));
			// 					result.cast(ret);
			// 					return ret;
			// 				}
			ret = m_handler(BOOST_PP_ENUM_PARAMS(N, arg));
			// 				if(faith::common::mlb_mode::getInstance().in_logging_mode())
			// 				{
			// 					faith::common::mlb_mode::getInstance().get_logger().store_calling(m_name,ret BOOST_PP_ENUM_TRAILING_PARAMS(N,arg));
			// 				}
			return ret;
		}
	private:
		Handler			m_handler;
		static xstring	m_name;
	};

	template <const xchar* Name, class R BOOST_PP_ENUM_TRAILING_PARAMS(N, class T) >
	xstring	mlb_helper<Name, R(BOOST_PP_ENUM_PARAMS(N, T)) >::m_name = Name;

	template <const xchar* Name BOOST_PP_ENUM_TRAILING_PARAMS(N, class T) >
	class mlb_helper<Name, void(BOOST_PP_ENUM_PARAMS(N, T)) >
	{
	public:
		typedef boost::function<void(BOOST_PP_ENUM_PARAMS(N, T))> Handler;
		mlb_helper(Handler handler) :
			m_handler(handler)
		{
		}

		void operator () (BOOST_PP_ENUM(N, ARG, ""))
		{
			// 				if(faith::common::mlb_mode::getInstance().in_recurrence_mode())
			// 				{
			// 					faith::common::MlbParam result;
			// 					faith::common::mlb_mode::getInstance().get_recurer().fetch_calling_result(m_name,result BOOST_PP_ENUM_TRAILING_PARAMS(N,arg));
			// 					return;
			// 				}
			m_handler(BOOST_PP_ENUM_PARAMS(N, arg));
			// 				if(faith::common::mlb_mode::getInstance().in_logging_mode())
			// 				{
			// 					faith::common::mlb_mode::getInstance().get_logger().store_calling(m_name,0 BOOST_PP_ENUM_TRAILING_PARAMS(N,arg));
			// 				}
		}
	private:
		Handler			m_handler;
		static xstring	m_name;
	};

	template <const xchar* Name BOOST_PP_ENUM_TRAILING_PARAMS(N, class T) >
	xstring mlb_helper<Name, void(BOOST_PP_ENUM_PARAMS(N, T)) >::m_name = Name;
}

#endif
