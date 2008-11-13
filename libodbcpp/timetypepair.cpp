#include "command.h"
#include <string.h>

ODBC::TimeTypePair::TimeTypePair()
{
	memset(&_c, 0, sizeof(_c));
	memset(&_sql, 0, sizeof(_sql));
}
ODBC::TimeTypePair::TimeTypePair(const ODBC::TimeTypePair::SQL_TS& t)
{
	memset(&_c, 0, sizeof(_c));
	memset(&_sql, 0, sizeof(_sql));
	set(t);
}
ODBC::TimeTypePair::TimeTypePair(tm const& t)
{
	memset(&_c, 0, sizeof(_c));
	memset(&_sql, 0, sizeof(_sql));
	set(t);
}

tm const&
ODBC::TimeTypePair::set(const ODBC::TimeTypePair::SQL_TS& t)
{
	_sql = t;
	sql2c();
	return _c;
}

void
ODBC::TimeTypePair::sql2c() const
{
	_c.tm_year = _sql.year - 1900;
	_c.tm_mon = _sql.month - 1;
	_c.tm_mday = _sql.day;
	_c.tm_hour = _sql.hour;
	_c.tm_min = _sql.minute;
	_c.tm_sec = _sql.second;
}

const ODBC::TimeTypePair::SQL_TS&
ODBC::TimeTypePair::set(tm const& t)
{
	_c = t;
	c2sql();
	return _sql;
}

void
ODBC::TimeTypePair::c2sql() const
{
	_sql.year = _c.tm_year + 1900;
	_sql.month = _c.tm_mon + 1;
	_sql.day = _c.tm_mday;
	_sql.hour = _c.tm_hour;
	_sql.minute = _c.tm_min;
	_sql.second = _c.tm_sec;
	_sql.fraction = 0;
}

