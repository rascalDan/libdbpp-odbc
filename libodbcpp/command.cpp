#include "command.h"
#include "error.h"
#include "param.h"
#include <sqlext.h>

ODBC::Command::Command(const Connection & c, const std::string & s) :
	DB::Command(s),
	connection(c)
{
	RETCODE rc = SQLAllocHandle(SQL_HANDLE_STMT, c.conn, &hStmt);
	if (!SQL_SUCCEEDED(rc)) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "Allocate statement handle");
	}
	rc = SQLSetStmtAttr(hStmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
	if (!SQL_SUCCEEDED(rc)) {
		throw ConnectionError(rc, SQL_HANDLE_STMT, hStmt, "Set scrollable cursor");
	}
    rc = SQLPrepare(hStmt, (SQLCHAR*)sql.c_str(), sql.length());
    if (!SQL_SUCCEEDED(rc)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "Prepare statement");
    }
	SQLSMALLINT pcount;
    rc = SQLNumParams(hStmt, &pcount);
    if (!SQL_SUCCEEDED(rc)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "Parameter count");
    }
	params.resize(pcount);
}

ODBC::Command::~Command()
{
	for (Params::iterator i = params.begin(); i != params.end(); ++i) {
		if (*i) {
			delete *i;
		}
	}
}

