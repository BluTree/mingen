#include "generator.hpp"

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

namespace generator
{
	int32_t ninja_generator(lua_State* L)
	{
		return 0;
	}
}