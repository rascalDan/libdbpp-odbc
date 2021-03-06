#include "odbc-param.h"
#include "command.h"
#include "odbc-command.h"
#include "odbc-error.h"
#include "odbc-param_fwd.h"
#include <boost/date_time/date.hpp>
#include <boost/date_time/gregorian_calendar.hpp>
#include <boost/date_time/time.hpp>
#include <memory>
#include <sqlext.h>

namespace Glib {
	class ustring;
}

ODBC::Param::Param() : paramCmd(nullptr), paramIdx(0), paramBound(false), dataLength(0) { }

ODBC::Param::Param(Command * c, unsigned int i) : paramCmd(c), paramIdx(i), paramBound(false), dataLength(0) { }

template<class ParamType>
ParamType *
ODBC::Command::makeParam(unsigned int idx)
{
	if (idx >= params.size()) {
		throw DB::ParameterOutOfRange();
	}
	auto & p = params[idx];
	if (p) {
		if (auto np = dynamic_cast<ParamType *>(p.get())) {
			return np;
		}
	}
	p = std::make_unique<ParamType>(this, idx);
	return static_cast<ParamType *>(p.get());
}

void
ODBC::Param::bind() const
{
	if (!paramBound) {
		SQLRETURN rc = SQLBindParameter(paramCmd->hStmt, static_cast<SQLUSMALLINT>(paramIdx + 1), SQL_PARAM_INPUT,
				ctype(), stype(), size(), dp(), const_cast<void *>(dataAddress()), static_cast<SQLLEN>(size()),
				&bindLen);
		if (!SQL_SUCCEEDED(rc)) {
			throw Error(rc, SQL_HANDLE_STMT, paramCmd->hStmt);
		}
		paramBound = true;
	}
}

#define SIMPLEBINDER(ctype, otype, suf) \
	void ODBC::Command::bindParam##suf(unsigned int i, ctype val) \
	{ \
		ODBC::otype * p = makeParam<ODBC::otype>(i); \
		*p = val; \
		p->bind(); \
	}
SIMPLEBINDER(int, SignedIntegerParam, I)
SIMPLEBINDER(long, SignedIntegerParam, I)
SIMPLEBINDER(long long, SignedIntegerParam, I)
SIMPLEBINDER(unsigned int, UnsignedIntegerParam, I)
SIMPLEBINDER(unsigned long int, UnsignedIntegerParam, I)
SIMPLEBINDER(unsigned long long int, UnsignedIntegerParam, I)

SIMPLEBINDER(bool, BooleanParam, B)

SIMPLEBINDER(double, FloatingPointParam, F)
SIMPLEBINDER(float, FloatingPointParam, F)

SIMPLEBINDER(const Glib::ustring &, StdStringParam, S)
SIMPLEBINDER(const std::string_view, StdStringParam, S)

SIMPLEBINDER(const boost::posix_time::ptime, TimeStampParam, T)
SIMPLEBINDER(const boost::posix_time::time_duration, IntervalParam, T)

void
ODBC::Command::bindNull(unsigned int i)
{
	makeParam<NullParam>(i)->bind();
}

ODBC::StdStringParam &
ODBC::StdStringParam::operator=(Glib::ustring const & d)
{
	const char * addr = data.data();
	data = d;
	bindLen = static_cast<SQLLEN>(d.bytes());
	// cppcheck-suppress invalidContainer
	if (addr != data.data()) {
		paramBound = false;
		bind();
	}
	return *this;
}

ODBC::StdStringParam &
ODBC::StdStringParam::operator=(const std::string_view d)
{
	const char * addr = data.data();
	data = d;
	bindLen = static_cast<SQLLEN>(d.length());
	// cppcheck-suppress invalidContainer
	if (addr != data.data()) {
		paramBound = false;
		bind();
	}
	return *this;
}

ODBC::TimeStampParam &
ODBC::TimeStampParam::operator=(const boost::posix_time::ptime d)
{
	data.year = static_cast<SQLSMALLINT>(d.date().year());
	data.month = d.date().month();
	data.day = d.date().day();
	data.hour = static_cast<SQLUSMALLINT>(d.time_of_day().hours());
	data.minute = static_cast<SQLUSMALLINT>(d.time_of_day().minutes());
	data.second = static_cast<SQLUSMALLINT>(d.time_of_day().seconds());
	data.fraction = static_cast<SQLUSMALLINT>(d.time_of_day().fractional_seconds());
	return *this;
}

ODBC::IntervalParam &
ODBC::IntervalParam::operator=(const boost::posix_time::time_duration d)
{
	data.interval_type = SQL_IS_DAY_TO_SECOND;
	data.interval_sign = d.is_negative();
	data.intval.day_second.day = static_cast<SQLUSMALLINT>(d.hours() / 24);
	data.intval.day_second.hour = static_cast<SQLUSMALLINT>(d.hours() % 24);
	data.intval.day_second.minute = static_cast<SQLUSMALLINT>(d.minutes());
	data.intval.day_second.second = static_cast<SQLUSMALLINT>(d.seconds());
	data.intval.day_second.fraction = static_cast<SQLUSMALLINT>(d.fractional_seconds());
	return *this;
}
