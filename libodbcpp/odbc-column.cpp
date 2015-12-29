#include <sqlext.h>
#include <stdio.h>
#include <stdlib.h>
#include "odbc-column.h"
#include "odbc-command.h"
#include "odbc-selectcommand.h"
#include "odbc-error.h"

ODBC::Column::Column(SelectCommand * sc, const Glib::ustring & s, unsigned int i) :
	DB::Column(s, i),
	selectCmd(sc)
{
	bindLen = 0;
}

ODBC::Column::~Column()
{
}

bool
ODBC::Column::resize()
{
	return false;
}

bool
ODBC::CharArrayColumn::resize()
{
	if (bindLen >= SQLLEN(data.size())) {
		data.resize(bindLen + 1);
		Column::bind();
		if (paramCmd) {
			paramBound = false;
			Param::bind();
		}
		return true;
	}
	return false;
}

bool
ODBC::Column::isNull() const
{
	return (bindLen == SQL_NULL_DATA);
}

void
ODBC::Column::bind()
{
	RETCODE rc = SQLBindCol(selectCmd->hStmt, colNo + 1, ctype(), rwDataAddress(), size(), &bindLen);
	if (!SQL_SUCCEEDED(rc)) {
		throw Error(rc, SQL_HANDLE_STMT, selectCmd->hStmt, "ODBC::Column::bind");
	}
}

ODBC::TimeStampColumn::operator boost::posix_time::ptime() const
{
	return boost::posix_time::ptime(
						boost::gregorian::date(data.year, data.month, data.day),
						boost::posix_time::time_duration(data.hour, data.minute, data.second, data.fraction));
}
ODBC::IntervalColumn::operator boost::posix_time::time_duration() const
{
	auto dur = boost::posix_time::time_duration(
			(24 * data.intval.day_second.day) + data.intval.day_second.hour,
			data.intval.day_second.minute, data.intval.day_second.second, data.intval.day_second.fraction);
	return (data.interval_sign ? -dur : dur);
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
void
ODBC::IntervalColumn::apply(DB::HandleField & h) const
{
	if (isNull()) return h.null();
	h.interval(*this);
}
