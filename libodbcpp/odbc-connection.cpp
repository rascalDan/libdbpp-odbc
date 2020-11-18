#include "odbc-connection.h"
#include "error.h"
#include "odbc-modifycommand.h"
#include "odbc-selectcommand.h"
#include <cstdio>
#include <cstring>
#include <sqlext.h>
#include <stdexcept>

NAMEDFACTORY("odbc", ODBC::Connection, DB::ConnectionFactory);

ODBC::Connection::Connection(const DSN & d) :
	env(nullptr), conn(nullptr), thinkDelStyle(DB::BulkDeleteUsingUsing), thinkUpdStyle(DB::BulkUpdateUsingFromSrc)
{
	connectPre();
	RETCODE dberr = SQLConnect(conn, (SQLCHAR *)d.dsn.c_str(), SQL_NTS, (SQLCHAR *)d.username.c_str(), SQL_NTS,
			(SQLCHAR *)d.password.c_str(), SQL_NTS);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_DBC, conn);
	}
	connectPost();
}

void
ODBC::Connection::connectPre()
{
	SQLRETURN dberr = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env);
	}

	dberr = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env);
	}

	dberr = SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env);
	}

	dberr = SQLSetConnectAttr(conn, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)5, 0);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env);
	}
}

void
ODBC::Connection::connectPost()
{
	RETCODE dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_DBC, conn);
	}
	std::array<SQLCHAR, 1024> info {};
	dberr = SQLGetInfo(conn, SQL_DRIVER_NAME, info.data(), info.size(), nullptr);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_DBC, conn);
	}
	// Apply known DB specific tweaks
	// NOLINTNEXTLINE(hicpp-no-array-decay)
	if (strcasestr((const char *)info.data(), "myodbc")) {
		thinkDelStyle = DB::BulkDeleteUsingUsingAlias;
		thinkUpdStyle = DB::BulkUpdateUsingJoin;
	}
}

ODBC::Connection::Connection(const std::string & s) :
	env(nullptr), conn(nullptr), thinkDelStyle(DB::BulkDeleteUsingUsing), thinkUpdStyle(DB::BulkUpdateUsingFromSrc)
{
	connectPre();
	RETCODE dberr = SQLDriverConnect(
			conn, nullptr, (SQLCHAR *)s.c_str(), s.length(), nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_DBC, conn);
	}
	connectPost();
}

ODBC::Connection::~Connection()
{
	if (conn) {
		SQLDisconnect(conn);
		SQLFreeHandle(SQL_HANDLE_DBC, conn);
	}
	if (env) {
		SQLFreeHandle(SQL_HANDLE_ENV, env);
	}
}

void
ODBC::Connection::beginTxInt()
{
	SQLRETURN dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF);
	if (!SQL_SUCCEEDED(dberr)) {
		throw Error(dberr, SQL_HANDLE_DBC, conn);
	}
}

void
ODBC::Connection::commitTxInt()
{
	SQLRETURN dberr = SQLEndTran(SQL_HANDLE_DBC, conn, SQL_COMMIT);
	if (!SQL_SUCCEEDED(dberr)) {
		throw Error(dberr, SQL_HANDLE_DBC, conn);
	}
	dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
	if (!SQL_SUCCEEDED(dberr)) {
		throw Error(dberr, SQL_HANDLE_DBC, conn);
	}
}

void
ODBC::Connection::rollbackTxInt()
{
	SQLRETURN dberr = SQLEndTran(SQL_HANDLE_DBC, conn, SQL_ROLLBACK);
	if (!SQL_SUCCEEDED(dberr)) {
		throw Error(dberr, SQL_HANDLE_DBC, conn);
	}
	dberr = SQLSetConnectOption(conn, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
	if (!SQL_SUCCEEDED(dberr)) {
		throw Error(dberr, SQL_HANDLE_DBC, conn);
	}
}

DB::BulkDeleteStyle
ODBC::Connection::bulkDeleteStyle() const
{
	return thinkDelStyle;
}

DB::BulkUpdateStyle
ODBC::Connection::bulkUpdateStyle() const
{
	return thinkUpdStyle;
}

DB::SelectCommandPtr
ODBC::Connection::select(const std::string & sql, const DB::CommandOptionsCPtr &)
{
	return std::make_shared<ODBC::SelectCommand>(*this, sql);
}

DB::ModifyCommandPtr
ODBC::Connection::modify(const std::string & sql, const DB::CommandOptionsCPtr &)
{
	return std::make_shared<ODBC::ModifyCommand>(*this, sql);
}

std::string
ODBC::Connection::getAttrStr(SQLINTEGER attr) const
{
	std::string rtn;
	rtn.resize(BUFSIZ);
	SQLINTEGER size = 0;
	SQLINTEGER dberr = SQLGetConnectAttr(conn, attr, (unsigned char *)rtn.c_str(), BUFSIZ, &size);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ODBC::Error(dberr, SQL_HANDLE_DBC, conn);
	}
	rtn.resize(size);
	return rtn;
}

SQLINTEGER
ODBC::Connection::getAttrInt(SQLINTEGER attr) const
{
	SQLINTEGER result;
	SQLINTEGER dberr = SQLGetConnectAttr(conn, attr, &result, sizeof(result), nullptr);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ODBC::Error(dberr, SQL_HANDLE_DBC, conn);
	}
	return result;
}

void
ODBC::Connection::ping() const
{
	SQLINTEGER dead = getAttrInt(SQL_ATTR_CONNECTION_DEAD);
	if (dead != SQL_CD_FALSE) {
		throw ODBC::ConnectionError(0, SQL_HANDLE_DBC, conn);
	}
}

ODBC::ConnectionError::ConnectionError(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle) :
	ODBC::Error(err, handletype, handle), DB::ConnectionError()
{
}
