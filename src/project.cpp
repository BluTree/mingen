#include "project.hpp"

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

#include "fs.hpp"
#include "string.hpp"

namespace
{
	void fill_sources(lua_State*  L,
	                  char const* dir_filter,
	                  char const* file_filter,
	                  uint32_t    srcs_table_id,
	                  uint32_t&   src_id)
	{
		fs::list_files_res files = fs::list_files(dir_filter, file_filter);
		for (uint32_t i {0}; i < files.size; ++i)
		{
			lua_pushstring(L, files.files[i]);
			lua_rawseti(L, srcs_table_id, src_id);
			++src_id;
			delete[] files.files[i];
		}
		if (files.size)
			delete[] files.files;

		fs::list_dirs_res sub_dirs = fs::list_dirs(dir_filter);
		for (uint32_t i {0}; i < sub_dirs.size; ++i)
		{
			uint32_t dir_len = static_cast<uint32_t>(strlen(sub_dirs.dirs[i]));
			char*    filter = new char[dir_len + 3];
			strcpy(filter, sub_dirs.dirs[i]);
			strcpy(filter + dir_len, "/*");
			fill_sources(L, filter, file_filter, srcs_table_id, src_id);

			delete[] filter;
			delete[] sub_dirs.dirs[i];
		}
		if (sub_dirs.size)
			delete[] sub_dirs.dirs;
	}
} // namespace

namespace prj
{
	int parse_project(lua_State* L)
	{
		luaL_argcheck(L, lua_isstring(L, 1), 1, "'string' expected");
		luaL_argcheck(L, lua_istable(L, 2), 2, "'table' expected");

		lua_pushliteral(L, "sources");
		lua_gettable(L, 2);
		luaL_argcheck(L, !lua_isnil(L, -1), 2, "\"sources\" entry expected in table");
		luaL_argcheck(L, lua_istable(L, -1), 2, "\"sources\": array expected");

		uint32_t sources_size {static_cast<uint32_t>(lua_rawlen(L, -1))};
		lua_newtable(L);

		uint32_t sources_processed_size {1};
		for (uint32_t i {0}; i < sources_size; ++i)
		{
			lua_rawgeti(L, -2, i + 1);
			if (lua_isstring(L, -1))
			{
				char const* dir = lua_tostring(L, -1);
				uint32_t    dir_len {static_cast<uint32_t>(strlen(dir))};
				uint32_t    pos {UINT32_MAX};
				if ((pos = str::find(dir, "**", dir_len)) != UINT32_MAX)
				{
					char* filter = new char[pos + 2];
					strncpy(filter, dir, pos + 1);
					filter[pos + 1] = '\0';
					fill_sources(L, filter, dir + pos + 2, 4, sources_processed_size);

					delete[] filter;
				}
				else if ((pos = str::find(dir, "*", dir_len)) != UINT32_MAX)
				{
					char* filter = new char[pos + 2];
					strncpy(filter, dir, pos + 1);
					filter[pos + 1] = '\0';
					fs::list_files_res files = fs::list_files(filter, dir + pos + 1);
					for (uint32_t i {0}; i < files.size; ++i)
					{
						lua_pushstring(L, files.files[i]);
						lua_rawseti(L, 4, sources_processed_size);
						++sources_processed_size;
						delete[] files.files[i];
					}
					if (files.size)
						delete[] files.files;
				}
				else
				{
					// check file exists + add
				}
			}
			lua_pop(L, 1);
		}

		return 1;
	}
} // namespace prj
