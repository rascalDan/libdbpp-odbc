#include "command.h"
#include "error.h"
#include "param.h"
#include <sqlext.h>

ODBC::Command::Command(const Connection& c, String s) :
	sql(s),
	connection(c)
{
	RETCODE rc = SQLAllocHandle(SQL_HANDLE_STMT, c.conn, &hStmt);
	if (rc != SQL_SUCCESS) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "Allocate statement handle");
	}
    rc = SQLPrepare(hStmt, sql, sql.size());
    if (rc != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "Prepare statement");
    }
	SQLSMALLINT pcount;
    rc = SQLNumParams(hStmt, &pcount);
    if (rc != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "Parameter count");
    }
	params.resize(pcount);
}

ODBC::Command::~Command()
{
	for (Params::iterator i = params.begin(); i != params.end(); i++) {
		if (*i) {
			delete *i;
		}
	}
}

