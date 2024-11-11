#include "generator.hpp"

#include "lua_env.hpp"
#include "mem.hpp"
#include "string.hpp"

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

#include <malloc.h>
#include <stdio.h>
#include <string.h>

namespace gen
{
	namespace
	{
		char** collect_objs(lua::output const& out)
		{
			char** objs = tmalloc<char*>(out.sources_size);
			for (uint32_t i {0}; i < out.sources_size; ++i)
			{
				uint32_t file_start {str::rfind(out.sources[i].file, "/")};
				if (file_start != UINT32_MAX)
					++file_start;
				else
					file_start = 0;
				uint32_t file_ext {str::rfind(out.sources[i].file, ".")};
				if (file_ext != UINT32_MAX)
				{
					objs[i] = tmalloc<char>(file_ext - file_start + 2);
					strncpy(objs[i], out.sources[i].file + file_start,
					        file_ext - file_start);
					strcpy(objs[i] + file_ext - file_start, ".o");
				}
				else
				{
					objs[i] = tmalloc<char>(strlen(out.sources[i].file) - file_start + 3);
					strcpy(objs[i], out.sources[i].file + file_start);
					strcpy(objs[i] + strlen(out.sources[i].file) - file_start, ".o");
				}
			}

			return objs;
		}

		void write_deps(lua::output const& out, FILE* file)
		{
			for (uint32_t i {0}; i < out.deps_size; ++i)
			{
				switch (out.deps[i].type)
				{
					case lua::project_type::sources:
					{
						char** objs = collect_objs(out.deps[i]);

						for (uint32_t i {0}; i < out.deps[i].sources_size; ++i)
							fprintf(file, "build/obj/%s ", objs[i]);

						for (uint32_t i {0}; i < out.deps[i].sources_size; ++i)
							tfree(objs[i]);
						tfree(objs);
						break;
					}
					case lua::project_type::shared_library:
					{
						fprintf(file, "build/lib/%s.a ", out.deps[i].name);
						break;
					}
					case lua::project_type::static_library:
					{
						fprintf(file, "build/lib/%s.a ", out.deps[i].name);
						break;
					}
					case lua::project_type::executable: [[fallthrough]];
					default: break;
				}

				write_deps(out.deps[i], file);
			}
		}

		void generate(lua::output const& out, FILE* file)
		{
			for (uint32_t i {0}; i < out.deps_size; ++i)
				generate(out.deps[i], file);

			char** objs = collect_objs(out);
			for (uint32_t i {0}; i < out.sources_size; ++i)
			{
				fprintf(file, "build build/obj/%s: cxx %s\n", objs[i],
				        out.sources[i].file);
				fprintf(file, "    cxxflags = %s\n",
				        out.sources[i].compile_options ? out.sources[i].compile_options
				                                       : out.compile_options);
			}

			switch (out.type)
			{
				case lua::project_type::executable:
				{
					fprintf(file, "build build/bin/%s.exe: link ", out.name);
					for (uint32_t i {0}; i < out.sources_size; ++i)
						fprintf(file, "build/obj/%s ", objs[i]);

					write_deps(out, file);
					fseek(file, -1, SEEK_CUR);
					if (out.link_options)
						fprintf(file, "\n    lflags = %s\n\n", out.link_options);
					else
						fwrite("\n\n", 1, 2, file);
					fprintf(file, "build %s: phony build/bin/%s.exe\n\n", out.name,
					        out.name);
					break;
				}
				case lua::project_type::shared_library:
				{
					fprintf(file, "build build/bin/%s.dll: link ", out.name);
					for (uint32_t i {0}; i < out.sources_size; ++i)
						fprintf(file, "build/obj/%s ", objs[i]);

					write_deps(out, file);
					fseek(file, -1, SEEK_CUR);

					if (out.link_options)
						fprintf(file, "\n    lflags = %s\n\n", out.link_options);
					else
						fwrite("\n\n", 1, 2, file);
					fprintf(file, "build %s: phony build/bin/%s.a\n\n", out.name,
					        out.name);
					break;
				}
				case lua::project_type::static_library:
				{
					fprintf(file, "build build/lib/%s.a: lib ", out.name);
					for (uint32_t i {0}; i < out.sources_size; ++i)
						fprintf(file, "build/obj/%s ", objs[i]);

					write_deps(out, file);
					fseek(file, -1, SEEK_CUR);
					fwrite("\n    lflags = scu\n\n", 1, 19, file);

					fprintf(file, "build %s: phony build/lib/%s.a\n\n", out.name,
					        out.name);
					break;
				}
				case lua::project_type::sources: [[fallthrough]];
				default:
				{
					fwrite("\n", 1, 1, file);
					break;
				}
			}

			for (uint32_t i {0}; i < out.sources_size; ++i)
				tfree(objs[i]);
			tfree(objs);
		}
	} // namespace

	int32_t ninja_generator(lua_State* L)
	{
		int32_t len = lua_rawlen(L, 1);

		luaL_argcheck(L, len > 0, 1, "generate must not be empty");

		// TODO create fs::open_file / fs::close_file
		// TODO allow customizing output directory
		FILE* file = fopen("build/build.ninja", "w");

		if (!file)
			luaL_error(L, "failed to open file for write");

		// create rules
		constexpr char rules[] =
			R"(builddir=build/
rule cxx
    description = Compiling ${in}
    deps = gcc
    depfile = ${out}.d
    command = clang++ ${flags} -MMD -MF ${out}.d -c ${in} -o ${out}

rule lib
    description = Creating ${out}
    command = llvm-ar ${flags} ${in} -o ${out}

rule link
    description = Creating ${out}
    command = clang++ ${flags} ${in} -o ${out}

)";

		fwrite(rules, 1, sizeof(rules) - 1, file);

		for (uint32_t i {0}; i < len; ++i)
		{
			lua_rawgeti(L, 1, 1);
			lua::output out = lua::parse_output(L);

			generate(out, file);
			lua_pop(L, 1);
		}
		fclose(file);
		return 0;
	}
} // namespace gen