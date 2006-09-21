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
	if (connection.txIsAborted()) {
		throw Error("Transaction has been aborted, not issuing any more commands");
	}
	RETCODE rc = SQLExecute(hStmt); 
    if (rc != SQL_SUCCESS) {
		if (rc != SQL_NO_DATA || !anc) {
			connection.abortTx();
			throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: SQLExecute",
					__FUNCTION__);
		}
    }
	SQLINTEGER rows;
	if ((rc = SQLRowCount(hStmt, &rows)) != SQL_SUCCESS) {
		connection.abortTx();
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: SQLRowCount",
				__FUNCTION__);
	}
	if (rows > 0 || anc) {
		return rows;
	}
	connection.abortTx();
	throw Error("%s: No rows affected", __FUNCTION__);
}

