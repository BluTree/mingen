#pragma once

#include <stdint.h>

namespace fs
{
	struct list_dirs_res
	{
		char** dirs;
		uint32_t size;
	};

	struct list_files_res
	{
		char** files;
		uint32_t size;
	};

	/// @brief Lists directories contained in `dir`.
	/// @param dir Non null, '\0' terminated string indicating the directory to read. A null or not '\0' terminated string results in undefined behavior
	/// @return list_dirs_res List of directories contained in `dir`. If no directories are present, `dirs = nullptr` and `size = 0`.
	list_dirs_res list_dirs(const char* dir_filter);

	/// @brief Lists files contained in `dir`.
	/// @param dir Non null, '\0' terminated string indicating the directory to read. A null or not '\0' terminated string results in undefined behavior
	/// @return list_dirs_res List of files contained in `dir`. If no files are present, `files = nullptr` and `size = 0`.
	list_files_res list_files(const char* dir_filter, const char* file_filter);

	bool file_exists(const char* file); // TODO
};