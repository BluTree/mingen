#pragma once

#include <stdint.h>

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

namespace lua
{
	void create();
	void run_file(char const* filename);

	enum project_type
	{
		custom,
		static_library,
		shared_library,
		executable,
		count
	};

	char const* const project_type_names[] {"custom", "static_library", "shared_library",
	                                        "executable"};

	struct output;

	struct input
	{
		char const*  name;
		project_type type;

		char const** sources;
		uint32_t     sources_size;
		char const** compile_options;
		uint32_t     compile_options_size;

		output*  deps;
		uint32_t deps_size;

		char const** link_options;
		uint32_t     link_options_size;
	};

	struct output
	{
		struct source
		{
			char const* file;
			// Empty by default, but can be written manually in projects files
			char const* compile_options;
		};

		char const* name;

		source*  sources;
		uint32_t sources_capacity;
		uint32_t sources_size;

		char const* compile_options;
		char const* link_options;
	};

	input parse_input(lua_State* L);
	void  free_input(input const& in);

	void   dump_output(lua_State* L, output const& out);
	output parse_output(lua_State* L);
	void   free_output(output const& out);
} // namespace lua