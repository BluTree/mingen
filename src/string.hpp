#pragma once

#include "macros.hpp"

#include <string.h>
#include <malloc.h>

#include <win32/misc.h>
#include <stdint.h>

#define STACK_WCHAR_TO_CHAR(wbuf, buf) \
	size_t CONCAT(wlen,__LINE__) = wcslen(wbuf);\
 	size_t CONCAT(len,__LINE__) = 0;\
 	CONCAT(len,__LINE__) = WideCharToMultiByte(CP_UTF8, 0, wbuf, CONCAT(wlen,__LINE__), nullptr, 0, nullptr, nullptr);\
 	char* buf = reinterpret_cast<char*>(_alloca(sizeof(char) * (CONCAT(len,__LINE__) + 1)));\
 	WideCharToMultiByte(CP_UTF8, 0, wbuf, CONCAT(wlen,__LINE__), buf, CONCAT(len,__LINE__), nullptr, nullptr);\
	buf[CONCAT(len,__LINE__)] = '\0';

#define STACK_CHAR_TO_WCHAR(buf, wbuf) \
 	size_t CONCAT(len,__LINE__) = strlen(buf);\
	size_t CONCAT(wlen,__LINE__) = 0;\
 	CONCAT(wlen,__LINE__) = MultiByteToWideChar(CP_UTF8, 0, buf, CONCAT(len,__LINE__), nullptr, 0);\
 	wchar_t* wbuf = reinterpret_cast<wchar_t*>(_alloca(sizeof(wchar_t) * (CONCAT(wlen,__LINE__) + 1)));\
 	MultiByteToWideChar(CP_UTF8, 0, buf, CONCAT(len,__LINE__), wbuf, CONCAT(wlen,__LINE__));\
	wbuf[CONCAT(wlen,__LINE__)] = L'\0';

[[nodiscard]] char* wchar_to_char(wchar_t const* wbuf);
[[nodiscard]] wchar_t* char_to_wchar(char const* buf);

uint32_t char_len(const wchar_t* wbuf);
uint32_t wchar_len(const char* buf);

namespace str
{

	uint32_t find(const char* str, const char* buf);
	uint32_t find(const char* str, const char* buf, uint32_t str_len, uint32_t buf_len = UINT32_MAX);
	bool starts_with(const char* str, const char* buf);
	bool starts_with(const char* str, const char* buf, uint32_t buf_len);
	bool ends_with(const char* str, const char* buf);
	bool ends_with(const char* str, const char* buf, uint32_t str_len, uint32_t buf_len = UINT32_MAX);
};