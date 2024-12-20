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
	/// @param dir String indicating the directory filter to read. A null or not '\0'
	/// terminated string results in undefined behavior
	/// @return list_dirs_res List of directories contained in `dir`. If no directories
	/// are present, `dirs = nullptr` and `size = 0`.
	list_dirs_res list_dirs(char const* dir_filter);

	/// @brief Lists files contained in `dir_filter`, filtered by `file_filter`.
	/// @param dir_filter String indicating the directory filter to read. A null or not
	/// '\0' terminated string results in undefined behavior.
	/// @param file_filter Non null, '\0' terminated string indicating the filter to
	/// process files discovered by `dir_filter`. to read. A null or not '\0' terminated
	/// string results in undefined behavior.
	/// @return list_files_res List of files present in `dir_filter`, matching
	/// `file_filter`. If no files match, `files = nullptr` and `size = 0`.
	list_files_res list_files(char const* dir_filter, char const* file_filter);

	/// @brief Verifies `file` presence in the filesystem.
	/// @param file String pointing to the file to verify. The file path is verified as
	/// is, meaning it will use current working directory for relative path.
	/// @return true File exists.
	/// @return false File doesn't exist.
	bool file_exists(char const* file);

	/// @brief Verifies `dir` presence in the filesystem.
	/// @param file String pointing to the directory to verify. The directory path is
	/// verified as is, meaning it will use current working directory for relative path.
	/// @return true Directory exists.
	/// @return false Directory doesn't exist.
	bool dir_exists(char const* dir);

	/// @brief Gets the current working directory.
	/// @return char* The working directory as a full path.
	char* get_cwd();

	/// @brief Sets the current working directory.
	/// @param cwd The current working directory.
	void set_cwd(char const* cwd);

	/// @brief Checks if the path is absolute.
	/// @param path The path to check.
	/// @return true Path is absolute.
	/// @return false Path is relative.
	bool is_absolute(char const* path);

	/// @brief Creates a directory. Doesn't create directories recursively.
	/// @param path Path to the directory to create.
	/// @return true Directory created.
	/// @return false Directory not created.
	bool create_dir(char const* path);
}; // namespace fs