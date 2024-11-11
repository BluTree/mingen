#pragma once

#include <stdint.h>

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

struct mingen_state
{
	lua_State* L {nullptr};

	char const*  config_param {nullptr};
	char const** configs {nullptr};
	int32_t      config_size {0};
};

// declared in main.cpp
extern mingen_state g;