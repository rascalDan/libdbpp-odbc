#include <sqlext.h>
#include <syslog.h>
#include "connection.h"
#include "error.h"

ODBC::Connection::Connection(const DSN& d) :
	DSN(d),
	env(0),
	conn(0)
{
	SQLRETURN dberr = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
	if ((dberr != SQL_SUCCESS) && (dberr != SQL_SUCCESS_WITH_INFO)) {
		throw Error(dberr, SQL_HANDLE_ENV, env, "Allocate handle");
	}

	dberr = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
	if ((dberr != SQL_SUCCESS) && (dberr != SQL_SUCCESS_WITH_INFO)) {
		throw Error(dberr, SQL_HANDLE_ENV, env, "Set ODBC version");
	}

	dberr = SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);
	if ((dberr != SQL_SUCCESS) && (dberr != SQL_SUCCESS_WITH_INFO)) {
		throw Error(dberr, SQL_HANDLE_ENV, env, "Allocate DBC handle");
	}

	dberr = SQLSetConnectAttr(conn, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)5, 0);
	if ((dberr != SQL_SUCCESS) && (dberr != SQL_SUCCESS_WITH_INFO)) {
		throw Error(dberr, SQL_HANDLE_ENV, env, "Set connection attributes");
	}

	dberr = SQLConnect(conn, dsn, SQL_NTS, username, SQL_NTS, password, SQL_NTS);
	if ((dberr != SQL_SUCCESS) && (dberr != SQL_SUCCESS_WITH_INFO)) {
		throw Error(dberr, SQL_HANDLE_DBC, conn, "Connect");
	}
}

ODBC::Connection::~Connection()
{
	if (conn) {
		if (SQLDisconnect(conn) != SQL_SUCCESS) {
			syslog(LOG_WARNING, "%s: Disconnect error", __FUNCTION__);
		}
		if (SQLFreeHandle(SQL_HANDLE_DBC, conn) != SQL_SUCCESS) {
			syslog(LOG_WARNING, "%s: Free connection handle error", __FUNCTION__);
		}
	}
	if (env) {
		if (SQLFreeHandle(SQL_HANDLE_ENV, env) != SQL_SUCCESS) {
			syslog(LOG_WARNING, "%s: Free connection handle error", __FUNCTION__);
		}
	}
}
