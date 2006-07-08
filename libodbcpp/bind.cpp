#include <sqlext.h>
#include "command.h"
#include "param.h"

ODBC::BindBase::BindBase() :
	bindSize(0),
	bindLen(0)
{
}

ODBC::Bind<unsigned char *>::~Bind()
{
	if (value) {
		delete[] value;
	}
}

SQLINTEGER
ODBC::BindBase::length() const
{
	return bindLen;
}

SQLUINTEGER
ODBC::BindBase::size() const
{
	return bindSize;
}

bool
ODBC::BindBase::isNull() const
{
	return (bindLen == SQL_NULL_DATA);
}


