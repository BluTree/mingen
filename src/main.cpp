#include "lua_env.hpp"
#include "project.hpp"

#include <win32/io.h>

int main(int argc, char** argv)
{
	SetCurrentDirectoryW(L"./tests");
	lua::create();
	lua::run_file("mingen.lua");
	return 0;
}