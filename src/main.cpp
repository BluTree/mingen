#include "lua_env.hpp"
#include "project.hpp"

int main(int argc, char** argv)
{
	lua::create();
	lua::run_file("sample.lua");
	return 0;
}