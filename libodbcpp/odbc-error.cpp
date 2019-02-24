#include "odbc-error.h"
#include <compileTimeFormatter.h>

namespace AdHoc {
	StreamWriterT('5') {
		template<typename ... Pn>
		static void write(stream & s, const SQLCHAR sqlstatus[6], const Pn & ... pn)
		{
			s.write(reinterpret_cast<const char *>(sqlstatus), 5);
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
	SQLCHAR     sqlstatus[6];
	SQLINTEGER  sqlerr;
	SQLCHAR     sqlerrmsg[12800];

	// NOLINTNEXTLINE(hicpp-no-array-decay)
	SQLRETURN rc = SQLGetDiagRec(handletype, handle, 1, sqlstatus, &sqlerr, sqlerrmsg, sizeof(sqlerrmsg), nullptr);
	switch (rc) {
		case SQL_SUCCESS:
		case SQL_SUCCESS_WITH_INFO:
			msg = ODBCErrorWithInfo::get(err, sqlerr, sqlstatus, sqlerrmsg);
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

