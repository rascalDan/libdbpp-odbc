#include "modifycommand.h"
#include "error.h"

ODBC::ModifyCommand::ModifyCommand(const ODBC::Connection &c, String sql) :
	Command(c, sql)
{
}

ODBC::ModifyCommand::~ModifyCommand()
{
}

unsigned int
ODBC::ModifyCommand::execute(bool anc)
{
	RETCODE rc = SQLExecute(hStmt); 
    if (rc != SQL_SUCCESS) {
		if (rc != SQL_NO_DATA || !anc) {
			throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: SQLExecute",
					__FUNCTION__);
		}
    }
	SQLINTEGER rows;
	if ((rc = SQLRowCount(hStmt, &rows)) != SQL_SUCCESS) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: SQLRowCount",
				__FUNCTION__);
	}
	if (rows > 0 || anc) {
		return rows;
	}
	throw Error("%s: No rows affected", __FUNCTION__);
}

