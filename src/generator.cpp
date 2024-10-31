#include "generator.hpp"
#include "lua_env.hpp"

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

#include <malloc.h>
#include <stdio.h>
#include <string.h>

namespace generator
{
	namespace
	{
		void generate(lua::output const& out, FILE* file)
		{
			for (uint32_t i {0}; i < out.deps_size; ++i)
				generate(out.deps[i], file);
		}
	}

	int32_t ninja_generator(lua_State* L)
	{
		int32_t len = lua_rawlen(L, 1);

		luaL_argcheck(L, len > 0, 1, "generate must not be empty");

		// TODO create fs::open_file / fs::close_file
		FILE* file = fopen("build/build.ninja", "w");

		for (uint32_t i {1}; i < len; ++i)
		{
			lua::output out = lua::parse_output(L, i);

			// create rules

			for (uint32_t j {0}; j < out.deps_size; ++j)
			{
			}
		}
		fclose(file);
		return 0;
	}
} // namespace generator