#include <sqlext.h>
#include <stdio.h>
#include <stdlib.h>
#include "column.h"
#include "command.h"
#include "selectcommand.h"
#include "error.h"

ODBC::Column::Column(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
	colNo(i),
	name(n),
	selectCmd(sc),
	composeCache(NULL)
{
	bindLen = 0;
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
ODBC::CharArrayColumn::resize(SQLHANDLE hStmt)
{
	if (bindLen > SQLLEN(data.size())) {
		data.resize(bindLen + 1);
		Column::bind();
		if (paramCmd) {
			paramBound = false;
			Param::bind();
		}
	}
}

void
ODBC::Column::onScroll()
{
	delete composeCache;
	composeCache = NULL;
}

bool
ODBC::Column::isNull() const
{
	return (bindLen == SQL_NULL_DATA);
}

void
ODBC::Column::rebind(Command * cmd, unsigned int idx) const
{
	meAsAParam()->paramCmd = cmd;
	meAsAParam()->paramIdx = idx;
	meAsAParam()->bind();
}

void
ODBC::Column::bind()
{
	RETCODE rc = SQLBindCol(selectCmd->hStmt, colNo + 1, ctype(), rwDataAddress(), size(), &bindLen);
	if (rc != SQL_SUCCESS) {
		throw Error(rc, SQL_HANDLE_STMT, selectCmd->hStmt, "%s: Bind column %u", __FUNCTION__, colNo);
	}
}

#define SIMPLEFORMATTER(ctype, deffmtstr) \
	int \
	ODBC::ctype::writeToBuf(char ** buf, const char * fmt) const \
	{ \
		return asprintf(buf, fmt, data); \
	} \
	int \
	ODBC::ctype::writeToBuf(char ** buf) const \
	{ \
		return writeToBuf(buf, deffmtstr); \
	} \
	const Glib::ustring & \
	ODBC::ctype::compose() const \
	{ \
		if (!composeCache) { \
			composeCache = new Glib::ustring(Glib::ustring::compose("%1", data)); \
		} \
		return *composeCache; \
	} \
	Glib::ustring \
	ODBC::ctype::compose(const Glib::ustring & fmt) const \
	{ \
		return Glib::ustring::compose(fmt, data); \
	}
SIMPLEFORMATTER(FloatingPointColumn, "%g");
SIMPLEFORMATTER(SignedIntegerColumn, "%ld");
#ifdef COMPLETENESS
SIMPLEFORMATTER(UnsignedIntegerColumn, "%lu");
#endif

int
ODBC::CharArrayColumn::writeToBuf(char ** buf, const char * fmt) const
{
	return asprintf(buf, fmt, &data[0]);
}
int
ODBC::CharArrayColumn::writeToBuf(char ** buf) const
{
	return writeToBuf(buf, "%s");
}
const Glib::ustring &
ODBC::CharArrayColumn::compose() const
{
	if (!composeCache) {
		composeCache = new Glib::ustring(&data.front(), bindLen);
	}
	return *composeCache;
}
Glib::ustring
ODBC::CharArrayColumn::compose(const Glib::ustring & fmt) const
{
	return Glib::ustring::compose(fmt, &data.front());
}
int
ODBC::TimeStampColumn::writeToBuf(char ** buf, const char * fmt) const
{
	*buf = (char *)malloc(300);
	struct tm t;
	t << data;
	return strftime(*buf, 300, fmt, &t);
}
int
ODBC::TimeStampColumn::writeToBuf(char ** buf) const
{
	return writeToBuf(buf, "%F %T");
}
Glib::ustring
ODBC::TimeStampColumn::compose(const Glib::ustring & fmt) const
{
	char buf[300];
	struct tm t;
	t << data;
	int len = strftime(buf, sizeof(buf), fmt.c_str(), &t);
	return Glib::ustring(buf, len);
}
const Glib::ustring &
ODBC::TimeStampColumn::compose() const
{
	if (!composeCache) {
		composeCache = new Glib::ustring(Glib::ustring(compose("%F %T")));
	}
	return *composeCache;
}
ODBC::TimeStampColumn::operator tm() const
{
	struct tm t;
	t << data;
	return t;
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
