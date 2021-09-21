#include "odbc-command.h"
#include "odbc-connection.h"
#include "odbc-error.h"
#include <command.h>
#include <sqlext.h>

ODBC::Command::Command(const Connection & c, const std::string & s) : DB::Command(s), hStmt(nullptr), connection(c)
{
	RETCODE rc = SQLAllocHandle(SQL_HANDLE_STMT, c.conn, &hStmt);
	if (!SQL_SUCCEEDED(rc)) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt);
	}
	rc = SQLSetStmtAttr(hStmt, SQL_ATTR_CURSOR_TYPE, reinterpret_cast<SQLPOINTER>(SQL_CURSOR_DYNAMIC), 0);
	if (!SQL_SUCCEEDED(rc)) {
		throw ConnectionError(rc, SQL_HANDLE_STMT, hStmt);
	}
	rc = SQLPrepare(hStmt, const_cast<SQLCHAR *>(reinterpret_cast<const SQLCHAR *>(sql.data())),
			static_cast<int>(sql.length()));
	if (!SQL_SUCCEEDED(rc)) {
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		throw Error(rc, SQL_HANDLE_STMT, hStmt);
	}
	SQLSMALLINT pcount;
	rc = SQLNumParams(hStmt, &pcount);
	if (!SQL_SUCCEEDED(rc)) {
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		throw Error(rc, SQL_HANDLE_STMT, hStmt);
	}
	params.resize(static_cast<std::string::size_type>(pcount));
}
