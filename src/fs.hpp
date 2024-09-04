#pragma once

#include <stdint.h>

namespace fs
{
	struct list_dirs_res
	{
		char**   dirs;
		uint32_t size;
	};

	struct list_files_res
	{
		char**   files;
		uint32_t size;
	};

	/// @brief Lists directories contained in `dir`.
	/// @param dir Non null, '\0' terminated string indicating the directory to read. A
	/// null or not '\0' terminated string results in undefined behavior
	/// @return list_dirs_res List of directories contained in `dir`. If no directories
	/// are present, `dirs = nullptr` and `size = 0`.
	list_dirs_res list_dirs(char const* dir_filter);

	/// @brief Lists files contained in `dir`.
	/// @param dir Non null, '\0' terminated string indicating the directory to read. A
	/// null or not '\0' terminated string results in undefined behavior
	/// @return list_dirs_res List of files contained in `dir`. If no files are present,
	/// `files = nullptr` and `size = 0`.
	list_files_res list_files(char const* dir_filter, char const* file_filter);

	bool file_exists(char const* file); // TODO
}; // namespace fs