#include <boost/cstdint.hpp>
#include <sstream>
#include "mlb.hpp"
#include "iserializer.hpp"
#include "oserializer.hpp"

namespace faith
{
//		namespace program_options
//		{
//			variables_map::variables_map()
//			{
//			}
//			void variables_map::serialize()
//			{
//			}
//			#define SAVE_VALUE(TYPE,ENUM_NAME)										\
//			if(val.type()==typeid(TYPE))											\
//			{																		\
//				type = static_cast<boost::uint8_t>(OVT_##ENUM_NAME);	\
//				const TYPE & value = boost::any_cast<const TYPE &>(val);			\
//				oserializer::getInstance() << type << value;						\
//			}
//			static void save_any(const boost::any & val)
//			{
//				boost::uint8_t type;
//				SAVE_VALUE(bool,BOOL)
//				else SAVE_VALUE(char,CHAR)
//				else SAVE_VALUE(signed char,S_CHAR)
//				else SAVE_VALUE(unsigned char,U_CHAR)
//				else SAVE_VALUE(signed short,S_SHORT)
//				else SAVE_VALUE(unsigned short,U_SHORT)
//				else SAVE_VALUE(signed int,S_INT)
//				else SAVE_VALUE(unsigned int,U_INT)
//				else SAVE_VALUE(signed long,S_LONG)
//				else SAVE_VALUE(unsigned long,U_LONG)
//				else SAVE_VALUE(signed long long,S_LONGLONG)
//				else SAVE_VALUE(unsigned long long,U_LONGLONG)
//				else SAVE_VALUE(float,FLOAT)
//				else SAVE_VALUE(double,DOUBLE)
//				else SAVE_VALUE(long double,LONGDOUBLE)
//				else SAVE_VALUE(std::string,STRING)
//				else SAVE_VALUE(std::wstring,WSTRING)
//				else
//				{
//// 					xostringstream err;
//// 					err << _XTEXT("common::program_options::variables_map::save_any not support type:") << val.type().name();
//// 					throw_mlb_exception(err.str().c_str());
//				}
//			}
//			#define LOAD_VALUE(TYPE,ENUM_NAME)				\
//			case OVT_##ENUM_NAME:							\
//			{												\
//				TYPE value;									\
//				iserializer::getInstance() >> value;		\
//				val = value;								\
//				break;										\
//			}
//			static void load_any(boost::any & val)
//			{
//				boost::uint8_t type;
//				iserializer::getInstance() >> type;
//				switch(type)
//				{
//					LOAD_VALUE(bool,BOOL)
//					LOAD_VALUE(char,CHAR)
//					LOAD_VALUE(signed char,S_CHAR)
//					LOAD_VALUE(unsigned char,U_CHAR)
//					LOAD_VALUE(signed short,S_SHORT)
//					LOAD_VALUE(unsigned short,U_SHORT)
//					LOAD_VALUE(signed int,S_INT)
//					LOAD_VALUE(unsigned int,U_INT)
//					LOAD_VALUE(signed long,S_LONG)
//					LOAD_VALUE(unsigned long,U_LONG)
//					LOAD_VALUE(signed long long,S_LONGLONG)
//					LOAD_VALUE(unsigned long long,U_LONGLONG)
//					LOAD_VALUE(float,FLOAT)
//					LOAD_VALUE(double,DOUBLE)
//					LOAD_VALUE(long double,LONGDOUBLE)
//					LOAD_VALUE(std::string,STRING)
//					LOAD_VALUE(std::wstring,WSTRING)
//					default:
//					{
//// 						xostringstream err;
//// 						err << _XTEXT("common::program_options::variables_map::load_any unknown type:") << type;
//// 						throw_mlb_exception(err.str().c_str());
//					}
//				}
//			}
//			MLB_CLASS_FUNC_0(xstring,variables_map,get_data)
//			{
//				typedef std::map<std::string,boost::program_options::variable_value> VAR_MAP;
//				xstring result;
//				oserializer::getInstance().reset();
//				VAR_MAP & var_map = static_cast< VAR_MAP & >(*this);
//				boost::uint32_t size = var_map.size();
//				oserializer::getInstance() << size;
//				for(VAR_MAP::const_iterator it = var_map.begin();it!=var_map.end();++it)
//				{
//					oserializer::getInstance() << it->first;
//					save_any(it->second.value());
//				}
//				const xchar * buffer;
//				std::size_t buffer_len;
//				oserializer::getInstance().get_data(buffer,buffer_len);
//				result.assign(buffer,buffer_len);
//				return result;
//			}
//			void variables_map::reset(const xstring &data)
//			{
//				typedef std::map<std::string,boost::program_options::variable_value> VAR_MAP;
//				VAR_MAP & var_map = static_cast< VAR_MAP & >(*this);
//				var_map.clear();
//				iserializer::getInstance().set_data(data.c_str(),data.length());
//				boost::uint32_t size;
//				iserializer::getInstance() >> size;
//				for(boost::uint32_t i = 0; i < size ; ++ i)
//				{
//					std::string name;
//					boost::program_options::variable_value value;
//					iserializer::getInstance() >> name;
//					load_any(value.value());
//					var_map.insert(std::make_pair(name,value));
//				}
//			}
//		}
}
