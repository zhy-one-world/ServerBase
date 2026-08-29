/********************************************************************
  created: 2014/08/12
  created: 12:8:2014 11:22
  file base: lua_script
  file ext: cpp
  author: YU REN
  
  purpose: 
*********************************************************************/
#include <lua_script.h>
#include "lua-pbc.h"

namespace faith
{
	namespace lua
	{
		lua_script::lua_script()
		{
			m_lua_state = luaL_newstate();
			m_is_runing	= false;
			if (m_lua_state == NULL)
			{
				script_error(LUA_CREATE_ERROR);				
				return ;
			}

			
			memset(m_script_name,0,sizeof(m_script_name));
			m_func_map.clear();
		}

		sol2_api::sol2_interface lua_script::sol2()
		{
			return sol2_api::sol2_interface(m_lua_state);
		}

		lua_script::~lua_script(void)
		{
			exit();
		}

		bool lua_script::init(const char* script_name)
		{
			if (!m_lua_state)
				return false;

			std::string name_str = script_name;
			memcpy(m_script_name,name_str.c_str(),sizeof(m_script_name) > name_str.size() + 1 ? name_str.size() + 1 : sizeof(m_script_name));

			// 加载lua标准库
			luaL_openlibs(m_lua_state);

			//注册lua-pbc
			luaopen_protobuf_c(m_lua_state);

			// 注册类/函数到lua
			extern  int  tolua_logic_open(lua_State* tolua_S);
			tolua_logic_open(m_lua_state);

			// 执行脚本
			int ret = luaL_dofile(m_lua_state,m_script_name);
			if(ret != 0)	// 非0 表示执行不正常
			{
				exit();
				return false;
			}
			
			m_is_runing	= true;
			return	true;
		}

// 		bool lua_script::register_function(unsigned int event_type,const char* func_name)
// 		{
// 			if (!(m_is_runing && m_lua_state))
// 			{
// 				script_error(LUA_SCRIPT_STATES_IS_NULL);
// 				return false;
// 			}
// 
// 			std::pair<func_map::iterator,bool> ret = m_func_map.insert(std::make_pair(event_type,func_name));
// 			return ret.second;
// 		}
// 
// 		bool lua_script::unregister_function(unsigned int event_type)
// 		{
// 			if (!(m_is_runing && m_lua_state))
// 			{
// 				script_error(LUA_SCRIPT_STATES_IS_NULL);
// 				return false;
// 			}
// 
// 			bool ret = false;
// 			func_map::iterator iter = m_func_map.find(event_type);
// 			if(iter != m_func_map.end())
// 			{
// 				m_func_map.erase(iter);
// 				ret = true;
// 			}
// 
// 			return ret;
// 		}

		int lua_script::call_function(const char* table_name, const char* func_name, int nResults, bool use_buff, const char* cFormat, va_list vlist)
		{
			if (!(m_is_runing && m_lua_state))
			{
				return LUA_SCRIPT_STATES_IS_NULL;
			}

			// 			func_map::iterator iter = m_func_map.find(event_type);
			// 			if(iter == m_func_map.end())
			// 				return false;

			double nNumber = 0.0;
			char* cString = NULL;
			void* pPoint = NULL;

			BYTE* pByte = NULL;
			lua_CFunction CFunc;
			int i = 0;
			int nArgnum = 0;
			int nIndex = 0;

			int nSize1 = lua_gettop(m_lua_state);

			{
				if (func_name == NULL || cFormat == NULL)
					return LUA_SCPIPT_PARAM_ERROR;
				if (nullptr != table_name)
				{
					lua_getglobal(m_lua_state, table_name);		//在堆栈中加入需要调用的模块
					if (!lua_istable(m_lua_state, -1))
					{
						return LUA_SCRIPT_NOT_TABLE_ERROR;
					}
					lua_pushstring(m_lua_state, func_name);
					lua_gettable(m_lua_state, -2);

					lua_pushstring(m_lua_state, func_name);
					nArgnum = 1;
				}
				else
				{
					lua_getglobal(m_lua_state, func_name);		//在堆栈中加入需要调用的函数名
					if (!lua_isfunction(m_lua_state, -1))
					{
						script_error(LUA_SCRIPT_NOT_FUNCTION_ERROR);
						lua_pop(m_lua_state, 1);
						return 0;
					}
				}
				while (cFormat[i] != '\0' && cFormat[i] != '>')
				{
					if (cFormat[i] == '%')
						i++;
					else
					{
						lua_settop(m_lua_state, nSize1);
						return LUA_SCRIPT_COMPILE_ERROR;
					}

					switch (cFormat[i])
					{
					case 'n':								//输入的数据是double形 NUMBER，Lua来说是Double型
					{
						nNumber = va_arg(vlist, double);
						lua_pushnumber(m_lua_state, nNumber);
						nArgnum++;

					}
					break;

					case 'd':								//输入的数据为整形
					{
						nNumber = (double)(va_arg(vlist, int));
						lua_pushnumber(m_lua_state, (double)nNumber);
						nArgnum++;
					}
					break;

					case 'l':								//输入的数据为long long形
					{
						nNumber = (double)(va_arg(vlist, long long));
						double num = (double)nNumber;
						lua_pushnumber(m_lua_state, num);
						nArgnum++;
					}
					break;

					case 's':								//字符串型
					{
						cString = va_arg(vlist, char*);
						if (use_buff)
						{
							nNumber = (double)(va_arg(vlist, int));
							lua_pushlstring(m_lua_state, cString, nNumber);
						}
						else
						{
							lua_pushstring(m_lua_state, cString);
						}
						nArgnum++;
					}
					break;
					case 'N':								//NULL
					{
						lua_pushnil(m_lua_state);
						nArgnum++;
					}
					break;

					case 'f':								//输入的是CFun形，即内部函数形
					{
						CFunc = va_arg(vlist, lua_CFunction);
						lua_pushcfunction(m_lua_state, CFunc);
						nArgnum++;
					}
					break;

					case 'v':								//输入的是堆栈中Index为nIndex的数据类型
					{
						nNumber = va_arg(vlist, int);
						int nIndex1 = (int)nNumber;
						lua_pushvalue(m_lua_state, nIndex1);
						nArgnum++;
					}
					break;
					case 'b':
					{
						lua_pushboolean(m_lua_state, va_arg(vlist, bool));
						nArgnum++;
					}
					break;
					case 't':								//输入为一Table类型
					{
						//补充该函数未支持的table类型					
						//传过来的是一个拼好的数据流，目前只支持table中拥有'd'和's'格式
						//流的头部有该table的长度
						pByte = va_arg(vlist, BYTE*);
						int nReadPos = 0;
						int nLen = *pByte;
						nReadPos += sizeof(int);

						lua_newtable(m_lua_state);
						int nCount = 1;
						while (nReadPos < nLen)
						{
							switch (pByte[nReadPos])
							{
							case 'd':
							{
								++nReadPos;
								int nNumValue = *((int*)(pByte + nReadPos));
								nReadPos += sizeof(int);
								lua_pushnumber(m_lua_state, nCount++);
								lua_pushnumber(m_lua_state, nNumValue);
								lua_rawset(m_lua_state, -3);
							}
							break;
							case 's':
							{
								++nReadPos;
								char* szValue = (char*)(pByte + nReadPos);
								size_t nStrLen = strlen(szValue);
								nReadPos += nStrLen;
								lua_pushnumber(m_lua_state, nCount++);
								lua_pushstring(m_lua_state, szValue);
								lua_rawset(m_lua_state, -3);
							}
							break;
							default:
								++nReadPos;
							}
						}
						++nArgnum;
					}
					break;
					default:
					{
						lua_settop(m_lua_state, nSize1);
						return LUA_SCPIPT_PARAM_ERROR;
					}
					break;
					} //end switch

					i++;
				} // end while

			}

#if  defined(_DEBUG)
			lua_call(m_lua_state, nArgnum, nResults);

#else
			int call_res = lua_pcall(m_lua_state, nArgnum, nResults, 0);
			if (call_res != 0)
			{
				script_error(LUA_SCRIPT_EXECUTE_ERROR);
				lua_settop(m_lua_state, nSize1);
				return call_res;
			}
#endif
			int nres = -nResults;
			if (nResults > 0) i++;
			while (nResults > 0 && cFormat[i] != '\0')
			{
				if (cFormat[i] == '%')
					i++;
				else
				{
					lua_settop(m_lua_state, nSize1);
					return 0;
				}
				switch (cFormat[i])
				{
				case 'd':
				{
					if (!lua_isnumber(m_lua_state, nres))
						script_error(LUA_SCPIPT_RESULT_ERROR);
					*va_arg(vlist, int *) = (int)lua_tonumber(m_lua_state, nres);
				}
				break;
				case  'n':
				{
					if (!lua_isnumber(m_lua_state, nres))
						script_error(LUA_SCPIPT_RESULT_ERROR);
					*va_arg(vlist, double *) = lua_tonumber(m_lua_state, nres);
				}
				break;
				case  'b':
				{
					if (!lua_isboolean(m_lua_state, nres))
						script_error(LUA_SCPIPT_RESULT_ERROR);
					*va_arg(vlist, bool *) = (0 != lua_toboolean(m_lua_state, nres));
				}
				break;
				case  's':
				{
					if (!lua_isstring(m_lua_state, nres))
						script_error(LUA_SCPIPT_RESULT_ERROR);
					*va_arg(vlist, std::string*) = lua_tostring(m_lua_state, nres);
				}
				break;
				default:
					script_error(LUA_SCPIPT_RESULT_ERROR);
				}
				i++;
				nres++;
			}
			lua_settop(m_lua_state, nSize1);
			return	0;
		}

		int	lua_script::get_lua_int_variable(const xstring lua_variable_name)
		{
			if (!(m_is_runing && m_lua_state))
			{
				script_error(LUA_SCRIPT_STATES_IS_NULL);
				return -1;
			}

			lua_getglobal(m_lua_state,lua_variable_name.c_str());	
			if (!lua_isnumber(m_lua_state, -1))
			{
				lua_pop(m_lua_state, 1);
				return -1;
			}
			int tempData = (int)lua_tointeger(m_lua_state, -1);
			lua_pop(m_lua_state, 1);
			return tempData;
		}

		double lua_script::get_lua_double_variable(const xstring lua_variable_name)
		{
			if (!(m_is_runing && m_lua_state))
			{
				script_error(LUA_SCRIPT_STATES_IS_NULL);
				return -1;
			}

			lua_getglobal(m_lua_state,lua_variable_name.c_str());	
			if (!lua_isnumber(m_lua_state, -1))
			{
				lua_pop(m_lua_state, 1);
				return -1;
			}
			double tempData = (double)lua_tonumber(m_lua_state, -1);
			lua_pop(m_lua_state, 1);
			return tempData;
		}

		xstring	lua_script::get_lua_string_variable(const xstring lua_variable_name)
		{
			if (!(m_is_runing && m_lua_state))
			{
				script_error(LUA_SCRIPT_STATES_IS_NULL);
				return "";
			}

			lua_getglobal(m_lua_state,lua_variable_name.c_str());	
			if (!lua_isstring(m_lua_state, -1))
			{
				lua_pop(m_lua_state, 1);
				return "";
			}

			xstring tempData = (const xchar*)lua_tostring(m_lua_state, -1);
			lua_pop(m_lua_state, 1);
			return tempData;
		}

		bool lua_script::get_lua_bool_variable(const xstring lua_variable_name)
		{
			if (!(m_is_runing && m_lua_state))
			{
				script_error(LUA_SCRIPT_STATES_IS_NULL);
				return false;
			}

			lua_getglobal(m_lua_state,lua_variable_name.c_str());	
			if (!lua_isboolean(m_lua_state, -1))
			{
				lua_pop(m_lua_state, 1);
				return false;
			}

			bool tempData = (bool)lua_toboolean(m_lua_state, -1);
			lua_pop(m_lua_state, 1);
			return tempData;
		}

		static int map_index = 0;
		xstring lua_script::get_lua_table_item_variable(const xstring lua_table_name,int lua_item_row,const xstring lua_item_col_name)
		{
			if (!(m_is_runing && m_lua_state))
			{
				script_error(LUA_SCRIPT_STATES_IS_NULL);
				return "";
			}

			lua_getglobal(m_lua_state,lua_table_name.c_str());
			if (!lua_istable(m_lua_state, -1))
 			{
				lua_pop(m_lua_state, 1);
				return "";
			}
 
			m_table_map.clear();
			map_index = 0;
			int index = lua_gettop(m_lua_state);
			get_lua_table_item_iterative(m_lua_state,index);
			lua_pop(m_lua_state, 1);
			table_map::iterator iter = m_table_map.find(lua_item_row);
			if(iter == m_table_map.end())
				return "";

			key_value_map& value_map = iter->second;
			key_value_map::iterator iter_value = value_map.find(lua_item_col_name);
			if(iter_value == value_map.end())
				return "";

			return iter_value->second;
		}

		bool lua_script::get_lua_table_item_iterative(lua_State* lua_state,int table_index)
		{
			bool ret = true;
			std::stringstream stream;
			stream.clear();
			xstring key_str;
			xstring value_str;
			key_value_map value_map;
			value_map.clear();

			lua_pushnil(m_lua_state);
			while(lua_next(m_lua_state, table_index) != 0)
			{
				int keyType = lua_type(m_lua_state, -2);
				if(keyType == LUA_TNUMBER)
				{
					double key = (double)lua_tonumber(m_lua_state, -2);
					stream << key;
					stream >> key_str;
				}
				else if(keyType == LUA_TSTRING)
				{
					key_str = (const char*)lua_tostring(m_lua_state, -2);
				}
				else
				{
					ret = false;
					break;
				}

				int valueType = lua_type(m_lua_state, -1);
				switch(valueType)
				{
				case LUA_TNIL:
					ret = false;
					break;
				case LUA_TBOOLEAN:
					{
						bool value = (bool)lua_toboolean(m_lua_state, -1);
						stream.clear();
						stream << value;
						stream >> value_str;

						value_map.insert(std::make_pair(key_str,value_str));
					}
					break;
				case LUA_TNUMBER:
					{
						double value = (double)lua_tonumber(m_lua_state, -1);
						stream.clear();
						stream << value;
						stream >> value_str;

						value_map.insert(std::make_pair(key_str,value_str));
					}
					break;
				case LUA_TSTRING:
					{
						value_str = (const char*)lua_tostring(m_lua_state, -1);

						value_map.insert(std::make_pair(key_str,value_str));
					}
					break;
				case LUA_TTABLE:
					{
						int index = lua_gettop(m_lua_state);
						if (!get_lua_table_item_iterative(m_lua_state,index))
							ret = false;
					}
					break;
				default:
					ret = false;
					break;
				}

				lua_pop(m_lua_state, 1);
			}

			m_table_map.insert(std::make_pair(++map_index,value_map));
			return ret;
		}

		void lua_script::exit()
		{
			if (!m_lua_state)	
				return;

			lua_close(m_lua_state);
			m_lua_state = NULL;
			m_is_runing = false;
			m_func_map.clear();
		}

		void lua_script::script_error(int Error)
		{
			if (Error == 0)
			{
				return;
			}
			if (!m_lua_state)
				return;

			char lszErrMsg[200];
			_snprintf(lszErrMsg, 200, "ScriptError %d. (%s) %s \n", Error, m_script_name, lua_tostring(m_lua_state,-1));
			lszErrMsg[199] = 0;
			std::cout << "lua_script::script_error :" << lszErrMsg << std::endl;
			return;
		}

		void lua_script::script_error(int Error1 ,int Error2)
		{
			if (!m_lua_state)
				return;

			char lszErrMsg[200];
			_snprintf(lszErrMsg, 200, "ScriptError %d:[%d] (%s) %s \n", Error1, Error2, m_script_name, lua_tostring(m_lua_state,-1));
			lszErrMsg[199] = 0;
			std::cout << "lua_script::script_error :" << lszErrMsg << std::endl;
			return;
		}

		bool lua_script::stop(void)
		{
			if (!m_is_runing)		return true;
			if (!m_lua_state)		return false;
			m_is_runing =  false;
			return true;
		}

		bool lua_script::resume(void)
		{
			if ((!m_is_runing) && (m_lua_state))
			{
				m_is_runing = false;
				return true;
			}
			return false;
		}

// 		int	lua_script::get_event_id(const char* event_type)
// 		{
// 			lua_getglobal(m_lua_state, event_type);
// 			if (!lua_isnumber(m_lua_state, -1))
// 			{
// 				lua_pop(m_lua_state, 1);
// 				return 0;
// 			}
// 
// 			return (int)lua_tointeger(m_lua_state, -1);
// 		}
	}
}

