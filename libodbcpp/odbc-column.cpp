#include "odbc-column.h"
#include "column.h"
#include "odbc-error.h"
#include "odbc-param_fwd.h"
#include "odbc-selectcommand.h"
#include <boost/date_time/gregorian_calendar.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/time.hpp>
#include <cstdio>

namespace Glib {
	class ustring;
}

ODBC::Column::Column(SelectCommand * sc, const Glib::ustring & s, unsigned int i) : DB::Column(s, i), selectCmd(sc)
{
	bindLen = 0;
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
		data.resize(static_cast<std::size_t>(bindLen + 1));
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
	RETCODE rc = SQLBindCol(selectCmd->hStmt, static_cast<SQLUSMALLINT>(colNo + 1), ctype(), rwDataAddress(),
			static_cast<SQLLEN>(size()), &bindLen);
	if (!SQL_SUCCEEDED(rc)) {
		throw Error(rc, SQL_HANDLE_STMT, selectCmd->hStmt);
	}
}

void
ODBC::SignedIntegerColumn::apply(DB::HandleField & h) const
{
	if (isNull()) {
		return h.null();
	}
	h.integer(data);
}
void
ODBC::FloatingPointColumn::apply(DB::HandleField & h) const
{
	if (isNull()) {
		return h.null();
	}
	h.floatingpoint(data);
}
void
ODBC::CharArrayColumn::apply(DB::HandleField & h) const
{
	if (isNull()) {
		return h.null();
	}
	h.string({data.data(), static_cast<std::size_t>(bindLen)});
}
void
ODBC::TimeStampColumn::apply(DB::HandleField & h) const
{
	if (isNull()) {
		return h.null();
	}
	h.timestamp(boost::posix_time::ptime(
			boost::gregorian::date(static_cast<unsigned short int>(data.year),
					static_cast<unsigned short int>(data.month), static_cast<unsigned short int>(data.day)),
			boost::posix_time::time_duration(data.hour, data.minute, data.second, data.fraction)));
}
void
ODBC::IntervalColumn::apply(DB::HandleField & h) const
{
	if (isNull()) {
		return h.null();
	}
	auto dur = boost::posix_time::time_duration((24 * data.intval.day_second.day) + data.intval.day_second.hour,
			data.intval.day_second.minute, data.intval.day_second.second, data.intval.day_second.fraction);
	h.interval(data.interval_sign ? -dur : dur);
}
