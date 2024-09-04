#include "fs.hpp"

#include <win32/file.h>
#include <win32/misc.h>

#include "string.hpp"

namespace fs
{
	list_dirs_res list_dirs(char const* dir_filter)
	{
		uint32_t dirs_count {0};
		STACK_CHAR_TO_WCHAR(dir_filter, wdir)

		WIN32_FIND_DATAW entry_data;

		HANDLE entry = FindFirstFileExW(wdir, FindExInfoBasic, &entry_data,
		                                FindExSearchNameMatch, nullptr, 0);

		if (!entry)
			return {nullptr, 0};

		do
		{
			if (entry_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
			    wcscmp(entry_data.cFileName, L".") != 0 &&
			    wcscmp(entry_data.cFileName, L"..") != 0)
			{
				++dirs_count;
			}
		}
		while (FindNextFileW(entry, &entry_data) != 0);

		if (!dirs_count)
			return {nullptr, 0};

		char**   dirs {new char*[dirs_count]};
		uint32_t i {0};
		entry = FindFirstFileExW(wdir, FindExInfoBasic, &entry_data,
		                         FindExSearchNameMatch, nullptr, 0);
		do
		{
			if (entry_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
			    wcscmp(entry_data.cFileName, L".") != 0 &&
			    wcscmp(entry_data.cFileName, L"..") != 0)
			{
				STACK_WCHAR_TO_CHAR(entry_data.cFileName, dn)
				dirs[i] = new char[strlen(dir_filter) - 1 + strlen(dn) + 1];
				strncpy(dirs[i], dir_filter, strlen(dir_filter) - 1);
				strcpy(dirs[i] + strlen(dir_filter) - 1, dn);
				++i;
			}
		}
		while (FindNextFileW(entry, &entry_data) != 0);
		return {dirs, dirs_count};
	}

	list_files_res list_files(char const* dir_filter, char const* file_filter)
	{
		uint32_t files_count {0};
		STACK_CHAR_TO_WCHAR(dir_filter, wdir)

		WIN32_FIND_DATAW entry_data;
		HANDLE           entry = FindFirstFileExW(wdir, FindExInfoBasic, &entry_data,
		                                          FindExSearchNameMatch, nullptr, 0);
		if (!entry)
			return {nullptr, 0};

		do
		{
			STACK_WCHAR_TO_CHAR(entry_data.cFileName, fn)
			if (!(entry_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			    str::ends_with(fn, file_filter))
			{
				++files_count;
			}
		}
		while (FindNextFileW(entry, &entry_data) != 0);

		if (!files_count)
			return {nullptr, 0};

		char**   files {new char*[files_count]};
		uint32_t i = 0;
		entry = FindFirstFileExW(wdir, FindExInfoBasic, &entry_data,
		                         FindExSearchNameMatch, nullptr, 0);
		do
		{
			STACK_WCHAR_TO_CHAR(entry_data.cFileName, fn)
			if (!(entry_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			    str::ends_with(fn, file_filter))
			{
				files[i] = new char[strlen(dir_filter) - 1 + strlen(fn) + 1];
				strncpy(files[i], dir_filter, strlen(dir_filter) - 1);
				strcpy(files[i] + strlen(dir_filter) - 1, fn);
				++i;
			}
		}
		while (FindNextFileW(entry, &entry_data) != 0);
		return {files, files_count};
	}
} // namespace fs