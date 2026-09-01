/********************************************************************
  created: 2014/08/12
  created: 12:8:2014 11:22
  file base: lua_script
  file ext: h
  author: zhy
  
  purpose: 
*********************************************************************/
#ifndef _LUA_SCRIPT_H_
#define _LUA_SCRIPT_H_

#include <sstream>
#include <windows.h>
#include <list>
#include <map>
#include "xchar.hpp"
#include "sol2_interface.hpp"


extern "C"
{
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
};



namespace faith
{
	namespace lua
	{
		#define LUA_CREATE_ERROR 1
		#define LUA_SCRIPT_LEN_ERROR 2
		#define LUA_SCRIPT_COMPILE_ERROR 3
		#define LUA_SCRIPT_EXECUTE_ERROR 4
		#define LUA_SCRIPT_NOT_NUMBER_ERROR 10
		#define LUA_SCRIPT_NOT_STRING_ERROR 11
		#define LUA_SCRIPT_NOT_TABLE_ERROR 12
		#define LUA_SCRIPT_NOT_FUNCTION_ERROR 13
		#define LUA_SCRIPT_STATES_IS_NULL 20
		#define LUA_SCRIPE_FILE_NAME 128
        #define LUA_SCPIPT_PARAM_ERROR   21
		#define LUA_SCPIPT_RESULT_ERROR  22 

		//---------------------------------------------------------------------------
		class  lua_script
		{
			typedef  void  (*func_handler)(unsigned int event_type,std::list<std::string>& params);
			typedef	 std::map<unsigned int,std::string>		func_map;

		public:
			lua_script();
			virtual ~lua_script();

		public:
			//---------------------------------------------------------------------------
			// 函数:	lua_script::Init
			// 功能:	初始化脚本对象，注册系统标准函数库,执行脚本
			// 返回:	bool 
			//---------------------------------------------------------------------------
			bool							init(const char* script_name);		
			//---------------------------------------------------------------------------
			// 函数:	lua_script::ReleaseScript
			// 功能:	释放该脚本资源。
			// 返回:	bool 
			//---------------------------------------------------------------------------
			void							exit();
			//---------------------------------------------------------------------------
			// 函数:	lua_script::register_function
			// 功能:	注册Lua脚本内的函数
			// 返回:	bool 
			//---------------------------------------------------------------------------
			//bool							register_function(unsigned int event_type,const char* func_name);
			//---------------------------------------------------------------------------
			// 函数:	lua_script::unregister_function
			// 功能:	注销Lua脚本内的函数(有形参函数)
			// 返回:	bool 
			//---------------------------------------------------------------------------
			//bool							unregister_function(unsigned int event_type);
			//---------------------------------------------------------------------------
			// 函数:	lua_script::CallFunction
			// 功能:	调用Lua脚本内的函数
			// 参数:	int nResults
			// 参数:	LPSTR cFormat  调用时所传参数的类型 
			//				n:数字型(double) d:整形(int) s:字符串型 f:C函数型  n:Nil v:Value p:Point
			//			    v:为Lua支持的，参数为整形的数index，指明将index所指堆栈的变量作为该函数的调用参数
			//	注意：由于该函数有不定参数…,对于数字，系统并不确定数是以double还是以int存在，两种保存形式是不同的。因此需要注意当传入的数是整形时，格式符应用d
			//  而不能用n,或者强行改变为double形。否则会出现计算的错误。
			// 返回:	bool 
			//---------------------------------------------------------------------------
			int							call_function(const char* table_name, const char* func_name, int nResults, bool use_buff, const char* cFormat, va_list vlist);
			//---------------------------------------------------------------------------
			// 函数:	lua_script::StopScript
			// 功能:	中止脚本
			// 参数:	void
			// 返回:	bool 
			//---------------------------------------------------------------------------
			bool							stop();
			//---------------------------------------------------------------------------
			// 函数:	lua_script::ResumeScript
			// 功能:	恢复已中止的脚本
			// 参数:	void
			// 返回:	bool 
			//---------------------------------------------------------------------------
			bool							resume();

			// 返回绑定到当前 lua_State 的 sol2 接口。
			sol2_api::sol2_interface	sol2();
			//---------------------------------------------------------------------------
			// 函数:	lua_script::ModifyTable
			// 功能:	将指定名称的LuaTable置堆栈顶端，并返回顶端Index
			// 参数:	LPSTR szTableName
			// 返回:	DWORD 若Lua中不存在该Table则返回-1
			//---------------------------------------------------------------------------
			//int								get_event_id(const char* event_type); 
			//---------------------------------------------------------------------------
			// 函数:	
			// 功能:	根据类型获取lua中的全局普简单类型变量
			//---------------------------------------------------------------------------
			int								get_lua_int_variable(const xstring lua_variable_name);
			double							get_lua_double_variable(const xstring lua_variable_name);
			xstring							get_lua_string_variable(const xstring lua_variable_name);
			bool							get_lua_bool_variable(const xstring lua_variable_name);
			/*
			 * 函数:	
			 * 功能:	根据类型获取lua中的全局复合类型变量
			 * 形如 test_table = 
							{
							[1] = 0,
							[2] = {a = 1,b = 2},
							[3] = {c = 3}
							}
					两层嵌套的table结构，第一层嵌套使用下标结构，第二层嵌套使用key-value结构，value值可以为lua中任意有效类型值
			 * 参数 const xstring lua_table_name  			全局table变量
			 * 参数 int lua_item_row							table变量中嵌套子table的位置，如[2] 为 row = 2
			 * 参数 const xstring lua_item_col_name			table变量中嵌套子table的中的item key值
			 * 返回值统一为xstring类型，需在逻辑中进行实际类型转换
			*/
			xstring							get_lua_table_item_variable(const xstring lua_table_name,int lua_item_row,const xstring lua_item_col_name);

		public:
			void							script_error(int error);
			void							script_error(int , int);
		private:
			bool							m_is_runing;													//是否该脚本有效
			char							m_script_name[LUA_SCRIPE_FILE_NAME];
			lua_State*						m_lua_state;
			func_map						m_func_map;

		};
	}
}

#endif 
