#include "string.hpp"

#include "mem.hpp"

#include <string.h>

#ifdef _WIN32
[[nodiscard]] char* wchar_to_char(wchar_t const* wbuf)
{
	size_t wlen = wcslen(wbuf);
	size_t len = 0;
	len = WideCharToMultiByte(CP_UTF8, 0, wbuf, wlen, nullptr, 0, nullptr, nullptr);
	char* buf = tmalloc<char>(len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wbuf, wlen, buf, len + 1, nullptr, nullptr);
	buf[len] = '\0';

	return buf;
}

[[nodiscard]] wchar_t* char_to_wchar(char const* buf)
{
	size_t len = strlen(buf);
	size_t wlen = 0;
	wlen = MultiByteToWideChar(CP_UTF8, 0, buf, len, nullptr, 0);
	wchar_t* wbuf = tmalloc<wchar_t>(wlen + 1);
	MultiByteToWideChar(CP_UTF8, 0, buf, len, wbuf, wlen + 1);
	wbuf[wlen] = L'\0';

	return wbuf;
}

uint32_t char_len(wchar_t const* wbuf)
{
	return WideCharToMultiByte(CP_UTF8, 0, wbuf, wcslen(wbuf), nullptr, 0, nullptr,
	                           nullptr);
}

uint32_t wchar_len(char const* buf)
{
	return MultiByteToWideChar(CP_UTF8, 0, buf, strlen(buf), nullptr, 0);
}
#endif

namespace str
{
	uint32_t find(char const* str, char const* buf, uint32_t str_len, uint32_t buf_len)
	{
		if (str_len == UINT32_MAX)
			str_len = static_cast<uint32_t>(strlen(str));

		if (buf_len == UINT32_MAX)
			buf_len = static_cast<uint32_t>(strlen(buf));

		uint32_t res {UINT32_MAX};
		for (uint32_t i {0}; i < str_len - buf_len + 1; ++i)
		{
			if (str[i] == buf[0] && strncmp(str + i, buf, buf_len) == 0)
			{
				res = i;
				break;
			}
		}

		return res;
	}

	uint32_t rfind(char const* str, char const* buf, uint32_t str_len, uint32_t buf_len)
	{
		if (str_len == UINT32_MAX)
			str_len = static_cast<uint32_t>(strlen(str));

		if (buf_len == UINT32_MAX)
			buf_len = static_cast<uint32_t>(strlen(buf));

		uint32_t res {UINT32_MAX};
		for (uint32_t i {str_len - buf_len + 1}; i > 0; --i)
		{
			if (str[i - 1] == buf[0] && strncmp(str + i - 1, buf, buf_len) == 0)
			{
				res = i - 1;
				break;
			}
		}

		return res;
	}

	bool starts_with(char const* str, char const* buf)
	{
		uint32_t buf_len {static_cast<uint32_t>(strlen(buf))};
		return strncmp(str, buf, buf_len) == 0;
	}

	bool starts_with(char const* str, char const* buf, uint32_t buf_len)
	{
		if (buf_len == UINT32_MAX)
			buf_len = static_cast<uint32_t>(strlen(buf));
		return strncmp(str, buf, buf_len) == 0;
	}

	bool ends_with(char const* str, char const* buf, uint32_t str_len, uint32_t buf_len)
	{
		if (buf_len == UINT32_MAX)
			buf_len = static_cast<uint32_t>(strlen(buf));

		if (str_len == UINT32_MAX)
			str_len = static_cast<uint32_t>(strlen(str));
		return strncmp(str + str_len - buf_len, buf, buf_len) == 0;
	}
} // namespace str