#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include <malloc.h>
#include <time.h>
#include <string.h>
#include "error.h"

static
void
odbc_verror(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle, char const * action, char ** msg)
{
    SQLCHAR     sqlstatus[6];
    SQLINTEGER  sqlerr;
    SQLCHAR     sqlerrmsg[12800];

    SQLRETURN rc = SQLGetDiagRec(handletype, handle, 1, sqlstatus, &sqlerr, sqlerrmsg,
			sizeof(sqlerrmsg), NULL);
    switch (rc) {
    case SQL_SUCCESS:
    case SQL_SUCCESS_WITH_INFO:
		if (msg) {
			if (asprintf(msg, "%d: %ld: %5.5s: \"%s\" while attempting to %s",
					err, sqlerr, sqlstatus, sqlerrmsg, action) < 1) {
				*msg = NULL;
			}
		}
        syslog(LOG_WARNING, "%s: %d: %ld: %5.5s: \"%s\" while attempting to %s",
                __FUNCTION__, err, sqlerr, sqlstatus, sqlerrmsg, action);
        break;

    case SQL_INVALID_HANDLE:
		if (msg) {
			if (asprintf(msg, "(%d) Invalid handle passed into function trying to %s.",
					err, action) < 1) {
				*msg = NULL;
			}
		}
        syslog(LOG_ERR, "%s: (%d) Invalid handle passed into function trying to %s.",
                __FUNCTION__, err, action);
        break;

    case SQL_NO_DATA:
		if (msg) {
			if (asprintf(msg, "(%d) No error data available for record trying to %s.",
					err, action) < 1) {
				*msg = NULL;
			}
		}
        syslog(LOG_ERR, "%s: (%d) No error data available for record trying to %s.",
                __FUNCTION__, err, action);
        break;

    case SQL_ERROR:
    default:
        syslog(LOG_ERR, "%s: Unexpected error!!", __FUNCTION__);
        break;
    }
}

ODBC::Error::Error(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle, char const * action)
{
	odbc_verror(err, handletype, handle, action, &msg);
}

ODBC::Error::Error(char const * action)
{
	syslog(LOG_ERR, "%s", action);
	msg = strdup(action);
}

ODBC::Error::Error(char * m) : msg(m)
{
}

ODBC::Error::~Error() throw()
{
	free(msg);
}

const char *
ODBC::Error::what() const throw()
{
	return msg;
}

