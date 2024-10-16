#include "lua_env.hpp"
#include "project.hpp"

int main(int argc, char** argv)
{
	int i = 0;
	lua::create();
	lua::run_file("test/mingen.lua");
	return 0;
}