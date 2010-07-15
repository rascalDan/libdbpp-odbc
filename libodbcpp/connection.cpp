#include <sqlext.h>
#include <syslog.h>
#include <pcap.h>
#include <string.h>
#include "connection.h"
#include "error.h"

ODBC::Connection::Connection(const DSN& d) :
	env(0),
	conn(0),
	txDepth(0),
	txAborted(false)
{
	SQLRETURN dberr = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
	if ((dberr != SQL_SUCCESS)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env, "Allocate handle");
	}

	dberr = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
	if ((dberr != SQL_SUCCESS)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env, "Set ODBC version");
	}

	dberr = SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);
	if ((dberr != SQL_SUCCESS)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env, "Allocate DBC handle");
	}

	dberr = SQLSetConnectAttr(conn, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)5, 0);
	if ((dberr != SQL_SUCCESS)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env, "Set connection attributes");
	}

	dberr = SQLConnect(conn, (SQLCHAR*)d.dsn.c_str(), SQL_NTS,
			(SQLCHAR*)d.username.c_str(), SQL_NTS, (SQLCHAR*)d.password.c_str(), SQL_NTS);
	if ((dberr != SQL_SUCCESS)) {
		throw ConnectionError(dberr, SQL_HANDLE_DBC, conn, "Connect");
	}

	dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
	if ((dberr != SQL_SUCCESS)) {
		throw ConnectionError(dberr, SQL_HANDLE_DBC, conn, "Set default auto commit");
	}
}

ODBC::Connection::Connection(const std::string & s) :
	env(0),
	conn(0),
	txDepth(0),
	txAborted(false)
{
	SQLRETURN dberr = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
	if ((dberr != SQL_SUCCESS)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env, "Allocate handle");
	}

	dberr = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
	if ((dberr != SQL_SUCCESS)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env, "Set ODBC version");
	}

	dberr = SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);
	if ((dberr != SQL_SUCCESS)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env, "Allocate DBC handle");
	}

	dberr = SQLSetConnectAttr(conn, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)5, 0);
	if ((dberr != SQL_SUCCESS)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env, "Set connection attributes");
	}

	dberr = SQLDriverConnect(conn, NULL, (SQLCHAR*)s.c_str(), s.length(), NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
	if ((dberr != SQL_SUCCESS)) {
		throw ConnectionError(dberr, SQL_HANDLE_DBC, conn, "Connect");
	}

	dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
	if ((dberr != SQL_SUCCESS)) {
		throw ConnectionError(dberr, SQL_HANDLE_DBC, conn, "Set default auto commit");
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

int
ODBC::Connection::beginTx() const
{
	if (txDepth == 0) {
		SQLRETURN dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF);
		if ((dberr != SQL_SUCCESS)) {
			throw Error(dberr, SQL_HANDLE_DBC, conn, "Set default auto commit");
		}
	}
	txDepth += 1;
	return txDepth;
}

int
ODBC::Connection::commitTx() const
{
	if (txDepth > 0) {
		if (txAborted) {
			return rollbackTx();
		}
		txDepth -= 1;
		if (txDepth == 0) {
			SQLRETURN dberr = SQLEndTran(SQL_HANDLE_DBC, conn, SQL_COMMIT);
			if ((dberr != SQL_SUCCESS)) {
				throw Error(dberr, SQL_HANDLE_DBC, conn, "SQLEndTran (SQL_COMMIT)");
			}
			dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
			if ((dberr != SQL_SUCCESS)) {
				throw Error(dberr, SQL_HANDLE_DBC, conn, "Enable auto commit");
			}
			txAborted = false;
		}
		return txDepth;
	}
	throw Error("Attempt to commit none existant transaction");
}

int
ODBC::Connection::rollbackTx() const
{
	if (txDepth > 0) {
		txDepth -= 1;
		if (txDepth == 0) {
			SQLRETURN dberr = SQLEndTran(SQL_HANDLE_DBC, conn, SQL_ROLLBACK);
			if ((dberr != SQL_SUCCESS)) {
				throw Error(dberr, SQL_HANDLE_DBC, conn, "SQLEndTran (SQL_ROLLBACK)");
			}
			dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
			if ((dberr != SQL_SUCCESS)) {
				throw Error(dberr, SQL_HANDLE_DBC, conn, "Enable auto commit");
			}
			txAborted = false;
		}
		return txDepth;
	}
	throw Error("Attempt to rollback none existant transaction");
}

void
ODBC::Connection::abortTx() const
{
	txAborted = true;
}

bool
ODBC::Connection::txIsAborted() const
{
	return txAborted;
}

bool
ODBC::Connection::inTx() const
{
	return (txDepth > 0);
}

std::string
ODBC::Connection::getAttrStr(SQLINTEGER attr) const
{
	unsigned char buf[BUFSIZ];
	SQLINTEGER size = 0;
	SQLINTEGER rc = SQLGetConnectAttr(conn, attr, buf, BUFSIZ, &size);
	if (rc != SQL_SUCCESS) {
		throw ODBC::Error(rc, SQL_HANDLE_DBC, conn, "%s", __FUNCTION__);
	}
	return std::string((const char *)buf, size);
}

SQLINTEGER
ODBC::Connection::getAttrInt(SQLINTEGER attr) const
{
	SQLINTEGER result;
	SQLINTEGER rc = SQLGetConnectAttr(conn, attr, &result, sizeof(result), 0);
	if (rc != SQL_SUCCESS) {
		throw ODBC::Error(rc, SQL_HANDLE_DBC, conn, "%s", __FUNCTION__);
	}
	return result;
}

ODBC::ConnectionError::ConnectionError(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle, char const * stage) :
	ODBC::Error(err, handletype, handle, "%s", stage),
	FailureTime(time(NULL))
{
}

ODBC::ConnectionError::ConnectionError(const ConnectionError & e) :
	ODBC::Error(strdup(e.what())),
	FailureTime(e.FailureTime)
{
}

