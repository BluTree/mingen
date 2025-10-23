#include "os.hpp"

#include <stdint.h>

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

#ifdef _WIN32
#include <win32/io.h>
#include <win32/process.h>
#include <win32/threads.h>
#elif defined(__linux__)
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "fs.hpp"
#include "lua_env.hpp"
#include "mem.hpp"
#include "string.hpp"

namespace os
{
	int execute(lua_State* L)
	{
		int top = lua_gettop(L);
		luaL_argcheck(L, lua_isstring(L, 1), 1, "'string' expected");
		if (top == 2)
			luaL_argcheck(L, lua_isstring(L, 2), 2, "'string' expected");
#ifdef _WIN32
		wchar_t* wworking_dir = nullptr;
		wchar_t* wcmd = nullptr;

		if (top == 2)
		{
			char const* working_dir = lua_tostring(L, 1);
			uint32_t    working_dir_size = strlen(working_dir);
			if (!fs::is_absolute(working_dir))
			{
				char* new_working_dir =
					lua::resolve_path_from_script(L, working_dir, working_dir_size);

				wworking_dir = char_to_wchar(new_working_dir);
				tfree(new_working_dir);
			}
			else
				wworking_dir = char_to_wchar(working_dir);

			wcmd = char_to_wchar(lua_tostring(L, 2));
		}
		else
			wcmd = char_to_wchar(lua_tostring(L, 1));

		DWORD        return_code = UINT32_MAX;
		STARTUPINFOW info;
		memset(&info, 0, sizeof(STARTUPINFOW));
		PROCESS_INFORMATION handles;
		memset(&handles, 0, sizeof(PROCESS_INFORMATION));
		if (CreateProcessW(nullptr, wcmd, nullptr, nullptr, true, 0, nullptr,
		                   wworking_dir, &info, &handles))
		{
			CloseHandle(handles.hThread);
			WaitForSingleObject(handles.hProcess, INFINITE);
			GetExitCodeProcess(handles.hProcess, &return_code);
		}

		tfree(wworking_dir);
		tfree(wcmd);

		lua_pushinteger(L, return_code);
#elif defined(__linux__)
		char*       working_dir = nullptr;
		char const* cmd = nullptr;

		if (top == 2)
		{
			char const* lua_working_dir = lua_tostring(L, 1);
			uint32_t    working_dir_size = strlen(lua_working_dir);
			if (!fs::is_absolute(lua_working_dir))
			{
				char* new_working_dir =
					lua::resolve_path_from_script(L, lua_working_dir, working_dir_size);

				working_dir = new_working_dir;
			}
			else
			{
				working_dir = tmalloc<char>(working_dir_size + 1);
				strcpy(working_dir, lua_working_dir);
			}

			cmd = lua_tostring(L, 2);
		}
		else
			cmd = lua_tostring(L, 1);

		pid_t pid;

		pid = fork();
		switch (pid)
		{
			case -1:
			{
				lua_pushinteger(L, -1);
				break;
			}

			case 0:
			{
				chdir(working_dir);
				execl("/bin/sh", "sh", "-c", cmd, nullptr);
				break;
			}
		}
		tfree(working_dir);

		int status;
		waitpid(pid, &status, 0);

		if (WIFEXITED(status))
			lua_pushinteger(L, WEXITSTATUS(status));
		else
			lua_pushinteger(L, -1);
#endif
		return 1;
	}

	int copy_file(lua_State* L)
	{
		luaL_argcheck(L, lua_isstring(L, 1), 1, "'string' expected");
		luaL_argcheck(L, lua_isstring(L, 2), 2, "'string' expected");

		char const* src_path = lua_tostring(L, 1);
		char const* dst_path = lua_tostring(L, 2);
		char*       resolved_src_path = nullptr;
		char*       resolved_dst_path = nullptr;

		if (!fs::is_absolute(src_path))
		{
			resolved_src_path = lua::resolve_path_from_script(L, src_path);
		}
		else
		{
			resolved_src_path = tmalloc<char>(strlen(src_path) + 1);
			strcpy(resolved_src_path, src_path);
		}

		if (!fs::is_absolute(dst_path))
		{
			resolved_dst_path = lua::resolve_path_from_script(L, dst_path);
		}
		else
		{
			resolved_dst_path = tmalloc<char>(strlen(dst_path) + 1);
			strcpy(resolved_dst_path, dst_path);
		}

		bool res = fs::copy_file(resolved_src_path, resolved_dst_path, true);

		tfree(resolved_src_path);
		tfree(resolved_dst_path);

		lua_pushboolean(L, res);

		return 1;
	}
} // namespace os
