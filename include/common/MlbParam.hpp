//	Octopus MMORPG Platform
//	Copyright(c) by NineYou Information technology(Shanghai) Co., Ltd.
//	created by Zhang Yongbo, 2008

#ifndef __0MP_MLB_PARAM_HEADER_FILE__
#define __0MP_MLB_PARAM_HEADER_FILE__

#pragma warning( disable : 4800 )
#pragma warning( disable : 4819 )
#pragma warning( disable : 4996 )

#include <string>		// for std::string
#include <typeinfo>		// for std::bad_typeid

#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/variant.hpp>
#include "string_buffer.hpp"

#define MLB_TYPE_BOOL					(bool)
#define MLB_TYPE_CHAR					(char)(wchar_t)
#define MLB_TYPE_basic_integer			(char)(short)(int)(long)(long long)
#define MLB_TYPE_int_pair(r,data,t)		(signed t)(unsigned t)
#define MLB_TYPE_INTEGER				\
	BOOST_PP_SEQ_FOR_EACH(MLB_TYPE_int_pair, ~, MLB_TYPE_basic_integer)
#define MLB_TYPE_FLOAT					(float)(double)(long double)

#define MLB_TYPE_STRING					(faith::common::string_buffer)(faith::common::wstring_buffer)
#define MLB_TYPE_C_STR					(const char *)(const wchar_t *)(std::string)(std::wstring)

#define MLB_TYPES_ARITHMETIC			\
	MLB_TYPE_BOOL						\
	MLB_TYPE_CHAR						\
	MLB_TYPE_INTEGER					\
	MLB_TYPE_FLOAT

#define MLB_TYPES_CORE					\
	MLB_TYPES_ARITHMETIC				\
	MLB_TYPE_STRING

#define OMP_PARAM_DECL_UNION_BUFFER(z, n, text) char buf ## n[sizeof( BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) )];
#define OMP_PARAM_CASE_ASSIGN(z, n, text)											\
	case n:																			\
		memset(&m_storage, 0, sizeof(m_storage));									\
		*reinterpret_cast<BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) *>(&m_storage)=*reinterpret_cast<const BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) *>(&o.m_storage);	\
		break;
#define OMP_PARAM_CASE_COMPARE(z, n, text)											\
	case n:																			\
		return *reinterpret_cast<const BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) *>(&m_storage)==*reinterpret_cast<const BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) *>(&o.m_storage);
#define OMP_PARAM_CASE_SERIALIZATION(z, n, text)									\
	case n:																			\
		ar & *reinterpret_cast<BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) *>(v.storage());	\
		break;
#define OMP_PARAM_CONSTRUCTOR(z, n, text)											\
	inline MlbParam(const BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) & o)					\
	{																				\
		m_which = n;																\
		memset(&m_storage, 0, sizeof(m_storage));									\
		*reinterpret_cast<BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) *>(&m_storage) = o;	\
	}

#define OMP_PARAM_ASSIGNMENT(z, n, text)											\
	inline MlbParam& operator=(const BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) & o)		\
	{																				\
		m_which = n;																\
		memset(&m_storage, 0, sizeof(m_storage));									\
		*reinterpret_cast<BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) *>(&m_storage) = o;	\
		return *this;																\
	}
#define OMP_PARAM_CAST(z, n, text)													\
	inline bool cast(BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) & d) const					\
	{																				\
		if( m_which != mlb_detail::which_traits<BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE)>::type)	\
		{																			\
			int int_min = BOOST_PP_SEQ_SIZE(MLB_TYPE_BOOL);\
			int int_max = BOOST_PP_SEQ_SIZE(MLB_TYPE_BOOL) + BOOST_PP_SEQ_SIZE(MLB_TYPE_CHAR) + BOOST_PP_SEQ_SIZE(MLB_TYPE_INTEGER);	\
			if(int_min <= m_which < int_max )	\
			{																		\
				d = *reinterpret_cast<const BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) *>(&m_storage);	\
				return true;														\
			}																		\
			return false;															\
		}																			\
		else																		\
		{																			\
			d = *reinterpret_cast<const BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) *>(&m_storage);	\
			return true;															\
		}																			\
	}

#define OMP_PARAM_WHICH_TRAINTS(z, n, t)						\
	template <>													\
	class which_traits<BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE)>		\
	{															\
	public:														\
		static const int type = n;								\
	};															\

#define OMP_PARAM_CASE_TYPEINFO(z, n, text)											\
	case n:																			\
		return typeid( BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) );

#define OMP_PARAM_APPLY_VISITOR(z, n, text)			\
	case n:											\
		{											\
		BOOST_PP_SEQ_ELEM(n,MLB_TYPES_CORE) value;	\
		cast(value);						\
		return visitor(value);					\
		}

namespace faith
{
	namespace common
	{
		namespace mlb_detail
		{
			template <class T>
			class which_traits
			{
			public:
				static const int type = -1;
			};

			BOOST_PP_REPEAT( BOOST_PP_SEQ_SIZE(MLB_TYPES_CORE), OMP_PARAM_WHICH_TRAINTS, NA )
		}
	}
}

namespace faith {
	namespace common {

		class MlbParam
		{
		public:

			~MlbParam()
			{
				if(m_which == mlb_detail::which_traits<string_buffer>::type)
				{
					reinterpret_cast<string_buffer *>(&m_storage)->~string_buffer();
				}
				else if(m_which == mlb_detail::which_traits<wstring_buffer>::type)
				{
					reinterpret_cast<wstring_buffer *>(&m_storage)->~wstring_buffer();
				}
			}

			// construct
			inline MlbParam()
				:m_which(0)
			{
				memset(&m_storage, 0, sizeof(m_storage));
			}
			inline MlbParam(const MlbParam &o)
			{
				switch(o.m_which)
				{
					BOOST_PP_REPEAT( BOOST_PP_SEQ_SIZE(MLB_TYPES_CORE), OMP_PARAM_CASE_ASSIGN, NA )
				default:
					throw std::bad_typeid("bad_typeid in MlbParam(const MlbParam &o)");
				}
				m_which = o.m_which;
			}
			BOOST_PP_REPEAT( BOOST_PP_SEQ_SIZE(MLB_TYPES_CORE),OMP_PARAM_CONSTRUCTOR , NA )

			inline MlbParam(const char * const & o)
			{
				m_which = mlb_detail::which_traits<common::string_buffer>::type;
				memset(&m_storage, 0, sizeof(m_storage));
				*reinterpret_cast<common::string_buffer *>(&m_storage)=common::string_buffer(o);
			}
			inline MlbParam(const std::string & o)
			{
				m_which = mlb_detail::which_traits<common::string_buffer>::type;
				memset(&m_storage, 0, sizeof(m_storage));
				*reinterpret_cast<common::string_buffer *>(&m_storage)=common::string_buffer(o);
			}
			inline MlbParam(const wchar_t * const & o)
			{
				m_which = mlb_detail::which_traits<common::wstring_buffer>::type;
				memset(&m_storage, 0, sizeof(m_storage));
				*reinterpret_cast<common::wstring_buffer *>(&m_storage)=common::wstring_buffer(o);
			}
			inline MlbParam(const std::wstring & o)
			{
				m_which = mlb_detail::which_traits<common::wstring_buffer>::type;
				memset(&m_storage, 0, sizeof(m_storage));
				*reinterpret_cast<common::wstring_buffer *>(&m_storage)=common::wstring_buffer(o);
			}

			//assignment operator
			inline MlbParam& operator=(const MlbParam & o)
			{
				if(this!=&o)
				{
					switch(o.m_which)
					{
						BOOST_PP_REPEAT( BOOST_PP_SEQ_SIZE(MLB_TYPES_CORE), OMP_PARAM_CASE_ASSIGN, NA )
					default:
						throw std::bad_typeid("bad_typeid in MlbParam(const MlbParam &o)");
					}
					m_which = o.m_which;
				}
				return *this;
			}
			inline MlbParam& operator=(const char * const & o)
			{
				m_which = mlb_detail::which_traits<common::string_buffer>::type;
				*reinterpret_cast<common::string_buffer *>(&m_storage)=common::string_buffer(o);
				return *this;
			}
			inline MlbParam& operator=(const std::string & o)
			{
				m_which = mlb_detail::which_traits<common::string_buffer>::type;
				*reinterpret_cast<common::string_buffer *>(&m_storage)=common::string_buffer(o);
				return *this;
			}
			inline MlbParam& operator=(const wchar_t * const & o)
			{
				m_which = mlb_detail::which_traits<common::wstring_buffer>::type;
				*reinterpret_cast<common::wstring_buffer *>(&m_storage)=common::wstring_buffer(o);
				return *this;
			}
			inline MlbParam& operator=(const std::wstring & o)
			{
				m_which = mlb_detail::which_traits<common::wstring_buffer>::type;
				*reinterpret_cast<common::wstring_buffer *>(&m_storage)=common::wstring_buffer(o);
				return *this;
			}
			BOOST_PP_REPEAT( BOOST_PP_SEQ_SIZE(MLB_TYPES_CORE),OMP_PARAM_ASSIGNMENT , NA )

			// cast
			inline bool cast(const char * & d) const
			{
				if( m_which != mlb_detail::which_traits<common::string_buffer>::type)
				{
					return false;
				}
				else
				{
					const common::string_buffer * str = reinterpret_cast<const common::string_buffer *>(&m_storage);
					d = str->c_str();
					return true;
				}
			}
			inline bool cast(std::string & d) const
			{
				if( m_which != mlb_detail::which_traits<common::string_buffer>::type)
				{
					return false;
				}
				else
				{
					const common::string_buffer * str = reinterpret_cast<const common::string_buffer *>(&m_storage);
					d.assign(str->c_str(),str->length());
					return true;
				}
			}
			inline bool cast(const wchar_t * & d) const
			{
				if( m_which != mlb_detail::which_traits<common::wstring_buffer>::type)
				{
					return false;
				}
				else
				{
					const common::wstring_buffer * str = reinterpret_cast<const common::wstring_buffer *>(&m_storage);
					d = str->c_str();
					return true;
				}
			}
			inline bool cast(std::wstring & d) const
			{
				if( m_which != mlb_detail::which_traits<common::wstring_buffer>::type)
				{
					return false;
				}
				else
				{
					const common::wstring_buffer * str = reinterpret_cast<const common::wstring_buffer *>(&m_storage);
					d.assign(str->c_str(),str->length());
					return true;
				}
			}

			#pragma warning( push ) 
			#pragma warning( disable : 4804 )
			BOOST_PP_REPEAT( BOOST_PP_SEQ_SIZE(MLB_TYPES_CORE),OMP_PARAM_CAST , NA )
			#pragma warning( pop )
			
			//compare operator
			inline bool operator == (const MlbParam &o) const
			{
				if(m_which==o.m_which)
				{
					switch(m_which)
					{
						BOOST_PP_REPEAT( BOOST_PP_SEQ_SIZE(MLB_TYPES_CORE),OMP_PARAM_CASE_COMPARE , NA )
					default:
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			inline bool operator != (const MlbParam &o) const
			{
				return !(*this==o);
			}

			//允许直接修改内部值，为序列化准备
			inline char & which()
			{
				return m_which;
			}
			inline void * storage()
			{
				return &m_storage;
			}
			
			const std::type_info & type() const
			{
				switch(m_which)
				{
					BOOST_PP_REPEAT( BOOST_PP_SEQ_SIZE(MLB_TYPES_CORE), OMP_PARAM_CASE_TYPEINFO, NA )
				default:
					throw std::bad_typeid("bad_typeid in MlbParam(const MlbParam &o)");
				}
			}

			template <typename Visitor>
			inline  BOOST_VARIANT_AUX_GENERIC_RESULT_TYPE(typename Visitor::result_type)
				apply_visitor(Visitor& visitor) const
			{
				switch(m_which)
				{
					BOOST_PP_REPEAT( BOOST_PP_SEQ_SIZE(MLB_TYPES_CORE), OMP_PARAM_APPLY_VISITOR, NA )
				default:
					throw std::bad_typeid("bad_typeid in RpcParam(const RpcParam &o)");
				}
			}

		protected:
			char				m_which;
			union
			{
				BOOST_PP_REPEAT( BOOST_PP_SEQ_SIZE(MLB_TYPES_CORE), OMP_PARAM_DECL_UNION_BUFFER, NA )
			}					m_storage;
		};
	}
}

//support for serialization
#include "simple_binary_iarchive.hpp"
#include "simple_binary_oarchive.hpp"
//support for serialization
namespace boost
{
	namespace serialization
	{
		template<class Archive>
		inline void serialize(Archive & ar, faith::common::MlbParam & v, const unsigned int version)
		{
			char & which = v.which();
			ar & v.which();
			switch(which)
			{
				BOOST_PP_REPEAT( BOOST_PP_SEQ_SIZE(MLB_TYPES_CORE), OMP_PARAM_CASE_SERIALIZATION, NA )
			default:
				throw std::bad_typeid("bad_typeid in inline void serialize(Archive & ar, MlbParam & v, const unsigned int version)");
			}
		}
	}
}

#endif
