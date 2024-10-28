#pragma once

#include <stdint.h>

struct lua_State;

namespace gen
{
	int32_t ninja_generator(lua_State* L);
}
