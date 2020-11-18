#include "odbc-error.h"
#include <array>
#include <compileTimeFormatter.h>

namespace AdHoc {
	StreamWriterT('5') {
		template<std::size_t l, typename... Pn>
		static void
		write(stream & s, const std::array<SQLCHAR, l> & sqlstatus, const Pn &... pn)
		{
			static_assert(l > 5);
			s.write((const char * const)sqlstatus.data(), 5);
			StreamWriter::next(s, pn...);
		}
	};
}

AdHocFormatter(ODBCErrorWithInfo, "%?: %?: %5: \"%?\"");
AdHocFormatter(ODBCErrorInvalidHandle, "(%?) Invalid handle passed into function");
AdHocFormatter(ODBCErrorNoData, "(%?) No error data available for record");
AdHocFormatter(ODBCError, "Failed to get diagnostics for return code %?");
ODBC::Error::Error(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle)
{
	std::array<SQLCHAR, 6> sqlstatus {};
	SQLINTEGER sqlerr;
	std::array<SQLCHAR, 12800> sqlerrmsg {};

	SQLRETURN rc = SQLGetDiagRec(
			handletype, handle, 1, sqlstatus.data(), &sqlerr, sqlerrmsg.data(), sqlerrmsg.size(), nullptr);
	switch (rc) {
		case SQL_SUCCESS:
		case SQL_SUCCESS_WITH_INFO:
			msg = ODBCErrorWithInfo::get(err, sqlerr, sqlstatus, sqlerrmsg.data());
			break;

		case SQL_INVALID_HANDLE:
			msg = ODBCErrorInvalidHandle::get(err);
			break;

		case SQL_NO_DATA:
			msg = ODBCErrorNoData::get(err);
			break;

		case SQL_ERROR:
		default:
			msg = ODBCError::get(err);
			break;
	}
}

std::string
ODBC::Error::message() const noexcept
{
	return msg;
}
