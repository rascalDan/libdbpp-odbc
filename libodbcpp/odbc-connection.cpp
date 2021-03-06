#include "odbc-connection.h"
#include "connection.h"
#include "odbc-dsn.h"
#include "odbc-error.h"
#include "odbc-modifycommand.h"
#include "odbc-selectcommand.h"
#include <array>
#include <cstdio>
#include <cstring>
#include <factory.h>
#include <memory>
#include <sqlext.h>

NAMEDFACTORY("odbc", ODBC::Connection, DB::ConnectionFactory)

ODBC::Connection::Connection(const DSN & d) :
	env(nullptr), conn(nullptr), thinkDelStyle(DB::BulkDeleteStyle::UsingUsing),
	thinkUpdStyle(DB::BulkUpdateStyle::UsingFromSrc)
{
	connectPre();
	RETCODE dberr = SQLConnect(conn, const_cast<SQLCHAR *>(reinterpret_cast<const SQLCHAR *>(d.dsn.c_str())), SQL_NTS,
			const_cast<SQLCHAR *>(reinterpret_cast<const SQLCHAR *>(d.username.c_str())), SQL_NTS,
			const_cast<SQLCHAR *>(reinterpret_cast<const SQLCHAR *>(d.password.c_str())), SQL_NTS);
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

	dberr = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER *>(SQL_OV_ODBC3), 0);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env);
	}

	dberr = SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ConnectionError(dberr, SQL_HANDLE_ENV, env);
	}

	dberr = SQLSetConnectAttr(conn, SQL_LOGIN_TIMEOUT, reinterpret_cast<SQLPOINTER *>(5), 0);
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
	if (strcasestr(reinterpret_cast<const char *>(info.data()), "myodbc")) {
		thinkDelStyle = DB::BulkDeleteStyle::UsingUsingAlias;
		thinkUpdStyle = DB::BulkUpdateStyle::UsingJoin;
	}
}

ODBC::Connection::Connection(const std::string & s) :
	env(nullptr), conn(nullptr), thinkDelStyle(DB::BulkDeleteStyle::UsingUsing),
	thinkUpdStyle(DB::BulkUpdateStyle::UsingFromSrc)
{
	connectPre();
	RETCODE dberr = SQLDriverConnect(conn, nullptr, const_cast<SQLCHAR *>(reinterpret_cast<const SQLCHAR *>(s.c_str())),
			static_cast<SQLSMALLINT>(s.length()), nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT);
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
	SQLRETURN dberr = SQLGetConnectAttr(
			conn, attr, const_cast<SQLCHAR *>(reinterpret_cast<const SQLCHAR *>(rtn.c_str())), BUFSIZ, &size);
	if (!SQL_SUCCEEDED(dberr)) {
		throw ODBC::Error(dberr, SQL_HANDLE_DBC, conn);
	}
	rtn.resize(static_cast<std::string::size_type>(size));
	return rtn;
}

SQLINTEGER
ODBC::Connection::getAttrInt(SQLINTEGER attr) const
{
	SQLINTEGER result;
	SQLRETURN dberr = SQLGetConnectAttr(conn, attr, &result, sizeof(result), nullptr);
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
	// NOLINTNEXTLINE(bugprone-throw-keyword-missing)
	ODBC::Error(err, handletype, handle), DB::ConnectionError()
{
}
