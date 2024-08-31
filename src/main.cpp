#include <stdlib.h>

#include "lua.h"
#include "lualib.h"

#include "project.h"

void* lua_alloc(void *ud, void *ptr, [[maybe_unused]] size_t osize, size_t nsize)
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
		{
			return malloc(nsize);
		}

		return realloc(ptr, nsize);
	}
}

int main(int argc, char** argv)
{
	lua_State* L = lua_newstate(lua_alloc, nullptr);
	luaL_openlibs(L);

	lua_register(L, "project", prj::parse_project);
	return 0;
}