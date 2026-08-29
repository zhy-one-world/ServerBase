/*
 * Lua runtime wrapper.
 *
 * The generated tolua binding still receives the native state because it is
 * the ABI used by the generated files. All runtime operations owned by this
 * class use sol2, so stack ownership and error handling stay in one place.
 */
#include <lua_script.h>
#include "lua-pbc.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace
{
	std::string value_to_string(const sol::object& value)
	{
		if (!value.valid() || value.is<sol::lua_nil_t>())
			return {};

		if (value.is<bool>())
			return value.as<bool>() ? "1" : "0";
		if (value.is<double>())
		{
			std::ostringstream stream;
			stream << value.as<double>();
			return stream.str();
		}
		if (value.is<std::string>())
			return value.as<std::string>();

		return {};
	}
}

namespace faith
{
	namespace lua
	{
		lua_script::lua_script()
			: m_is_runing(false)
			, m_lua_state(luaL_newstate())
		{
			std::memset(m_script_name, 0, sizeof(m_script_name));
			m_func_map.clear();

			if (m_lua_state == nullptr)
				script_error(LUA_CREATE_ERROR);
		}

		sol2_api::sol2_interface lua_script::sol2()
		{
			return sol2_api::sol2_interface(m_lua_state);
		}

		lua_script::~lua_script()
		{
			exit();
		}

		bool lua_script::init(const char* script_name)
		{
			if (m_lua_state == nullptr || script_name == nullptr || *script_name == '\0')
				return false;

			const std::string name(script_name);
			std::memcpy(
				m_script_name,
				name.c_str(),
				(name.size() + 1 < sizeof(m_script_name))
					? name.size() + 1
					: sizeof(m_script_name) - 1);
			m_script_name[sizeof(m_script_name) - 1] = '\0';

			sol2_api::sol2_interface api(m_lua_state);
			api.open_libraries(
				sol::lib::base,
				sol::lib::package,
				sol::lib::coroutine,
				sol::lib::string,
				sol::lib::table,
				sol::lib::math,
				sol::lib::io,
				sol::lib::os,
				sol::lib::debug,
				sol::lib::utf8);

			// pbc is a C module; sol2 still owns module loading and stack cleanup.
			api.state().require("protobuf.c", luaopen_protobuf_c, false);

			// Generated tolua code is kept as the compatibility boundary for
			// existing C++ classes and functions.
			extern int tolua_logic_open(lua_State* tolua_S);
			tolua_logic_open(m_lua_state);

			std::string error_message;
			if (!api.run_file(name, &error_message))
			{
				if (!error_message.empty())
					std::cerr << "lua_script::init: " << error_message << std::endl;
				exit();
				return false;
			}

			m_is_runing = true;
			return true;
		}

		int lua_script::call_function(
			const char* table_name,
			const char* func_name,
			int nResults,
			bool use_buff,
			const char* cFormat,
			va_list vlist)
		{
			if (!(m_is_runing && m_lua_state))
				return LUA_SCRIPT_STATES_IS_NULL;
			if (func_name == nullptr || cFormat == nullptr || nResults < 0)
				return LUA_SCPIPT_PARAM_ERROR;

			sol::state_view state(m_lua_state);
			std::vector<sol::object> args;
			bool valid_format = true;

			for (const char* format = cFormat; *format != '\0' && *format != '>'; ++format)
			{
				if (*format != '%')
				{
					valid_format = false;
					break;
				}

				++format;
				switch (*format)
				{
				case 'n':
					args.emplace_back(sol::make_object(m_lua_state, va_arg(vlist, double)));
					break;
				case 'd':
					args.emplace_back(sol::make_object(
						m_lua_state, static_cast<double>(va_arg(vlist, int))));
					break;
				case 'l':
					args.emplace_back(sol::make_object(
						m_lua_state, static_cast<double>(va_arg(vlist, long long))));
					break;
				case 's':
				{
					const char* value = va_arg(vlist, char*);
					if (value == nullptr)
					{
						args.emplace_back(sol::make_object(m_lua_state, sol::nil));
					}
					else if (use_buff)
					{
						const size_t length = static_cast<size_t>(va_arg(vlist, int));
						args.emplace_back(sol::make_object(
							m_lua_state, std::string(value, length)));
					}
					else
					{
						args.emplace_back(sol::make_object(m_lua_state, value));
					}
					break;
				}
				case 'N':
					args.emplace_back(sol::make_object(m_lua_state, sol::nil));
					break;
				case 'b':
					args.emplace_back(sol::make_object(
						m_lua_state, va_arg(vlist, int) != 0));
					break;
				case 't':
				{
					// Keep the legacy wire format, but create the Lua table
					// through sol2 instead of manipulating the Lua stack.
					const BYTE* bytes = va_arg(vlist, BYTE*);
					if (bytes == nullptr)
					{
						valid_format = false;
						break;
					}

					const int length = *reinterpret_cast<const int*>(bytes);
					if (length < static_cast<int>(sizeof(int)))
					{
						valid_format = false;
						break;
					}

					sol::table table = state.create_table();
					int read_position = static_cast<int>(sizeof(int));
					int table_index = 1;
					while (read_position < length)
					{
						switch (bytes[read_position++])
						{
						case 'd':
							if (read_position + static_cast<int>(sizeof(int)) > length)
							{
								valid_format = false;
								break;
							}
							table[table_index++] =
								*reinterpret_cast<const int*>(bytes + read_position);
							read_position += sizeof(int);
							break;
						case 's':
						{
							const char* value =
								reinterpret_cast<const char*>(bytes + read_position);
							const size_t remaining =
								static_cast<size_t>(length - read_position);
							const void* terminator =
								std::memchr(value, '\0', remaining);
							if (terminator == nullptr)
							{
								valid_format = false;
								break;
							}

							const size_t value_length =
								static_cast<const char*>(terminator) - value;
							table[table_index++] =
								std::string(value, value_length);
							read_position += static_cast<int>(value_length + 1);
							break;
						}
						default:
							valid_format = false;
							break;
						}

						if (!valid_format)
							break;
					}
					args.emplace_back(std::move(table));
					break;
				}
				case 'f':
					args.emplace_back(sol::make_object(
						m_lua_state, va_arg(vlist, lua_CFunction)));
					break;
				case 'v':
					// This API has no caller stack after moving to sol2, so a
					// stack index cannot be resolved safely.
					valid_format = false;
					break;
				default:
					valid_format = false;
					break;
				}

				if (!valid_format)
					break;
			}

			if (!valid_format)
				return LUA_SCPIPT_PARAM_ERROR;

			sol::protected_function function;
			sol::table table;
			if (table_name != nullptr)
			{
				sol::object table_object = state[table_name];
				if (!table_object.is<sol::table>())
					return LUA_SCRIPT_NOT_TABLE_ERROR;

				table = table_object.as<sol::table>();
				function = table[func_name];
			}
			else
			{
				function = state[func_name];
			}

			if (!function.valid())
				return LUA_SCRIPT_NOT_FUNCTION_ERROR;

			sol::protected_function_result result =
				table_name != nullptr
					? function(table, sol::as_args(args))
					: function(sol::as_args(args));
			if (!result.valid())
			{
				sol::error error = result;
				std::cerr << "lua_script::call_function: "
					<< error.what() << std::endl;
				return LUA_SCRIPT_EXECUTE_ERROR;
			}

			const char* output = cFormat;
			while (*output != '\0' && *output != '>')
				++output;
			if (*output != '>')
				return 0;

			++output;
			for (int result_index = 0;
				result_index < nResults && *output != '\0';
				++result_index)
			{
				if (*output++ != '%')
					return LUA_SCPIPT_RESULT_ERROR;

				sol::object value = result.get<sol::object>(result_index);
				switch (*output++)
				{
				case 'd':
					if (!value.is<double>())
						return LUA_SCPIPT_RESULT_ERROR;
					*va_arg(vlist, int*) = static_cast<int>(value.as<double>());
					break;
				case 'n':
					if (!value.is<double>())
						return LUA_SCPIPT_RESULT_ERROR;
					*va_arg(vlist, double*) = value.as<double>();
					break;
				case 'b':
					if (!value.is<bool>())
						return LUA_SCPIPT_RESULT_ERROR;
					*va_arg(vlist, bool*) = value.as<bool>();
					break;
				case 's':
					if (!value.is<std::string>())
						return LUA_SCPIPT_RESULT_ERROR;
					*va_arg(vlist, std::string*) = value.as<std::string>();
					break;
				default:
					return LUA_SCPIPT_RESULT_ERROR;
				}
			}

			return 0;
		}

		int lua_script::get_lua_int_variable(const xstring lua_variable_name)
		{
			if (!(m_is_runing && m_lua_state))
			{
				script_error(LUA_SCRIPT_STATES_IS_NULL);
				return -1;
			}

			sol::object value = sol2().state()[lua_variable_name.c_str()];
			return value.is<double>() ? static_cast<int>(value.as<double>()) : -1;
		}

		double lua_script::get_lua_double_variable(const xstring lua_variable_name)
		{
			if (!(m_is_runing && m_lua_state))
			{
				script_error(LUA_SCRIPT_STATES_IS_NULL);
				return -1;
			}

			sol::object value = sol2().state()[lua_variable_name.c_str()];
			return value.is<double>() ? value.as<double>() : -1;
		}

		xstring lua_script::get_lua_string_variable(const xstring lua_variable_name)
		{
			if (!(m_is_runing && m_lua_state))
			{
				script_error(LUA_SCRIPT_STATES_IS_NULL);
				return {};
			}

			sol::object value = sol2().state()[lua_variable_name.c_str()];
			return value.is<std::string>() ? value.as<std::string>().c_str() : "";
		}

		bool lua_script::get_lua_bool_variable(const xstring lua_variable_name)
		{
			if (!(m_is_runing && m_lua_state))
			{
				script_error(LUA_SCRIPT_STATES_IS_NULL);
				return false;
			}

			sol::object value = sol2().state()[lua_variable_name.c_str()];
			return value.is<bool>() ? value.as<bool>() : false;
		}

		xstring lua_script::get_lua_table_item_variable(
			const xstring lua_table_name,
			int lua_item_row,
			const xstring lua_item_col_name)
		{
			if (!(m_is_runing && m_lua_state))
			{
				script_error(LUA_SCRIPT_STATES_IS_NULL);
				return {};
			}

			sol::state_view state(m_lua_state);
			sol::object table_object = state[lua_table_name.c_str()];
			if (!table_object.is<sol::table>())
				return {};

			sol::table table = table_object.as<sol::table>();
			sol::object row_object = table[lua_item_row];
			if (!row_object.is<sol::table>())
				return {};

			sol::table row = row_object.as<sol::table>();
			return value_to_string(row[lua_item_col_name.c_str()]);
		}

		void lua_script::exit()
		{
			if (m_lua_state != nullptr)
				lua_close(m_lua_state);

			m_lua_state = nullptr;
			m_is_runing = false;
			m_func_map.clear();
		}

		void lua_script::script_error(int error)
		{
			if (error == 0 || m_lua_state == nullptr)
				return;

			std::cerr << "lua_script::script_error: " << error
				<< " (" << m_script_name << ")" << std::endl;
		}

		void lua_script::script_error(int error1, int error2)
		{
			if (m_lua_state == nullptr)
				return;

			std::cerr << "lua_script::script_error: " << error1
				<< "[" << error2 << "] (" << m_script_name << ")"
				<< std::endl;
		}

		bool lua_script::stop()
		{
			if (!m_is_runing)
				return true;
			if (m_lua_state == nullptr)
				return false;

			m_is_runing = false;
			return true;
		}

		bool lua_script::resume()
		{
			if (!m_is_runing && m_lua_state != nullptr)
			{
				m_is_runing = true;
				return true;
			}
			return false;
		}
	}
}
