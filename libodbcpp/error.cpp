#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include <malloc.h>
#include "error.h"

static
void
odbc_verror(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle, char const * actionfmt, va_list ap, char ** msg)
{
    SQLCHAR     sqlstatus[6];
    SQLINTEGER  sqlerr;
    SQLCHAR     sqlerrmsg[12800];

    char * action;
	if (vasprintf(&action, actionfmt, ap) < 0) {
        syslog(LOG_WARNING, "%s: %d: %ld: %5.5s: \"%s\" : failed to malloc for vasprintf",
                __FUNCTION__, err, sqlerr, sqlstatus, sqlerrmsg);
		return;
	}

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
    free(action);
}

ODBC::Error::Error(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle, char const * action, ...)
{
    va_list ap;

    va_start(ap, action);
    	odbc_verror(err, handletype, handle, action, ap, &msg);
	va_end(ap);
}

ODBC::Error::Error(char const * action, ...)
{
    va_list ap;

    va_start(ap, action);
	vsyslog(LOG_ERR, action, ap);
	if (vasprintf(&msg, action, ap) < 1) {
		msg = NULL;
	}
    va_end(ap);
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

