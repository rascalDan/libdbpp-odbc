#include <stdarg.h>
#include <stdio.h>
#include "ustring.h"

ODBC::String::String()
{
}
ODBC::String::String(std::basic_string<unsigned char> s) :
	std::basic_string<unsigned char>(s)
{
}
ODBC::String::String(std::basic_string<char> s) :
	std::basic_string<unsigned char>((unsigned char *)s.c_str())
{
}
ODBC::String::String(const char * s) :
	std::basic_string<unsigned char>((unsigned char *)s)
{
}
ODBC::String::String(const unsigned char * s) :
	std::basic_string<unsigned char>(s)
{
}

ODBC::String::operator unsigned char * () const
{
	return (unsigned char*)c_str();
}

ODBC::String
ODBC::String::Format(const char * fmt, ...)
{
	char * buf;
	va_list va;
	va_start(va, fmt);
	if (vasprintf(&buf, fmt, va)) {
		buf = NULL;
	}
	va_end(va);
	return buf;
}

