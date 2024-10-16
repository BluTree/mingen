#include "lua_env.hpp"

#include <stdlib.h>
#include <string.h>

#include "project.hpp"

namespace
{
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
} // namespace

namespace lua
{
	lua_State* L {nullptr};

	void create()
	{
		L = lua_newstate(lua_alloc, nullptr);
		luaL_openlibs(L);

		lua_newtable(L);

		lua_pushcclosure(L, prj::parse_project, 0);
		lua_setfield(L, -2, "project");

		lua_newtable(L);
		for (uint32_t i {0}; i < project_type::count; ++i)
		{
			lua_newtable(L);
			lua_pushinteger(L, i);
			lua_setfield(L, -2, "__enum_value");
			lua_setfield(L, -2, project_type_names[i]);
		}
		lua_setfield(L, -2, "project_type");

		// TODO generate func

		lua_setglobal(L, "mg");
	}

	void run_file(char const* filename)
	{
		luaL_dofile(L, filename);
	}

	// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)
	input parse_input(lua_State* L)
	{
		input in {};

		lua_pushnil(L);

		while (lua_next(L, -2))
		{
			char const* key = lua_tostring(L, -2);
			int32_t     value_type = lua_type(L, -1);
			if (strcmp(key, "name") == 0)
			{
				if (value_type != LUA_TSTRING)
					luaL_error(L, "name: expecting string");

				char const* lua_name = lua_tostring(L, -1);
				char*       name = new char[strlen(lua_name)];
				strcpy(name, lua_name);
				in.name = name;
			}
			else if (strcmp(key, "type") == 0)
			{
				if (value_type != LUA_TTABLE)
				{
					luaL_error(L, "type: expecting project_type enum");
				}
				else
				{
					lua_getfield(L, -1, "__enum_value");
					if (lua_isnil(L, -1))
						luaL_error(L, "type: expecting project_type enum");
					in.type = static_cast<project_type>(lua_tointeger(L, -1));
				}
				lua_pop(L, 1);
			}
			else if (strcmp(key, "sources") == 0)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "sources: expecting array");

				uint32_t len = lua_rawlen(L, -1);
				if (!len)
					continue;
				in.sources_size = len;
				in.sources = new char const*[len];
				for (uint32_t i {0}; i < len; ++i)
				{
					lua_rawgeti(L, -1, i + 1);
					if (lua_isstring(L, -1))
					{
						char const* lua_str = lua_tostring(L, -1);
						char*       str = new char[strlen(lua_str)];
						in.sources[i] = str;
					}
					lua_pop(L, 1);
				}
			}
			else if (strcmp(key, "compile_options") == 0)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "compile_options: expecting array");

				uint32_t len = lua_rawlen(L, -1);
				if (!len)
					continue;
				in.compile_options_size = len;
				in.compile_options = new char const*[len];
				for (uint32_t i {0}; i < len; ++i)
				{
					lua_rawgeti(L, -1, i + 1);
					if (lua_isstring(L, -1))
					{
						char const* lua_str = lua_tostring(L, -1);
						char*       str = new char[strlen(lua_str)];
						in.compile_options[i] = str;
					}
					lua_pop(L, 1);
				}
			}
			else if (strcmp(key, "link_options") == 0)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "link_options: expecting array");

				uint32_t len = lua_rawlen(L, -1);
				if (!len)
					continue;
				in.link_options_size = len;
				in.link_options = new char const*[len];
				for (uint32_t i {0}; i < len; ++i)
				{
					lua_rawgeti(L, -1, i + 1);
					if (lua_isstring(L, -1))
					{
						char const* lua_str = lua_tostring(L, -1);
						char*       str = new char[strlen(lua_str)];
						in.link_options[i] = str;
					}
					lua_pop(L, 1);
				}
			}
			else if (strcmp(key, "dependencies") == 0)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "link_options: expecting array");

				uint32_t len = lua_rawlen(L, -1);
				if (!len)
					continue;

				in.deps_size = len;
				in.deps = new output[len];
				for (uint32_t i {0}; i < len; ++i)
				{
					lua_rawgeti(L, -1, i + 1);
					if (lua_istable(L, -1))
						in.deps[i] = parse_output(L);
					lua_pop(L, 1);
				}
			}
			else
			{
				// TODO not error output
				luaL_error(L, "Unknown key: %s", key);
			}
		}

		return in;
	}

	// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

	void free_input(input const& in)
	{
		if (in.name)
			delete[] in.name;

		if (in.sources)
			for (uint32_t i {0}; i < in.sources_size; ++i)
				delete[] in.sources[i];

		if (in.compile_options)
			for (uint32_t i {0}; i < in.compile_options_size; ++i)
				delete[] in.compile_options[i];

		if (in.link_options)
			for (uint32_t i {0}; i < in.link_options_size; ++i)
				delete[] in.link_options[i];

		if (in.deps)
			for (uint32_t i {0}; i < in.deps_size; ++i)
				free_output(in.deps[i]);
	}

	void dump_output(lua_State* L, output const& out) {}

	output parse_output(lua_State* L)
	{
		output out {};

		return out;
	}

	void free_output(output const& out) {}
} // namespace lua