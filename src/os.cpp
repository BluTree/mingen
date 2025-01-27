#include "os.hpp"

#include <stdint.h>

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

#include <win32/io.h>
#include <win32/process.h>
#include <win32/threads.h>

#include "fs.hpp"
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

		lua_Debug ar;
		lua_getstack(L, 1, &ar);
		lua_getinfo(L, "Sl", &ar);
		// printf("ar = %s\n", ar.source);
		char*    base_path = nullptr;
		uint32_t base_path_size = 0;
		if (str::starts_with(ar.short_src, "./") || str::starts_with(ar.short_src, ".\\"))
		{
			base_path_size = str::rfind(ar.short_src, "/");
			if (base_path_size != UINT32_MAX)
			{
				base_path = ar.short_src + 2;
				base_path_size -= 1;
			}
			else
				base_path_size = 0;
		}

		wchar_t* wworking_dir = nullptr;
		wchar_t* wcmd = nullptr;

		if (top == 2)
		{
			char const* working_dir = lua_tostring(L, 1);
			uint32_t    working_dir_size = strlen(working_dir);
			if (base_path_size && !fs::is_absolute(working_dir))
			{
				char* new_working_dir =
					tmalloc<char>(base_path_size + working_dir_size + 1);
				strncpy(new_working_dir, base_path, base_path_size);
				strncpy(new_working_dir + base_path_size, working_dir, working_dir_size);
				new_working_dir[base_path_size + working_dir_size] = '\0';

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
		return 1;
	}

	int copy_file(lua_State* L)
	{
		luaL_argcheck(L, lua_isstring(L, 1), 1, "'string' expected");
		luaL_argcheck(L, lua_isstring(L, 2), 2, "'string' expected");

		lua_Debug ar;
		lua_getstack(L, 1, &ar);
		lua_getinfo(L, "Sl", &ar);
		// printf("ar = %s\n", ar.source);
		char*    base_path = nullptr;
		uint32_t base_path_size = 0;
		if (str::starts_with(ar.short_src, "./") || str::starts_with(ar.short_src, ".\\"))
		{
			base_path_size = str::rfind(ar.short_src, "/");
			if (base_path_size != UINT32_MAX)
			{
				base_path = ar.short_src + 2;
				base_path_size -= 1;
			}
			else
				base_path_size = 0;
		}

		char const* src_path = lua_tostring(L, 1);
		char const* dst_path = lua_tostring(L, 2);
		char*       resolved_src_path = nullptr;
		char*       resolved_dst_path = nullptr;

		if (base_path_size && !fs::is_absolute(src_path))
		{
			resolved_src_path = tmalloc<char>(base_path_size + strlen(src_path) + 1);
			strncpy(resolved_src_path, base_path, base_path_size);
			strncpy(resolved_src_path + base_path_size, src_path, strlen(src_path));
			resolved_src_path[base_path_size + strlen(src_path)] = '\0';
		}
		else
		{
			resolved_src_path = tmalloc<char>(strlen(src_path) + 1);
			strcpy(resolved_src_path, src_path);
		}

		if (base_path_size && !fs::is_absolute(dst_path))
		{
			resolved_dst_path = tmalloc<char>(base_path_size + strlen(dst_path) + 1);
			strncpy(resolved_dst_path, base_path, base_path_size);
			strncpy(resolved_dst_path + base_path_size, dst_path, strlen(dst_path));
			resolved_dst_path[base_path_size + strlen(dst_path)] = '\0';
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
