#ifndef _FAITH_LUA_SOL2_INTERFACE_HPP_
#define _FAITH_LUA_SOL2_INTERFACE_HPP_

#include <string>
#include <utility>

#include <sol/sol.hpp>

namespace faith
{
	namespace lua
	{
		namespace sol2_api
		{
			// Non-owning sol2 wrapper for an existing lua_State.
			// The caller owns the lifetime of the lua_State.
			class sol2_interface
			{
			public:
				explicit sol2_interface(lua_State* state) noexcept
					: state_(state)
				{
				}

				bool valid() const noexcept
				{
					return state_ != nullptr;
				}

				lua_State* native_state() const noexcept
				{
					return state_;
				}

				sol::state_view state() const
				{
					return sol::state_view(state_);
				}

				template <typename... Libraries>
				void open_libraries(Libraries&&... libraries) const
				{
					if (valid())
					{
						state().open_libraries(
							std::forward<Libraries>(libraries)...);
					}
				}

				template <typename Function, typename... Args>
				void set_function(const std::string& name,
					Function&& function,
					Args&&... args) const
				{
					if (valid())
					{
						state().set_function(
							name,
							std::forward<Function>(function),
							std::forward<Args>(args)...);
					}
				}

				bool run_file(const std::string& filename,
					std::string* error_message = nullptr) const
				{
					if (!valid())
					{
						set_error_(error_message, "lua_State is null");
						return false;
					}

					sol::protected_function_result result =
						state().safe_script_file(filename);
					return handle_result_(result, error_message);
				}

				bool run_string(const std::string& script,
					std::string* error_message = nullptr) const
				{
					if (!valid())
					{
						set_error_(error_message, "lua_State is null");
						return false;
					}

					sol::protected_function_result result =
						state().safe_script(script);
					return handle_result_(result, error_message);
				}

				sol::table create_table() const
				{
					return state().create_table();
				}

			private:
				static void set_error_(std::string* error_message,
					const std::string& message)
				{
					if (error_message != nullptr)
					{
						*error_message = message;
					}
				}

				static bool handle_result_(
					const sol::protected_function_result& result,
					std::string* error_message)
				{
					if (result.valid())
					{
						return true;
					}

					if (error_message != nullptr)
					{
						sol::error error = result;
						*error_message = error.what();
					}
					return false;
				}

				lua_State* state_;
			};
		}
	}
}

#endif
