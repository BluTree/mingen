#include "project.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

namespace prj
{
	int parse_project(lua_State* L)
	{
		luaL_argcheck(L, lua_isstring(L, 1), 1, "'string' expected");
		luaL_argcheck(L, lua_istable(L, 2), 2, "'table' expected");

		return 0;
	}
}
