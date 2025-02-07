#include "lua_env.hpp"

#include <stdlib.h>
#include <string.h>

#include "generator.hpp"
#include "mem.hpp"
#include "net.hpp"
#include "os.hpp"
#include "project.hpp"
#include "state.hpp"
#include "string.hpp"

#include "fs.hpp"

namespace lua
{

	namespace
	{
		void* lua_alloc(void* ud, void* ptr, [[maybe_unused]] size_t osize, size_t nsize)
		{
			if (!nsize)
			{
				if (ptr)
					tfree(ptr);
				return nullptr;
			}
			else
			{
				if (!ptr)
					return malloc(nsize);

				return realloc(ptr, nsize);
			}
		}

		int32_t configurations(lua_State* L)
		{
			luaL_argcheck(L, lua_istable(L, 1), 1, "'array' expected");

			g.config_size = lua_rawlen(L, 1);

			luaL_argcheck(L, g.config_size > 0, 1,
			              "expecting at least one configuration");

			g.configs = tmalloc<char const*>(g.config_size);

			for (uint32_t i {0}; i < g.config_size; ++i)
			{
				lua_rawgeti(L, 1, i + 1);
				char const* lua_str = lua_tostring(L, -1);
				char*       str = tmalloc<char>(strlen(lua_str) + 1);
				strcpy(str, lua_str);
				g.configs[i] = str;
				lua_pop(L, 1);
			}

			if (!g.config_param)
				g.config_param = g.configs[0];
			return 0;
		}

		int32_t platform(lua_State* L)
		{
#ifdef _WIN32
			lua_pushliteral(L, "windows");
#elif defined(__linux__)
			lua_pushliteral(L, "linux");
#else
			luaL_error(L, "Unknown platform");
#endif
			return 1;
		}

		int32_t need_generate(lua_State* L)
		{
			lua_Debug info;
			lua_getstack(L, 1, &info);
			lua_getinfo(L, "Sl", &info);

			lua_pushboolean(L, strcmp(g.file, info.short_src) == 0);

			return 1;
		}

		int32_t get_build_dir(lua_State* L)
		{
			char* build_dir = resolve_path_to_script(L, "build/");

			lua_pushstring(L, build_dir);

			tfree(build_dir);
			return 1;
		}
	} // namespace

	void create()
	{
		lua_State* L = lua_newstate(lua_alloc, nullptr);
		luaL_openlibs(L);

		lua_getglobal(L, "os");
		lua_pushcclosure(L, os::execute, 0);
		lua_setfield(L, -2, "execute");
		lua_pushcclosure(L, os::copy_file, 0);
		lua_setfield(L, -2, "copy_file");

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

		lua_pushcclosure(L, gen::ninja_generator, 0);
		lua_setfield(L, -2, "generate");

		lua_pushcclosure(L, configurations, 0);
		lua_setfield(L, -2, "configurations");

		lua_pushcclosure(L, platform, 0);
		lua_setfield(L, -2, "platform");

		lua_pushcclosure(L, need_generate, 0);
		lua_setfield(L, -2, "need_generate");

		lua_pushcclosure(L, get_build_dir, 0);
		lua_setfield(L, -2, "get_build_dir");

		lua_setglobal(L, "mg");

		lua_newtable(L);
		lua_pushcclosure(L, net::download, 0);
		lua_setfield(L, -2, "download");

		lua_setglobal(L, "net");

		g.L = L;
	}

	void destroy()
	{
		lua_close(g.L);
		if (g.config_size)
		{
			for (uint32_t i {0}; i < g.config_size; ++i)
				tfree(g.configs[i]);
			tfree(g.configs);
		}
	}

	int32_t run_file(char const* filename)
	{
		g.file = filename;
		if (luaL_dofile(g.L, filename))
		{
			printf("%s", lua_tostring(g.L, -1));
			g.file = nullptr;
			return 1;
		}
		g.file = nullptr;

		return 0;
	}

	char* resolve_path_from_script(lua_State* L, char const* path, uint32_t len)
	{
		if (len == UINT32_MAX)
			len = strlen(path);
		lua_Debug info;
		lua_getstack(L, 1, &info);
		lua_getinfo(L, "Sl", &info);

		uint32_t res_path_capacity = len;
		uint32_t res_path_size = 0;
		char*    res_path = tmalloc<char>(res_path_capacity + 1);

		uint32_t pos = 0;
		if (str::starts_with(info.short_src, "./") ||
		    str::starts_with(info.short_src, ".\\"))
			pos = 2;
		else
		{
			char* cwd = fs::get_cwd();
			if (str::starts_with(info.short_src, cwd))
			{
				if (str::ends_with(cwd, "/") || str::ends_with(cwd, "\\"))
					pos = strlen(cwd);
				else
					pos = strlen(cwd) + 1;
			}
			else
			{
				strcpy(res_path, path);
				tfree(cwd);
				return res_path;
			}
			tfree(cwd);
		}

		uint32_t find_pos = str::rfind(info.short_src + pos, "/");
		if (find_pos == UINT32_MAX)
			find_pos = str::rfind(info.short_src + pos, "\\");

		if (find_pos != UINT32_MAX)
		{
			if (res_path_capacity < res_path_size + find_pos + 1 + len)
			{
				while (res_path_capacity < res_path_size + find_pos + 1 + len)
					res_path_capacity *= 2;
				res_path = trealloc(res_path, res_path_capacity + 1);
			}

			strncpy(res_path + res_path_size, info.short_src + pos, find_pos + 1);
			res_path_size += find_pos + 1;
		}
		else if (res_path_capacity < res_path_size + len)
		{
			while (res_path_capacity < res_path_size + len)
				res_path_capacity *= 2;
			res_path = trealloc(res_path, res_path_capacity + 1);
		}
		strncpy(res_path + res_path_size, path, len);
		res_path[res_path_size + len] = '\0';
		return res_path;
	}

	char* resolve_path_to_script(lua_State* L, char const* path, uint32_t len)
	{
		if (len == UINT32_MAX)
			len = strlen(path);
		lua_Debug info;
		lua_getstack(L, 1, &info);
		lua_getinfo(L, "Sl", &info);

		uint32_t res_path_capacity = len;
		uint32_t res_path_size = 0;
		char*    res_path = tmalloc<char>(res_path_capacity + 1);

		uint32_t pos = 0;
		if (str::starts_with(info.short_src, "./") ||
		    str::starts_with(info.short_src, ".\\"))
			pos = 2;
		else
		{
			char* cwd = fs::get_cwd();
			if (str::starts_with(info.short_src, cwd))
			{
				if (str::ends_with(cwd, "/") || str::ends_with(cwd, "\\"))
					pos = strlen(cwd);
				else
					pos = strlen(cwd) + 1;
			}
			else
			{
				strcpy(res_path, path);
				tfree(cwd);
				return res_path;
			}
			tfree(cwd);
		}

		uint32_t current_script_dir_count = 0;
		uint32_t path_dir_count = 0;

		for (uint32_t i {pos}; i < strlen(info.short_src); ++i)
			if (info.short_src[i] == '/' || info.short_src[i] == '\\')
				++current_script_dir_count;

		while (current_script_dir_count > 0)
		{
			if (res_path_capacity < res_path_size + 3 /*../*/)
			{
				while (res_path_capacity < res_path_size + 3)
					res_path_capacity *= 2;
				res_path = trealloc(res_path, res_path_capacity + 1);
			}

			strncpy(res_path + res_path_size, "../", 3);
			res_path_size += 3;
			--current_script_dir_count;
			uint32_t find_pos = str::find(info.short_src + pos, "/");
			if (find_pos == UINT32_MAX)
				find_pos = str::find(info.short_src + pos, "\\");
			pos += find_pos + 1;
		}

		uint32_t find_pos = str::rfind(info.short_src + pos, "/");
		if (find_pos == UINT32_MAX)
			find_pos = str::rfind(info.short_src + pos, "\\");

		if (find_pos != UINT32_MAX)
		{
			if (res_path_capacity < res_path_size + find_pos + 1 + len)
			{
				while (res_path_capacity < res_path_size + find_pos + 1 + len)
					res_path_capacity *= 2;
				res_path = trealloc(res_path, res_path_capacity + 1);
			}

			strncpy(res_path + res_path_size, info.short_src + pos, find_pos + 1);
			res_path_size += find_pos + 1;
		}
		else if (res_path_capacity < res_path_size + len)
		{
			while (res_path_capacity < res_path_size + len)
				res_path_capacity *= 2;
			res_path = trealloc(res_path, res_path_capacity + 1);
		}
		strncpy(res_path + res_path_size, path, len);
		res_path[res_path_size + len] = '\0';
		return res_path;
	}

	// NOLINTBEGIN(clang-analyzer-unix.Malloc)

	namespace
	{
		bool
		parse_config_input(lua_State* L, char const* key, int32_t value_type, input& in)
		{
			if (strcmp(key, "sources") == 0)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "sources: expecting array");

				uint32_t len = lua_rawlen(L, -1);
				if (!len)
					return true;
				in.sources = trealloc(in.sources, in.sources_size + len);
				for (uint32_t i {in.sources_size}; i < in.sources_size + len; ++i)
				{
					lua_rawgeti(L, -1, i + 1);
					if (lua_isstring(L, -1))
					{
						char const* lua_str = lua_tostring(L, -1);
						char*       str = tmalloc<char>(strlen(lua_str) + 1);
						strcpy(str, lua_str);
						in.sources[i] = str;
					}
					lua_pop(L, 1);
				}
				in.sources_size += len;

				return true;
			}
			else if (strcmp(key, "includes") == 0)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "includes: expecting array");

				uint32_t len = lua_rawlen(L, -1);
				if (!len)
					return true;
				in.includes = trealloc(in.includes, in.includes_size + len);
				for (uint32_t i {in.includes_size}; i < in.includes_size + len; ++i)
				{
					lua_rawgeti(L, -1, i - in.includes_size + 1);
					if (lua_isstring(L, -1))
					{
						char const* lua_str = lua_tostring(L, -1);
						char*       str = tmalloc<char>(strlen(lua_str) + 1);
						strcpy(str, lua_str);
						in.includes[i] = str;
					}
					lua_pop(L, 1);
				}
				in.includes_size += len;

				return true;
			}
			else if (strcmp(key, "compile_options") == 0)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "compile_options: expecting array");

				uint32_t len = lua_rawlen(L, -1);
				if (!len)
					return true;
				in.compile_options =
					trealloc(in.compile_options, in.compile_options_size + len);
				for (uint32_t i {in.compile_options_size};
				     i < in.compile_options_size + len; ++i)
				{
					lua_rawgeti(L, -1, i - in.compile_options_size + 1);
					if (lua_isstring(L, -1))
					{
						char const* lua_str = lua_tostring(L, -1);
						char*       str = tmalloc<char>(strlen(lua_str) + 1);
						strcpy(str, lua_str);
						in.compile_options[i] = str;
					}
					lua_pop(L, 1);
				}
				in.compile_options_size += len;

				return true;
			}
			else if (strcmp(key, "link_options") == 0)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "link_options: expecting array");

				uint32_t len = lua_rawlen(L, -1);
				if (!len)
					return true;
				in.link_options = trealloc(in.link_options, in.link_options_size + len);
				for (uint32_t i {in.link_options_size}; i < in.link_options_size + len;
				     ++i)
				{
					lua_rawgeti(L, -1, i - in.link_options_size + 1);
					if (lua_isstring(L, -1))
					{
						char const* lua_str = lua_tostring(L, -1);
						char*       str = tmalloc<char>(strlen(lua_str) + 1);
						strcpy(str, lua_str);
						in.link_options[i] = str;
					}
					lua_pop(L, 1);
				}

				in.link_options_size += len;

				return true;
			}
			else if (strcmp(key, "dependencies") == 0)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "dependencies: expecting array");

				uint32_t len = lua_rawlen(L, -1);
				if (!len)
					return true;
				in.deps = trealloc(in.deps, in.deps_size + len);
				for (uint32_t i {in.deps_size}; i < in.deps_size + len; ++i)
				{
					lua_rawgeti(L, -1, i - in.deps_size + 1);
					if (lua_istable(L, -1))
						in.deps[i] = parse_output(L);
					lua_pop(L, 1);
				}

				in.deps_size += len;

				return true;
			}
			else if (strcmp(key, "static_libraries") == 0 &&
			         in.type == project_type::prebuilt)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "static_libraries: expecting array");

				uint32_t len = lua_rawlen(L, -1);
				if (!len)
					return true;

				in.static_libraries =
					trealloc(in.static_libraries, in.static_libraries_size + len);
				for (uint32_t i {in.static_libraries_size};
				     i < in.static_libraries_size + len; ++i)
				{
					lua_rawgeti(L, -1, i - in.static_libraries_size + 1);
					if (lua_isstring(L, -1))
					{
						char const* lua_str = lua_tostring(L, -1);
						char*       str = tmalloc<char>(strlen(lua_str) + 1);
						strcpy(str, lua_str);
						in.static_libraries[i] = str;
					}
					lua_pop(L, 1);
				}

				in.static_libraries_size += len;

				return true;
			}
			else if (strcmp(key, "static_library_directories") == 0 &&
			         in.type == project_type::prebuilt)
			{
				if (value_type != LUA_TTABLE)
					luaL_error(L, "static_library_directories: expecting array");

				uint32_t len = lua_rawlen(L, -1);
				if (!len)
					return true;

				in.static_library_directories =
					trealloc(in.static_library_directories,
				             in.static_library_directories_size + len);
				for (uint32_t i {in.static_library_directories_size};
				     i < in.static_library_directories_size + len; ++i)
				{
					lua_rawgeti(L, -1, i - in.static_library_directories_size + 1);
					if (lua_isstring(L, -1))
					{
						char const* lua_str = lua_tostring(L, -1);
						char*       str = tmalloc<char>(strlen(lua_str) + 1);
						strcpy(str, lua_str);
						in.static_library_directories[i] = str;
					}
					lua_pop(L, 1);
				}

				in.static_library_directories_size += len;

				return true;
			}

			return false;
		}
	} // namespace

	input parse_input(lua_State* L, int32_t idx)
	{
		input in {0};
		in.type = project_type::count;

		if (idx != -1)
			lua_pushvalue(L, idx);
		// TODO manually read allowed configs before iterating on table, because lua_next
		// doesn't order the values in initialisation order

		lua_getfield(L, -1, "type");
		if (lua_type(L, -1) == LUA_TTABLE)
		{
			lua_getfield(L, -1, "__project_type_enum_value");
			if (lua_isnil(L, -1))
				luaL_error(L, "type: expecting project_type enum");
			in.type = static_cast<project_type>(lua_tointeger(L, -1));

			lua_pop(L, 1);
		}
		else if (lua_type(L, -1) != LUA_TNIL)
			luaL_error(L, "type: expecting project_type enum");
		else
		{
			luaL_error(L, "missing key: type");
		}
		lua_pop(L, 1);

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
				char*       name = tmalloc<char>(strlen(lua_name) + 1);
				strcpy(name, lua_name);
				in.name = name;
			}
			else if (strcmp(key, "type") == 0)
			{
			}
			else if (strcmp(key, g.config_param) == 0)
			{
				if (value_type != LUA_TTABLE)
				{
					luaL_error(L, "%s: expecting table", key);
				}
				else
				{
					lua_pushnil(L);

					while (lua_next(L, -2))
					{
						char const* config_key = lua_tostring(L, -2);
						int32_t     config_value_type = lua_type(L, -1);
						if (!parse_config_input(L, config_key, config_value_type, in))
						{
							// TODO not error output
							luaL_error(L, "Unknown key: %s", key);
						}
						lua_pop(L, 1);
					}
				}
			}
			else if (!parse_config_input(L, key, value_type, in))
			{
				bool err {true};

				for (uint32_t i {0}; i < g.config_size; ++i)
				{
					if (strcmp(key, g.configs[i]) == 0)
					{
						err = false;
						break;
					}
				}

				if (err)
				{
					luaL_where(L, 1);
					char const* src = lua_tostring(L, -1);
					printf("%s: Unknown key: %s\n", src, key);
					lua_pop(L, 1);
				}
			}
			lua_pop(L, 1);
		}

		if (!in.name)
			luaL_error(L, "missing key: name");
		if (idx != -1)
			lua_pop(L, 1);

		return in;
	}

	// NOLINTEND(clang-analyzer-unix.Malloc)

	void free_input(input const& in)
	{
		if (in.name)
			tfree(in.name);

		if (in.sources)
		{
			for (uint32_t i {0}; i < in.sources_size; ++i)
				if (in.sources[i])
					tfree(in.sources[i]);
			tfree(in.sources);
		}

		if (in.compile_options)
		{
			for (uint32_t i {0}; i < in.compile_options_size; ++i)
				if (in.compile_options[i])
					tfree(in.compile_options[i]);
			tfree(in.compile_options);
		}

		if (in.link_options)
		{
			for (uint32_t i {0}; i < in.link_options_size; ++i)
				if (in.link_options[i])
					tfree(in.link_options[i]);
			tfree(in.link_options);
		}

		if (in.deps)
		{
			for (uint32_t i {0}; i < in.deps_size; ++i)
				free_output(in.deps[i]);
			tfree(in.deps);
		}

		if (in.static_libraries)
		{
			for (uint32_t i {0}; i < in.static_libraries_size; ++i)
				if (in.static_libraries[i])
					tfree(in.static_libraries[i]);
			tfree(in.static_libraries);
		}

		if (in.static_library_directories)
		{
			for (uint32_t i {0}; i < in.static_library_directories_size; ++i)
				if (in.static_library_directories[i])
					tfree(in.static_library_directories[i]);
			tfree(in.static_library_directories);
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

	// NOLINTBEGIN(clang-analyzer-unix.Malloc)

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
				char*       name = tmalloc<char>(strlen(lua_name) + 1);
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
				out.sources = tmalloc<output::source>(len);
				for (uint32_t i {0}; i < len; ++i)
				{
					lua_rawgeti(L, -1, i + 1);
					if (lua_istable(L, -1))
					{
						lua_getfield(L, -1, "file");
						if (lua_isstring(L, -1))
						{
							char const* lua_file = lua_tostring(L, -1);
							char*       file = tmalloc<char>(strlen(lua_file) + 1);
							strcpy(file, lua_file);
							out.sources[i].file = file;
						}
						lua_getfield(L, -1, "compile_options");
						if (lua_isstring(L, -1))
						{
							char const* lua_compile_options = lua_tostring(L, -1);
							char*       compile_options =
								tmalloc<char>(strlen(lua_compile_options) + 1);
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
				char* compile_options = tmalloc<char>(strlen(lua_compile_options) + 1);
				strcpy(compile_options, lua_compile_options);
				out.compile_options = compile_options;
			}
			else if (strcmp(key, "link_options") == 0)
			{
				if (value_type != LUA_TSTRING)
					luaL_error(L, "link_options: expecting string");

				char const* lua_link_options = lua_tostring(L, -1);
				char*       link_options = tmalloc<char>(strlen(lua_link_options) + 1);
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
				out.deps = tmalloc<output>(len);
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

		if (idx != -1)
			lua_pop(L, 1);

		return out;
	}

	// NOLINTEND(clang-analyzer-unix.Malloc)

	void free_output(output const& out)
	{
		if (out.name)
			tfree(out.name);

		if (out.sources)
		{
			for (uint32_t i {0}; i < out.sources_size; ++i)
			{
				tfree(out.sources[i].file);
				if (out.sources[i].compile_options)
					tfree(out.sources[i].compile_options);
			}
			tfree(out.sources);
		}

		if (out.compile_options)
			tfree(out.compile_options);

		if (out.link_options)
			tfree(out.link_options);
	}
} // namespace lua