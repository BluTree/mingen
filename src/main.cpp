#include <stdlib.h>

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

#include "project.hpp"

void* lua_alloc(void* ud, void* ptr, [[maybe_unused]] size_t osize, size_t nsize)
{
	if (!nsize)
	{
		if (ptr)
			free(ptr);
		return nullptr;
	}
	else
	{
		if (!ptr)
			return malloc(nsize);

		return realloc(ptr, nsize);
	}
}

int main(int argc, char** argv)
{
	lua_State* L = lua_newstate(lua_alloc, nullptr);
	luaL_openlibs(L);

	lua_register(L, "project", prj::parse_project);

	luaL_dofile(L, "sample.lua");
	return 0;
}