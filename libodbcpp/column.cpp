#include <sqlext.h>
#include <stdio.h>
#include <stdlib.h>
#include "column.h"
#include "command.h"
#include "selectcommand.h"
#include "error.h"

ODBC::Column::Column(SelectCommand * sc, const Glib::ustring & s, unsigned int i) :
	DB::Column(s, i),
	selectCmd(sc)
{
	bindLen = 0;
}

ODBC::Column::~Column()
{
}

void
ODBC::Column::resize(SQLHANDLE hStmt)
{
}

void
ODBC::CharArrayColumn::resize(SQLHANDLE hStmt)
{
	if (bindLen >= SQLLEN(data.size())) {
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
}

bool
ODBC::Column::isNull() const
{
	return (bindLen == SQL_NULL_DATA);
}

void
ODBC::Column::rebind(DB::Command * cmd, unsigned int idx) const
{
	meAsAParam()->paramCmd = dynamic_cast<ODBC::Command *>(cmd);
	meAsAParam()->paramIdx = idx;
	meAsAParam()->bind();
}

void
ODBC::Column::bind()
{
	RETCODE rc = SQLBindCol(selectCmd->hStmt, colNo + 1, ctype(), rwDataAddress(), size(), &bindLen);
	if (!SQL_SUCCEEDED(rc)) {
		throw Error(rc, SQL_HANDLE_STMT, selectCmd->hStmt, "ODBC::Column::bind");
	}
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

void
ODBC::SignedIntegerColumn::apply(DB::HandleField & h) const
{
	if (isNull()) return h.null();
	h.integer(data);
}
void
ODBC::FloatingPointColumn::apply(DB::HandleField & h) const
{
	if (isNull()) return h.null();
	h.floatingpoint(data);
}
void
ODBC::CharArrayColumn::apply(DB::HandleField & h) const
{
	if (isNull()) return h.null();
	h.string(&data.front(), bindLen);
}
void
ODBC::TimeStampColumn::apply(DB::HandleField & h) const
{
	if (isNull()) return h.null();
	h.timestamp(*this);
}
