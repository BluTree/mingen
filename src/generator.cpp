#include "generator.hpp"

#include "fs.hpp"
#include "lua_env.hpp"
#include "mem.hpp"
#include "state.hpp"
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
		char* unesc_str(char const* str)
		{
			uint32_t len = strlen(str);
			char*    unescaped = tmalloc<char>(len * 2);
			uint32_t pos = 0;
			for (uint32_t i {0}; i < len; ++i)
			{
				switch (str[i])
				{
					case '\a':
						strcpy(unescaped + pos, "\\a");
						pos += 2;
						break;
					case '\b':
						strcpy(unescaped + pos, "\\b");
						pos += 2;
						break;
					case '\f':
						strcpy(unescaped + pos, "\\f");
						pos += 2;
						break;
					case '\n':
						strcpy(unescaped + pos, "\\n");
						pos += 2;
						break;
					case '\r':
						strcpy(unescaped + pos, "\\r");
						pos += 2;
						break;
					case '\t':
						strcpy(unescaped + pos, "\\t");
						pos += 2;
						break;
					case '\v':
						strcpy(unescaped + pos, "\\v");
						pos += 2;
						break;
					case '\\':
						strcpy(unescaped + pos, "\\\\");
						pos += 2;
						break;
					case '\'':
						strcpy(unescaped + pos, "\\'");
						pos += 2;
						break;
					case '\"':
						strcpy(unescaped + pos, "\\\"");
						pos += 2;
						break;
					case '\?':
						strcpy(unescaped + pos, "\\\?");
						pos += 2;
						break;
					default:
						unescaped[pos] = str[i];
						++pos;
						break;
				}
			}
			unescaped[pos] = '\0';
			return unescaped;
		}

		void generate_db(lua::output* outs, uint32_t outs_size)
		{
			FILE* file = fopen("build/compile_commands.json", "w");
			char* cwd = fs::get_cwd();
			char* unesc_cwd = unesc_str(cwd);
			tfree(cwd);
			fwrite("[\n", 1, 2, file);
			for (uint32_t i {0}; i < outs_size; ++i)
			{
				for (uint32_t j {0}; j < outs[i].sources_size; ++j)
				{
					fwrite("	{\n", 1, 3, file);
#ifdef _WIN32
					fprintf(file, "		\"directory\": \"%s\\\\build\",\n", unesc_cwd);
#elif defined(__linux__)
					fprintf(file, "		\"directory\": \"%s/build\",\n", unesc_cwd);
#endif
					char* unesc_options =
						unesc_str(outs[i].sources[j].compile_options
					                  ? outs[i].sources[j].compile_options
					                  : outs[i].compile_options);
					fprintf(file, "		\"command\": \"clang++ %s\",\n", unesc_options);
					tfree(unesc_options);
					fprintf(file, "		\"file\": \"../%s\"\n", outs[i].sources[j].file);
					if (i == outs_size - 1 && j == outs[i].sources_size - 1)
						fwrite("	}\n", 1, 3, file);
					else
						fwrite("	},\n", 1, 4, file);
				}
			}
			fwrite("]", 1, 1, file);

			tfree(unesc_cwd);
			fclose(file);
		}

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
					objs[i] = tmalloc<char>(file_ext - file_start + 3);
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

						for (uint32_t j {0}; j < out.deps[i].sources_size; ++j)
							fprintf(file, "obj/%s/%s ", out.deps[i].name, objs[j]);

						for (uint32_t j {0}; j < out.deps[i].sources_size; ++j)
							tfree(objs[j]);
						tfree(objs);
						break;
					}
					case lua::project_type::shared_library:
					{
						fprintf(file, "lib/%s.a ", out.deps[i].name);
						break;
					}
					case lua::project_type::static_library:
					{
						fprintf(file, "lib/%s.a ", out.deps[i].name);
						break;
					}
					case lua::project_type::executable: [[fallthrough]];
					default: break;
				}

				write_deps(out.deps[i], file);
			}
		}

		char* get_ninja_cwd()
		{
			char* cwd = fs::get_cwd();
#ifdef _WIN32
			char* ninja_cwd = tmalloc<char>(strlen(cwd) + 2);
			ninja_cwd[0] = cwd[0];
			ninja_cwd[1] = '$';
			strcpy(ninja_cwd + 2, cwd + 1);
			tfree(cwd);
			return ninja_cwd;
#elif defined(__linux__)
			return cwd;
#endif
		}

		void generate(lua::output const& out, FILE* file)
		{
			char*  cwd = get_ninja_cwd();
			char** objs = collect_objs(out);
			for (uint32_t i {0}; i < out.sources_size; ++i)
			{
				fprintf(file, "build obj/%s/%s: cxx %s/%s\n", out.name, objs[i], cwd,
				        out.sources[i].file);
				fprintf(file, "    cxxflags = %s\n",
				        out.sources[i].compile_options ? out.sources[i].compile_options
				                                       : out.compile_options);
			}

			switch (out.type)
			{
				case lua::project_type::executable:
				{
#ifdef _WIN32
					fprintf(file, "build bin/%s.exe: link ", out.name);
#elif defined(__linux__)
					fprintf(file, "build bin/%s: link ", out.name);
#endif
					for (uint32_t i {0}; i < out.sources_size; ++i)
						fprintf(file, "obj/%s/%s ", out.name, objs[i]);

					write_deps(out, file);
					fseek(file, -1, SEEK_CUR);
					if (out.link_options)
						fprintf(file, "\n    lflags = %s\n\n", out.link_options);
					else
						fwrite("\n\n", 1, 2, file);
#ifdef _WIN32
					fprintf(file, "build %s: phony bin/%s.exe\n\n", out.name, out.name);
#elif defined(__linux__)
					fprintf(file, "build %s: phony bin/%s\n\n", out.name, out.name);
#endif
					break;
				}
				// TODO Verify implementation, to put static export library in lib, not
				// bin
				case lua::project_type::shared_library:
				{
#ifdef _WIN32
					fprintf(file, "build bin/%s.dll: link ", out.name);
#elif defined(__linux__)
					fprintf(file, "build bin/%s.so: link ", out.name);
#endif
					for (uint32_t i {0}; i < out.sources_size; ++i)
						fprintf(file, "obj/%s ", objs[i]);

					write_deps(out, file);
					fseek(file, -1, SEEK_CUR);

					if (out.link_options)
						fprintf(file, "\n    lflags = %s\n\n", out.link_options);
					else
						fwrite("\n\n", 1, 2, file);
					fprintf(file, "build %s: phony bin/%s.a\n\n", out.name, out.name);
					break;
				}
				case lua::project_type::static_library:
				{
					fprintf(file, "build lib/%s.a: lib ", out.name);
					for (uint32_t i {0}; i < out.sources_size; ++i)
						fprintf(file, "obj/%s/%s ", out.name, objs[i]);

					write_deps(out, file);
					fseek(file, -1, SEEK_CUR);
					fwrite("\n    lflags = rscu\n\n", 1, 19, file);

					fprintf(file, "build %s: phony lib/%s.a\n\n", out.name, out.name);
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
			tfree(cwd);
		}
	} // namespace

	int32_t ninja_generator(lua_State* L)
	{
		int32_t len = lua_rawlen(L, 1);

		luaL_argcheck(L, len > 0, 1, "generate must not be empty");

		// TODO create fs::open_file / fs::close_file
		// TODO allow customizing output directory

		if (!fs::dir_exists("build/"))
			fs::create_dir("build/");

		FILE* file = fopen("build/build.ninja", "w");

		if (!file)
			luaL_error(L, "failed to open file for write");

		// create rules
		constexpr char rules[] =
			R"(rule cxx
    description = Compiling ${in}
    deps = gcc
    depfile = ${out}.d
    command = clang++ -fdiagnostics-absolute-paths -fcolor-diagnostics -fansi-escape-codes ${cxxflags} -MMD -MF ${out}.d -c ${in} -o ${out}

rule lib
    description = Creating ${out}
    command = llvm-ar ${lflags} ${out} ${in}

rule link
    description = Creating ${out}
    command = clang++ ${lflags} ${in} -o ${out}

)";

		fwrite(rules, 1, sizeof(rules) - 1, file);

		lua::output* outputs = tmalloc<lua::output>(len);
		uint32_t     outputs_size = 0;
		uint32_t     outputs_capacity = len;

		// TODO detect correctly deps written
		for (uint32_t i {0}; i < len; ++i)
		{
			lua_rawgeti(L, 1, 1);
			lua::output out = lua::parse_output(L);
			for (uint32_t j {0}; j < out.deps_size; ++j)
			{
				bool write {true};
				for (uint32_t k {0}; k < outputs_size; ++k)
				{
					if (strcmp(outputs[k].name, out.deps[j].name) == 0)
					{
						write = false;
						break;
					}
				}

				if (write)
				{
					if (outputs_capacity == outputs_size)
					{
						outputs = trealloc(outputs, outputs_capacity * 2);
						outputs_capacity *= 2;
					}
					outputs[outputs_size] = out.deps[j];
					++outputs_size;
				}
			}

			if (outputs_capacity == outputs_size)
			{
				outputs = trealloc(outputs, outputs_capacity * 2);
				outputs_capacity *= 2;
			}
			outputs[outputs_size] = out;
			++outputs_size;

			lua_pop(L, 1);
		}

		if (g.gen_compile_db)
			generate_db(outputs, outputs_size);

		for (uint32_t i {0}; i < outputs_size; ++i)
			generate(outputs[i], file);
		for (uint32_t i {0}; i < outputs_size; ++i)
			lua::free_output(outputs[i]);
		tfree(outputs);
		fclose(file);
		return 0;
	}
} // namespace gen