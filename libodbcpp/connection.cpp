#include <sqlext.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include "connection.h"
#include "selectcommand.h"
#include "modifycommand.h"
#include "error.h"

ODBC::Connection::Connection(const DSN& d) :
	env(0),
	conn(0),
	txDepth(0),
	txAborted(false)
{
	connectPre();
	RETCODE dberr = SQLConnect(conn, (SQLCHAR*)d.dsn.c_str(), SQL_NTS,
			(SQLCHAR*)d.username.c_str(), SQL_NTS, (SQLCHAR*)d.password.c_str(), SQL_NTS);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_DBC, conn, "Connect");
	}
	connectPost();
}

void
ODBC::Connection::connectPre()
{
	SQLRETURN dberr = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env, "Allocate handle");
	}

	dberr = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env, "Set ODBC version");
	}

	dberr = SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env, "Allocate DBC handle");
	}

	dberr = SQLSetConnectAttr(conn, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)5, 0);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env, "Set connection attributes");
	}
}

void
ODBC::Connection::connectPost()
{
	RETCODE dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_DBC, conn, "Set default auto commit");
	}
}

ODBC::Connection::Connection(const std::string & s) :
	env(0),
	conn(0),
	txDepth(0),
	txAborted(false)
{
	connectPre();
	RETCODE dberr = SQLDriverConnect(conn, NULL, (SQLCHAR*)s.c_str(), s.length(), NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_DBC, conn, "Connect");
	}
	connectPost();
}

ODBC::Connection::~Connection()
{
	if (conn) {
		if (!SQL_SUCCEEDED(SQLDisconnect(conn))) {
			syslog(LOG_WARNING, "%s: Disconnect error", __FUNCTION__);
		}
		if (!SQL_SUCCEEDED(SQLFreeHandle(SQL_HANDLE_DBC, conn))) {
			syslog(LOG_WARNING, "%s: Free connection handle error", __FUNCTION__);
		}
	}
	if (env) {
		if (!SQL_SUCCEEDED(SQLFreeHandle(SQL_HANDLE_ENV, env))) {
			syslog(LOG_WARNING, "%s: Free connection handle error", __FUNCTION__);
		}
	}
}

int
ODBC::Connection::beginTx() const
{
	if (txDepth == 0) {
		SQLRETURN dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF);
		if (!SQL_SUCCEEDED(dberr)) {
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
			if (!SQL_SUCCEEDED(dberr)) {
				throw Error(dberr, SQL_HANDLE_DBC, conn, "SQLEndTran (SQL_COMMIT)");
			}
			dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
			if (!SQL_SUCCEEDED(dberr)) {
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
			if (!SQL_SUCCEEDED(dberr)) {
				throw Error(dberr, SQL_HANDLE_DBC, conn, "SQLEndTran (SQL_ROLLBACK)");
			}
			dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
			if (!SQL_SUCCEEDED(dberr)) {
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

DB::SelectCommand *
ODBC::Connection::newSelectCommand(const std::string & sql) const
{
	return new ODBC::SelectCommand(*this, sql);
}

DB::ModifyCommand *
ODBC::Connection::newModifyCommand(const std::string & sql) const
{
	return new ODBC::ModifyCommand(*this, sql);
}

std::string
ODBC::Connection::getAttrStr(SQLINTEGER attr) const
{
	std::string rtn;
	rtn.resize(BUFSIZ);
	SQLINTEGER size = 0;
	SQLINTEGER dberr = SQLGetConnectAttr(conn, attr, (unsigned char *)rtn.c_str(), BUFSIZ, &size);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ODBC::Error(dberr, SQL_HANDLE_DBC, conn, "ODBC::Connection::getAttrStr SQLGetConnectAttr");
	}
	rtn.resize(size);
	return rtn;
}

SQLINTEGER
ODBC::Connection::getAttrInt(SQLINTEGER attr) const
{
	SQLINTEGER result;
	SQLINTEGER dberr = SQLGetConnectAttr(conn, attr, &result, sizeof(result), 0);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ODBC::Error(dberr, SQL_HANDLE_DBC, conn, "ODBC::Connection::getAttrInt SQLGetConnectAttr");
	}
	return result;
}

void
ODBC::Connection::ping() const
{
	SQLINTEGER dead = getAttrInt(SQL_ATTR_CONNECTION_DEAD);
	if (dead != SQL_CD_FALSE) {
		throw ODBC::Error("Connection is dead");
	}
}

ODBC::ConnectionError::ConnectionError(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle, char const * stage) :
	ODBC::Error(err, handletype, handle, stage),
	DB::ConnectionError()
{
}

ODBC::ConnectionError::ConnectionError(const ConnectionError & e) :
	ODBC::Error(strdup(e.what())),
	DB::ConnectionError(e.FailureTime)
{
}

