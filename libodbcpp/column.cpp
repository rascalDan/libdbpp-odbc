#include <sqlext.h>
#include <stdio.h>
#include <stdlib.h>
#include "column.h"
#include "command.h"
#include "error.h"

ODBC::Column::Column(const Glib::ustring & n, unsigned int i) :
	colNo(i),
	name(n),
	composeCache(NULL),
	bindSize(0)
{
}

ODBC::Column::~Column()
{
	delete composeCache;
}

void
ODBC::Column::resize(SQLHANDLE hStmt)
{
}

void
ODBC::StringColumn::resize(SQLHANDLE hStmt)
{
	if (bindSize < bindLen) {
		value.resize(bindLen + 1);
		bindSize = bindLen;
		bind(hStmt, colNo + 1, SQL_C_CHAR, &value[0], bindSize + 1);
	}
}

bool
ODBC::Column::isNull() const
{
	return (bindLen == SQL_NULL_DATA);
}

#define ODBC_DEFAULT_COLUMN_CAST(ctype, rtype) \
	ODBC::Column::operator rtype() const { \
		return (dynamic_cast<const _Column<ctype>& >(*this)).value; \
	}

ODBC_DEFAULT_COLUMN_CAST(SQLINTEGER, unsigned int);
ODBC_DEFAULT_COLUMN_CAST(SQLINTEGER, unsigned long long);
ODBC_DEFAULT_COLUMN_CAST(SQLINTEGER, long long);
ODBC_DEFAULT_COLUMN_CAST(SQLINTEGER, int);
ODBC_DEFAULT_COLUMN_CAST(SQLDOUBLE, double);
ODBC_DEFAULT_COLUMN_CAST(SQLDOUBLE, float);
ODBC::Column::operator Glib::ustring() const {
	return Glib::ustring((const char *)((dynamic_cast<const _Column<SQLCHAR*>& >(*this)).value));
}
ODBC::Column::operator std::string() const {
	return (const char*)((dynamic_cast<const _Column<SQLCHAR*>& >(*this)).value);
}
ODBC::Column::operator const char * () const {
	return (const char*)((dynamic_cast<const _Column<SQLCHAR*>& >(*this)).value);
}
ODBC::Column::operator struct tm () const {
	const _Column<SQL_TIMESTAMP_STRUCT>& c = dynamic_cast<const _Column<SQL_TIMESTAMP_STRUCT>& >(*this);
	struct tm rtn;
	rtn << c.value;
	return rtn;
}

void
ODBC::Column::bind(SQLHANDLE hStmt, SQLUINTEGER col, SQLSMALLINT ctype, void * buf, size_t size)
{
	bindSize = size;
	RETCODE rc = SQLBindCol(hStmt, col, ctype, buf, bindSize, &bindLen);
	if (rc != SQL_SUCCESS) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: Bind column %lu", __FUNCTION__, col);
	}
}

#define REBIND(t, p) \
	template<> void _Column<t>::rebind(Command * cmd, unsigned int col) const \
	{ \
		cmd->p(col, value); \
	}
namespace ODBC {
	REBIND(int, bindParamI)
	REBIND(long, bindParamI)
	REBIND(unsigned int, bindParamI)
	REBIND(long unsigned int, bindParamI)
	REBIND(long long unsigned int, bindParamI)
	REBIND(double, bindParamF)
	REBIND(float, bindParamF)
	REBIND(SQL_TIMESTAMP_STRUCT, bindParamT)

	template<>
	void
	_Column<SQLCHARVEC>::rebind(Command * cmd, unsigned int col) const \
	{
		cmd->bindParamS(col, &value[0]);
	}
	template <>
	int
	_Column<SQLDOUBLE>::writeToBuf(char ** buf, const char * fmt) const
	{
		return asprintf(buf, fmt, value);
	}
	template <>
	int
	_Column<SQLDOUBLE>::writeToBuf(char ** buf) const
	{
		return writeToBuf(buf, "%g");
	}
	template <>
	const Glib::ustring &
	_Column<SQLDOUBLE>::compose() const
	{
		if (!composeCache) {
			composeCache = new Glib::ustring(Glib::ustring::compose("%1", value));
		}
		return *composeCache;
	}
	template <>
	Glib::ustring
	_Column<SQLDOUBLE>::compose(const Glib::ustring & fmt) const
	{
		return Glib::ustring::compose(fmt, value);
	}
	template <>
	int
	_Column<SQLINTEGER>::writeToBuf(char ** buf, const char * fmt) const
	{
		return asprintf(buf, fmt, value);
	}
	template <>
	int
	_Column<SQLINTEGER>::writeToBuf(char ** buf) const
	{
		return writeToBuf(buf, "%ld");
	}
	template <>
	const Glib::ustring &
	_Column<SQLINTEGER>::compose() const
	{
		if (!composeCache) {
			composeCache = new Glib::ustring(Glib::ustring::compose("%1", value));
		}
		return *composeCache;
	}
	template <>
	Glib::ustring
	_Column<SQLINTEGER>::compose(const Glib::ustring & fmt) const
	{
		return Glib::ustring::compose(fmt, value);
	}
	template <>
	int
	_Column<SQLCHARVEC>::writeToBuf(char ** buf, const char * fmt) const
	{
		return asprintf(buf, fmt, &value[0]);
	}
	template <>
	int
	_Column<SQLCHARVEC>::writeToBuf(char ** buf) const
	{
		return writeToBuf(buf, "%s");
	}
	template <>
	const Glib::ustring &
	_Column<SQLCHARVEC>::compose() const
	{
		if (!composeCache) {
			composeCache = new Glib::ustring((const char *)&value[0]);
		}
		return *composeCache;
	}
	template <>
	Glib::ustring
	_Column<SQLCHARVEC>::compose(const Glib::ustring & fmt) const
	{
		return Glib::ustring::compose(fmt, &value[0]);
	}
	template <>
	int
	_Column<SQL_TIMESTAMP_STRUCT>::writeToBuf(char ** buf, const char * fmt) const
	{
		*buf = (char *)malloc(300);
		struct tm t;
		t << value;
		return strftime(*buf, 300, fmt, &t);
	}
	template <>
	int
	_Column<SQL_TIMESTAMP_STRUCT>::writeToBuf(char ** buf) const
	{
		return writeToBuf(buf, "%F %T");
	}
	template <>
	Glib::ustring
	_Column<SQL_TIMESTAMP_STRUCT>::compose(const Glib::ustring & fmt) const
	{
		char buf[300];
		struct tm t;
		t << value;
		int len = strftime(buf, sizeof(buf), fmt.c_str(), &t);
		return Glib::ustring(buf, len);
	}
	template <>
	const Glib::ustring &
	_Column<SQL_TIMESTAMP_STRUCT>::compose() const
	{
		if (!composeCache) {
			composeCache = new Glib::ustring(Glib::ustring(compose("%F %T")));
		}
		return *composeCache;
	}
}

void operator << (SQL_TIMESTAMP_STRUCT & target, const struct tm & src)
{
	target.year = src.tm_year + 1900;
	target.month = src.tm_mon + 1;
	target.day = src.tm_mday;
	target.hour = src.tm_hour;
	target.minute = src.tm_min;
	target.second = src.tm_sec;
	target.fraction = 0;
}
void operator << (struct tm & target, const SQL_TIMESTAMP_STRUCT & src)
{
	target.tm_year = src.year - 1900;
	target.tm_mon = src.month - 1;
	target.tm_mday = src.day;
	target.tm_hour = src.hour;
	target.tm_min = src.minute;
	target.tm_sec = src.second;
}
