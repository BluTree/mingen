#include "fs.hpp"
#include "lua_env.hpp"
#include "project.hpp"
#include "state.hpp"
#include "string.hpp"

#include <stdio.h>

mingen_state g {};

#define ITALIC "\x1b[3m"
#define DEFAULT "\x1b[0m"

void help()
{
	// clang-format off
	char const* help_str =
"mingen: MINimal GENerator.\n"
"Generates ninja-build files from a lua script.\n"
"\n"
"Usage: mingen [-d dir] [-f file]\n"
"\n"
"    -d, --directory " ITALIC "dir" DEFAULT "\n"
"        Changes to directory " ITALIC "dir" DEFAULT " before running the script.\n"
"\n"
"    -f, --file " ITALIC "file" DEFAULT "\n"
"        Runs " ITALIC "file" DEFAULT " for generating. If no file is specified, defaults to running mingen.lua\n"
"\n"
"    -c, --configuration " ITALIC "config" DEFAULT "\n"
"        Generates build files for " ITALIC "config" DEFAULT ". If no configuration is specified, defaults to the first declared configuration.\n"
"\n"
"    -f, --flags " ITALIC "flags..." DEFAULT " (Soon)\n"
"        Set the flags to be used when running the script. The flags are to be handled by the script maintainer for its own purpose."
	   " mingen does nothing with the flags itself.\n"
"\n"
"	--compile-db\n"
"		Generates a JSON Compilation Database with the ninja file\n"
"\n"
"\n"
"Miscellaneous: \n"
"\n"
"    cp " ITALIC "src dst" DEFAULT "\n"
"        Copies file " ITALIC "src " DEFAULT "to " ITALIC "dst" DEFAULT ".\n"
"        This is meant to be used internally for copies in the build process on Windows, since the system tools provided are horrible.\n";
	// clang-format on
	printf("%s", help_str);
}

int main(int argc, char** argv)
{
	char const* file = "mingen.lua";
	char const* dir = nullptr;
	bool        print_help = false;
	for (uint32_t i {1}; i < argc; ++i)
	{
		if (str::starts_with(argv[i], "-d"))
		{
			if (strlen(argv[i]) > 2 && argv[i][2] == '=')
				dir = argv[i] + 3;
			else
			{
				if (i == argc - 1)
				{
					printf("Invalid arugment '-d'\n");
					print_help = true;
					break;
				}
				dir = argv[++i];
			}
		}
		else if (str::starts_with(argv[i], "--directory"))
		{
			if (strlen(argv[i]) > 11 && argv[i][11] == '=')
				dir = argv[i] + 12;
			else
			{
				if (i == argc - 1)
				{
					printf("Invalid arugment '--directory'\n");
					print_help = true;
					break;
				}
				dir = argv[++i];
			}
		}
		else if (str::starts_with(argv[i], "-f"))
		{
			if (strlen(argv[i]) > 2 && argv[i][2] == '=')
				file = argv[i] + 3;
			else
			{
				if (i == argc - 1)
				{
					printf("Invalid arugment '-f'");
					print_help = true;
					break;
				}
				file = argv[++i];
			}
		}
		else if (str::starts_with(argv[i], "--file"))
		{
			if (strlen(argv[i]) > 6 && argv[i][6] == '=')
				file = argv[i] + 7;
			else
			{
				if (i == argc - 1)
				{
					printf("Invalid arugment '--file'\n");
					print_help = true;
					break;
				}
				file = argv[++i];
			}
		}
		else if (str::starts_with(argv[i], "-c"))
		{
			if (strlen(argv[i]) > 6 && argv[i][2] == '=')
				g.config_param = argv[i] + 3;
			else
			{
				if (i == argc - 1)
				{
					printf("Invalid arugment '-c'\n");
					print_help = true;
					break;
				}
				g.config_param = argv[++i];
			}
		}
		else if (str::starts_with(argv[i], "--configuration"))
		{
			if (strlen(argv[i]) > 6 && argv[i][15] == '=')
				g.config_param = argv[i] + 16;
			else
			{
				if (i == argc - 1)
				{
					printf("Invalid arugment '-c'\n");
					print_help = true;
					break;
				}
				g.config_param = argv[++i];
			}
		}
		else if (str::starts_with(argv[i], "--compile-db"))
		{
			g.gen_compile_db = true;
		}
		else if (strcmp(argv[i], "cp") == 0)
		{
			if (i > argc - 3)
				fprintf(stderr, "Not enough arguments given");
			char const* src = argv[i + 1];
			char const* dst = argv[i + 2];
			if (!fs::file_exists(src))
				fprintf(stderr, "%s does not exist", src);

			bool res = fs::copy_file(src, dst, true);
			// Needed because file copy also copies the last modification metadata, and
			// ninja relies on it to verify if copy is still needed.
			// TODO verify behaviour on linux, if this is a Windows only thing.
			res &= fs::update_last_write_time(dst);
			return !res;
		}
		else if (str::starts_with(argv[i], "-h") || str::starts_with(argv[i], "--help"))
		{
			help();
			return 0;
		}
	}

	if (print_help)
	{
		putc('\n', stdout);
		help();
		return 1;
	}

	if (dir)
		fs::set_cwd(dir);

	lua::create();
	if (!fs::file_exists(file))
	{
		printf("'%s': file not found\n\n", file);
		help();
		return 1;
	}
	int32_t res = lua::run_file(file);

	lua::destroy();
	return res;
}