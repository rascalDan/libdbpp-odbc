#include "odbc-modifycommand.h"
#include "odbc-error.h"

ODBC::ModifyCommand::ModifyCommand(const ODBC::Connection & c, const std::string & sql) :
	DB::Command(sql),
	ODBC::Command(c, sql),
	DB::ModifyCommand(sql)
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
    if (!SQL_SUCCEEDED(rc)) {
		if (rc != SQL_NO_DATA || !anc) {
			connection.abortTx();
			throw Error(rc, SQL_HANDLE_STMT, hStmt, "ODBC::ModifyCommand::execute SQLExecute");
		}
    }
	SQLLEN rows;
	rc = SQLRowCount(hStmt, &rows);
    if (!SQL_SUCCEEDED(rc)) {
		connection.abortTx();
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "ODBC::ModifyCommand::execute SQLRowCount");
	}
	if (rows > 0 || anc) {
		return rows;
	}
	connection.abortTx();
	throw Error("No rows affected");
}

