#include "lua_env.hpp"

#include <stdlib.h>
#include <string.h>

#include "generator.hpp"
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

		lua_pushcclosure(L, prj::new_project, 0);
		lua_setfield(L, -2, "project");

		lua_newtable(L);
		for (uint32_t i {0}; i < project_type::count; ++i)
		{
			lua_newtable(L);
			lua_pushinteger(L, i);
			lua_setfield(L, -2, "__project_type_enum_value");
			lua_setfield(L, -2, project_type_names[i]);
		}
		lua_setfield(L, -2, "project_type");

		// TODO generate func
		lua_pushcclosure(L, gen::ninja_generator, 0);
		lua_setfield(L, -2, "generate");

		lua_setglobal(L, "mg");
	}

	void run_file(char const* filename)
	{
		if (luaL_dofile(L, filename))
			printf("%s", lua_tostring(L, -1));
	}

	// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)

	input parse_input(lua_State* L, int32_t idx)
	{
		input in {0};
		in.type = project_type::count;

		if (idx != -1)
			lua_pushvalue(L, idx);
		// TODO manually read allowed configs before iterating on table, because lua_next
		// doesn't order the values in initialisation order

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
				char*       name = new char[strlen(lua_name) + 1];
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
					lua_getfield(L, -1, "__project_type_enum_value");
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
						char*       str = new char[strlen(lua_str) + 1];
						strcpy(str, lua_str);
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
						char*       str = new char[strlen(lua_str) + 1];
						strcpy(str, lua_str);
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
						char*       str = new char[strlen(lua_str) + 1];
						strcpy(str, lua_str);
						in.link_options[i] = str;
					}
					lua_pop(L, 1);
				}
			}
			else if (strcmp(key, "dependencies") == 0)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "dependencies: expecting array");

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
			lua_pop(L, 1);
		}

		if (!in.name)
			luaL_error(L, "missing key: name");
		if (in.type == project_type::count)
			luaL_error(L, "missing key: type");

		return in;
	}

	// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

	void free_input(input const& in)
	{
		if (in.name)
			delete[] in.name;

		if (in.sources)
		{
			for (uint32_t i {0}; i < in.sources_size; ++i)
				if (in.sources[i])
					delete[] in.sources[i];
			delete[] in.sources;
		}

		if (in.compile_options)
		{
			for (uint32_t i {0}; i < in.compile_options_size; ++i)
				if (in.compile_options[i])
					delete[] in.compile_options[i];
			delete[] in.compile_options;
		}

		if (in.link_options)
		{
			for (uint32_t i {0}; i < in.link_options_size; ++i)
				if (in.link_options[i])
					delete[] in.link_options[i];
			delete[] in.link_options;
		}

		if (in.deps)
		{
			for (uint32_t i {0}; i < in.deps_size; ++i)
				free_output(in.deps[i]);
			delete[] in.deps;
		}
	}

	void dump_output(lua_State* L, output const& out)
	{
		lua_newtable(L);

		lua_pushstring(L, out.name);
		lua_setfield(L, -2, "name");

		lua_newtable(L);
		lua_pushinteger(L, static_cast<int32_t>(out.type));
		lua_setfield(L, -2, "__project_type_enum_value");
		lua_setfield(L, -2, "type");

		if (out.sources)
		{
			lua_newtable(L);
			for (uint32_t i {0}; i < out.sources_size; ++i)
			{
				lua_newtable(L);
				lua_pushstring(L, out.sources[i].file);
				lua_setfield(L, -2, "file");

				if (out.sources[i].compile_options)
				{
					lua_pushstring(L, out.sources[i].compile_options);
					lua_setfield(L, -2, "compile_options");
				}
				lua_rawseti(L, -2, i + 1);
			}
			lua_setfield(L, -2, "sources");
		}

		if (out.compile_options)
		{
			lua_pushstring(L, out.compile_options);
			lua_setfield(L, -2, "compile_options");
		}

		if (out.link_options)
		{
			lua_pushstring(L, out.link_options);
			lua_setfield(L, -2, "link_options");
		}

		if (out.deps)
		{
			lua_newtable(L);
			for (uint32_t i {0}; i < out.deps_size; ++i)
			{
				dump_output(L, out.deps[i]);
				lua_rawseti(L, -2, i + 1);
			}
			lua_setfield(L, -2, "dependencies");
		}
	}

	// NOLINTBEGIN(clang-analyzer-cplusplus.NewDeleteLeaks)

	output parse_output(lua_State* L, int32_t idx)
	{
		output out {0};

		if (idx != -1)
			lua_pushvalue(L, idx);

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
				char*       name = new char[strlen(lua_name) + 1];
				strcpy(name, lua_name);
				out.name = name;
			}
			else if (strcmp(key, "type") == 0)
			{
				if (value_type != LUA_TTABLE)
				{
					luaL_error(L, "type: expecting project_type enum");
				}
				else
				{
					lua_getfield(L, -1, "__project_type_enum_value");
					if (lua_isnil(L, -1))
						luaL_error(L, "type: expecting project_type enum");
					out.type = static_cast<project_type>(lua_tointeger(L, -1));
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
				out.sources_size = out.sources_capacity = len;
				out.sources = new output::source[len];
				for (uint32_t i {0}; i < len; ++i)
				{
					lua_rawgeti(L, -1, i + 1);
					if (lua_istable(L, -1))
					{
						lua_getfield(L, -1, "file");
						if (lua_isstring(L, -1))
						{
							char const* lua_file = lua_tostring(L, -1);
							char*       file = new char[strlen(lua_file) + 1];
							strcpy(file, lua_file);
							out.sources[i].file = file;
						}
						lua_getfield(L, -1, "compile_options");
						if (lua_isstring(L, -1))
						{
							char const* lua_compile_options = lua_tostring(L, -1);
							char*       compile_options =
								new char[strlen(lua_compile_options) + 1];
							strcpy(compile_options, lua_compile_options);
							out.sources[i].compile_options = compile_options;
						}
						else
						{
							out.sources[i].compile_options = nullptr;
						}
						lua_pop(L, 2);
					}
					lua_pop(L, 1);
				}
			}
			else if (strcmp(key, "compile_options") == 0)
			{
				if (value_type != LUA_TSTRING)
					luaL_error(L, "compile_options: expecting string");

				char const* lua_compile_options = lua_tostring(L, -1);
				char*       compile_options = new char[strlen(lua_compile_options) + 1];
				strcpy(compile_options, lua_compile_options);
				out.compile_options = compile_options;
			}
			else if (strcmp(key, "link_options") == 0)
			{
				if (value_type != LUA_TSTRING)
					luaL_error(L, "link_options: expecting string");

				char const* lua_link_options = lua_tostring(L, -1);
				char*       link_options = new char[strlen(lua_link_options) + 1];
				strcpy(link_options, lua_link_options);
				out.link_options = link_options;
			}
			else if (strcmp(key, "dependencies") == 0)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "dependencies: expecting array");

				uint32_t len = lua_rawlen(L, -1);
				if (!len)
					continue;

				out.deps_size = len;
				out.deps = new output[len];
				for (uint32_t i {0}; i < len; ++i)
				{
					lua_rawgeti(L, -1, i + 1);
					if (lua_istable(L, -1))
						out.deps[i] = parse_output(L);
					lua_pop(L, 1);
				}
			}
			lua_pop(L, 1);
		}

		return out;
	}

	// NOLINTEND(clang-analyzer-cplusplus.NewDeleteLeaks)

	void free_output(output const& out)
	{
		if (out.name)
			delete[] out.name;

		if (out.sources)
		{
			for (uint32_t i {0}; i < out.sources_size; ++i)
			{
				delete[] out.sources[i].file;
				if (out.sources[i].compile_options)
					delete[] out.sources[i].compile_options;
			}
			delete[] out.sources;
		}

		if (out.compile_options)
			delete[] out.compile_options;

		if (out.link_options)
			delete[] out.link_options;
	}
} // namespace lua