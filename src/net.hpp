#pragma once

#include <stdint.h>

struct lua_State;

namespace net
{
	int32_t download(lua_State* L);
}