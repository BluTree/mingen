#include "fs.hpp"

#ifdef _WIN32
#include <win32/file.h>
#include <win32/io.h>
#include <win32/misc.h>
#elif defined(__linux__)
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "mem.hpp"
#include "string.hpp"

namespace fs
{
#ifdef _WIN32
	list_dirs_res list_dirs(char const* dir_filter)
	{
		uint32_t dirs_count {0};
		STACK_CHAR_TO_WCHAR(dir_filter, wdir)

		WIN32_FIND_DATAW entry_data;

		HANDLE entry = FindFirstFileExW(wdir, FindExInfoBasic, &entry_data,
		                                FindExSearchNameMatch, nullptr, 0);

		if (entry == INVALID_HANDLE_VALUE)
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

		char**   dirs {tmalloc<char*>(dirs_count)};
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
				dirs[i] = tmalloc<char>(strlen(dir_filter) - 1 + strlen(dn) + 1);
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
		if (entry == INVALID_HANDLE_VALUE)
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

		char**   files {tmalloc<char*>(files_count)};
		uint32_t i = 0;
		entry = FindFirstFileExW(wdir, FindExInfoBasic, &entry_data,
		                         FindExSearchNameMatch, nullptr, 0);
		do
		{
			STACK_WCHAR_TO_CHAR(entry_data.cFileName, fn)
			if (!(entry_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			    str::ends_with(fn, file_filter))
			{
				files[i] = tmalloc<char>(strlen(dir_filter) - 1 + strlen(fn) + 1);
				strncpy(files[i], dir_filter, strlen(dir_filter) - 1);
				strcpy(files[i] + strlen(dir_filter) - 1, fn);
				++i;
			}
		}
		while (FindNextFileW(entry, &entry_data) != 0);
		return {files, files_count};
	}

	bool file_exists(char const* file)
	{
		STACK_CHAR_TO_WCHAR(file, wfile);
		uint32_t attr = GetFileAttributesW(wfile);

		return (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool dir_exists(char const* dir)
	{
		STACK_CHAR_TO_WCHAR(dir, wdir);
		uint32_t attr = GetFileAttributesW(wdir);

		return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
	}

	char* get_cwd()
	{
		wchar_t  wcwd[512];
		uint32_t len = GetCurrentDirectoryW(512, wcwd);
		return wchar_to_char(wcwd);
	}

	void set_cwd(char const* cwd)
	{
		STACK_CHAR_TO_WCHAR(cwd, wcwd);
		SetCurrentDirectoryW(wcwd);
	}

	bool is_absolute(char const* path)
	{
		return path[1] == ':';
	}

	bool create_dir(char const* path)
	{
		STACK_CHAR_TO_WCHAR(path, wpath);
		return CreateDirectoryW(wpath);
	}
#elif defined(__linux__)
	list_dirs_res list_dirs(char const* dir_filter)
	{
		uint32_t dirs_count {0};
		DIR*     dir_p = opendir(dir_filter);

		if (!dir_p)
			return {nullptr, 0};

		dirent* entry {nullptr};
		while ((entry = readdir(dir_p)))
			if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 &&
			    strcmp(entry->d_name, "..") != 0)
				++dirs_count;

		closedir(dir_p);

		if (!dirs_count)
			return {nullptr, 0};

		char**   dirs {tmalloc<char*>(dirs_count)};
		uint32_t i {0};
		dir_p = opendir(dir_filter);
		while ((entry = readdir(dir_p)))
		{
			if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 &&
			    strcmp(entry->d_name, "..") != 0)
			{
				dirs[i] = tmalloc<char>(strlen(dir_filter) + strlen(entry->d_name));
				strncpy(dirs[i], dir_filter, strlen(dir_filter));
				strcpy(dirs[i] + strlen(dir_filter), entry->d_name);
				++i;
			}
		}

		return {dirs, dirs_count};
	}

	list_files_res list_files(char const* dir_filter, char const* file_filter)
	{
		uint32_t files_count {0};
		DIR*     dir_p = opendir(dir_filter);

		if (!dir_p)
			return {nullptr, 0};

		dirent* entry {nullptr};
		while ((entry = readdir(dir_p)))
			if (entry->d_type == DT_REG && str::ends_with(entry->d_name, file_filter))
				++files_count;

		closedir(dir_p);

		if (!files_count)
			return {nullptr, 0};

		char**   files {tmalloc<char*>(files_count)};
		uint32_t i {0};
		dir_p = opendir(dir_filter);
		while ((entry = readdir(dir_p)))
		{
			if (entry->d_type == DT_REG && str::ends_with(entry->d_name, file_filter))
			{
				files[i] = tmalloc<char>(strlen(dir_filter) + strlen(entry->d_name));
				strncpy(files[i], dir_filter, strlen(dir_filter));
				strcpy(files[i] + strlen(dir_filter), entry->d_name);
				++i;
			}
		}

		return {files, files_count};
	}

	bool file_exists(char const* file)
	{
		return access(file, F_OK) == 0;
	}

	bool dir_exists(char const* dir)
	{
		struct stat res;
		int32_t     err = stat(dir, &res);

		return err == 0 && S_ISDIR(res.st_mode);
	}

	char* get_cwd()
	{
		return get_current_dir_name();
	}

	void set_cwd(char const* cwd)
	{
		chdir(cwd);
	}

	bool is_absolute(char const* path)
	{
		return str::starts_with(path, "/");
	}

	bool create_dir(char const* path)
	{
		return mkdir(path, 0755) == 0;
	}
#else
#error "Unsupported platform"
#endif
} // namespace fs