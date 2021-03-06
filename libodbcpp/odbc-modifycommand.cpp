#include "odbc-modifycommand.h"
#include "command.h"
#include "modifycommand.h"
#include "odbc-command.h"
#include "odbc-error.h"
#include <sql.h>

ODBC::ModifyCommand::ModifyCommand(const ODBC::Connection & c, const std::string & sql) :
	DB::Command(sql), ODBC::Command(c, sql), DB::ModifyCommand(sql)
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
		return static_cast<unsigned int>(rows);
	}
	throw DB::NoRowsAffected();
}
