#include "project.hpp"

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

#include "fs.hpp"
#include "lua_env.hpp"
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
	int new_project(lua_State* L)
	{
		luaL_argcheck(L, lua_istable(L, 1), 2, "'table' expected");
		lua::input  in = lua::parse_input(L);
		lua::output out {0};

		for (uint32_t i {0}; i < in.sources_size; ++i)
		{
			uint32_t source_len {static_cast<uint32_t>(strlen(in.sources[i]))};
			uint32_t pos {UINT32_MAX};

			if ((pos = str::find(in.sources[i], "**", source_len)) != UINT32_MAX)
			{
				char* filter = new char[pos + 2];
				strncpy(filter, in.sources[i], pos + 1);
				filter[pos + 1] = '\0';
				// TODO fill_sources
				delete[] filter;
			}
			else if ((pos = str::find(in.sources[i], "*", source_len)) != UINT32_MAX)
			{
				char* filter = new char[pos + 2];
				strncpy(filter, in.sources[i], pos + 1);
				filter[pos + 1] = '\0';
				fs::list_files_res files =
					fs::list_files(filter, in.sources[i] + pos + 1);
				if (out.sources_capacity < out.sources_size + files.size)
				{
					out.sources_capacity *= 2;
					lua::output::source* new_sources =
						new lua::output::source[out.sources_capacity];
					memcpy(new_sources, out.sources,
					       out.sources_size * sizeof(lua::output::source));
					delete[] out.sources;
					out.sources = new_sources;
				}
				for (uint32_t i {0}; i < files.size; ++i)
					out.sources[out.sources_size + i].file = files.files[i];
				if (files.size)
					delete[] files.files;
			}
			else
			{
			}
		}

		lua::dump_output(L, out);
		lua::free_output(out);

		return 1;
	}
} // namespace prj
