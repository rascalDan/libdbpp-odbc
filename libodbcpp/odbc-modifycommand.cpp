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
	RETCODE rc = SQLExecute(hStmt);
	if (!SQL_SUCCEEDED(rc)) {
		if (rc != SQL_NO_DATA || !anc) {
			throw Error(rc, SQL_HANDLE_STMT, hStmt);
		}
	}
	SQLLEN rows = 0;
	rc = SQLRowCount(hStmt, &rows);
	if (!SQL_SUCCEEDED(rc)) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt);
	}
	if (rows > 0 || anc) {
		return rows;
	}
	throw DB::NoRowsAffected();
}

