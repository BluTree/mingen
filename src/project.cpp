#include "project.hpp"

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

#include "fs.hpp"
#include "lua_env.hpp"
#include "mem.hpp"
#include "string.hpp"

namespace prj
{
	namespace
	{
		void
		fill_sources(char const* dir_filter, char const* file_filter, lua::output& out)
		{
			fs::list_files_res files = fs::list_files(dir_filter, file_filter);
			if (out.sources_capacity < out.sources_size + files.size)
			{
				if (!out.sources_capacity)
					out.sources_capacity = 1;
				else
					out.sources_capacity *= 2;
				lua::output::source* new_sources = trealloc(
					out.sources, out.sources_capacity * sizeof(lua::output::source));
				out.sources = new_sources;
			}
			for (uint32_t i {0}; i < files.size; ++i)
			{
				out.sources[out.sources_size + i].file = files.files[i];
				out.sources[out.sources_size + i].compile_options = nullptr;
			}
			out.sources_size += files.size;
			if (files.size)
				tfree(files.files);

			fs::list_dirs_res sub_dirs = fs::list_dirs(dir_filter);
			for (uint32_t i {0}; i < sub_dirs.size; ++i)
			{
				uint32_t dir_len = static_cast<uint32_t>(strlen(sub_dirs.dirs[i]));
				char*    filter = tmalloc<char>(dir_len + 3);
				strcpy(filter, sub_dirs.dirs[i]);
				strcpy(filter + dir_len, "/*");
				fill_sources(filter, file_filter, out);

				tfree(filter);
				tfree(sub_dirs.dirs[i]);
			}
			if (sub_dirs.size)
				tfree(sub_dirs.dirs);
		}
	} // namespace

	int new_project(lua_State* L)
	{
		luaL_argcheck(L, lua_istable(L, 1), 2, "'table' expected");
		lua::input  in = lua::parse_input(L);
		lua::output out {0};

		out.name = in.name;
		in.name = nullptr;

		out.type = in.type;

		for (uint32_t i {0}; i < in.sources_size; ++i)
		{
			uint32_t source_len {static_cast<uint32_t>(strlen(in.sources[i]))};
			uint32_t pos {UINT32_MAX};

			if ((pos = str::find(in.sources[i], "**", source_len)) != UINT32_MAX)
			{
				char* filter = tmalloc<char>(pos + 2);
				strncpy(filter, in.sources[i], pos + 1);
				filter[pos + 1] = '\0';
				fill_sources(filter, in.sources[i] + pos + 2, out);
				tfree(filter);
			}
			else if ((pos = str::find(in.sources[i], "*", source_len)) != UINT32_MAX)
			{
				char* filter = tmalloc<char>(pos + 2);
				strncpy(filter, in.sources[i], pos + 1);
				filter[pos + 1] = '\0';
				fs::list_files_res files =
					fs::list_files(filter, in.sources[i] + pos + 1);
				if (out.sources_capacity < out.sources_size + files.size)
				{
					if (!out.sources_capacity)
						out.sources_capacity = 1;
					else
						out.sources_capacity *= 2;
					lua::output::source* new_sources = trealloc(
						out.sources, out.sources_capacity * sizeof(lua::output::source));
					out.sources = new_sources;
				}
				for (uint32_t i {0}; i < files.size; ++i)
					out.sources[out.sources_size + i].file = files.files[i];
				out.sources_size += files.size;
				if (files.size)
					tfree(files.files);
			}
			else if (fs::file_exists(in.sources[i]))
			{
				if (out.sources_capacity < out.sources_size + 1)
				{
					if (!out.sources_capacity)
						out.sources_capacity = 1;
					else
						out.sources_capacity *= 2;
					lua::output::source* new_sources = trealloc(
						out.sources, out.sources_capacity * sizeof(lua::output::source));
					out.sources = new_sources;
				}
				out.sources[out.sources_size].file = in.sources[i];
				++out.sources_size;
				in.sources[i] = nullptr;
			}
		}

		if (in.compile_options_size)
		{
			uint32_t compile_options_str_size {0};
			for (uint32_t i {0}; i < in.compile_options_size; ++i)
				compile_options_str_size += strlen(in.compile_options[i]) + 1;

			char*    compile_options = tmalloc<char>(compile_options_str_size);
			uint32_t pos {0};
			for (uint32_t i {0}; i < in.compile_options_size; ++i)
			{
				uint32_t len {static_cast<uint32_t>(strlen(in.compile_options[i]))};
				strncpy(compile_options + pos, in.compile_options[i], len);
				pos += len;
				if (i < in.compile_options_size - 1)
				{
					strncpy(compile_options + pos, " ", 1);
					pos += 1;
				}
			}
			compile_options[compile_options_str_size - 1] = '\0';
			out.compile_options = compile_options;
		}

		if (in.link_options_size)
		{
			uint32_t link_options_str_size {0};
			for (uint32_t i {0}; i < in.link_options_size; ++i)
				link_options_str_size += strlen(in.link_options[i]) + 1;

			char*    link_options = tmalloc<char>(link_options_str_size);
			uint32_t pos {0};
			for (uint32_t i {0}; i < in.link_options_size; ++i)
			{
				uint32_t len {static_cast<uint32_t>(strlen(in.link_options[i]))};
				strncpy(link_options + pos, in.link_options[i], len);
				pos += len;
				if (i < in.link_options_size - 1)
				{
					strncpy(link_options + pos, " ", 1);
					pos += 1;
				}
			}
			link_options[link_options_str_size - 1] = '\0';
			out.link_options = link_options;
		}

		if (in.deps)
		{
			out.deps = in.deps;
			out.deps_size = in.deps_size;
			in.deps = nullptr;
			in.deps_size = 0;
		}

		lua::free_input(in);
		lua::dump_output(L, out);
		lua::free_output(out);

		return 1;
	}
} // namespace prj
