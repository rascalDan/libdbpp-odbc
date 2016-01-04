#include "odbc-error.h"
#include <buffer.h>

ODBC::Error::Error(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle)
{
	SQLCHAR     sqlstatus[6];
	SQLINTEGER  sqlerr;
	SQLCHAR     sqlerrmsg[12800];

	SQLRETURN rc = SQLGetDiagRec(handletype, handle, 1, sqlstatus, &sqlerr, sqlerrmsg, sizeof(sqlerrmsg), NULL);
	switch (rc) {
		case SQL_SUCCESS:
		case SQL_SUCCESS_WITH_INFO:
			msg = stringbf("%d: %d: %5.5s: \"%s\"", err, (int)sqlerr, sqlstatus, sqlerrmsg);
			break;

		case SQL_INVALID_HANDLE:
			msg = stringbf("(%d) Invalid handle passed into function", err);
			break;

		case SQL_NO_DATA:
			msg = stringbf("(%d) No error data available for record", err);
			break;

		case SQL_ERROR:
		default:
			msg = stringbf("Failed to get diagnostics for return code %d", err);
			break;
	}
}

std::string
ODBC::Error::message() const throw()
{
	return msg;
}

