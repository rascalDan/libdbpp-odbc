#include "selectcommand.h"
#include "error.h"
#include "column.h"
#include <sqlext.h>

ODBC::SelectCommand::SelectCommand(const Connection& c, String s) :
	Command(c, s)
{
}

ODBC::SelectCommand::~SelectCommand()
{
	for (Columns::iterator i = columns.begin(); i != columns.end(); i++) {
		if (*i) {
			delete *i;
		}
	}
	RETCODE rc;
	if ((rc = SQLCloseCursor(hStmt)) != SQL_SUCCESS) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: SQLCloseCursor",
				__FUNCTION__);
	}
}

bool
ODBC::SelectCommand::fetch()
{
	if (columns.size() == 0) {
		execute();
	}
	RETCODE rc = SQLFetch(hStmt);
	switch (rc) {
		case SQL_SUCCESS:
			for (Columns::iterator col = columns.begin(); col != columns.end(); col++) {
				(*col)->fresh = true;
			}
			return true;
		case SQL_NO_DATA:
			return false;
		default:
			throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: SQLFetch",
					__FUNCTION__);
	}
}

// This is here cos it needs to be referenced by (and only by) execute
template <class t>
ODBC::_Column<t>::_Column(String n, u_int i) : Column(n, i)
{
}

void
ODBC::SelectCommand::execute()
{
	RETCODE rc = SQLExecute(hStmt); 
    if (rc != SQL_SUCCESS) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: SQLExecute",
				__FUNCTION__);
    }
    SQLSMALLINT colCount;
    if ((rc = SQLNumResultCols(hStmt, &colCount)) != SQL_SUCCESS) {
        throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: SQLNumResultCols",
                __FUNCTION__);
    }
	if (colCount < 1) {
		throw Error("%s: No result columns", __FUNCTION__);
	}
	columns.resize(colCount);
	for (int col = 0; col < colCount; col++) {
		SQLCHAR colName[300];
		SQLSMALLINT nameLen, dp, nullable, bindType;
		SQLUINTEGER bindSize;
		int sqlcol = col + 1;
		if ((rc = SQLDescribeCol(hStmt, sqlcol, colName, sizeof(colName), &nameLen, &bindType,
					&bindSize, &dp, &nullable)) != SQL_SUCCESS) {
			throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: SQLDescribeCol for %d",
					__FUNCTION__, col);
		}
		colName[nameLen] = '\0';
		switch (bindType) {
			case SQL_CHAR:
			case SQL_VARCHAR:
			case SQL_LONGVARCHAR:
				{
					_Column<SQLCHAR*>* s = new _Column<SQLCHAR*>(colName, col);
					s->value = new SQLCHAR[bindSize + 1];
					s->bind(hStmt, sqlcol, SQL_C_CHAR, s->value, bindSize + 1);
					columns[col] = s;
					break;
				}
			case SQL_DECIMAL:
			case SQL_NUMERIC:
			case SQL_REAL:
			case SQL_FLOAT:
			case SQL_DOUBLE:
				{
					_Column<SQLDOUBLE>* d = new _Column<SQLDOUBLE>(colName, col);
					d->bind(hStmt, sqlcol, SQL_C_DOUBLE, &d->value, sizeof(double));
					columns[col] = d;
					break;
				}
			case SQL_SMALLINT:
			case SQL_INTEGER:
			case SQL_TINYINT:
			case SQL_BIGINT:
				{
					_Column<SQLINTEGER>* i = new _Column<SQLINTEGER>(colName, col);
					i->bind(hStmt, sqlcol, SQL_C_LONG, &i->value, sizeof(int));
					columns[col] = i;
					break;
				}
			case SQL_DATETIME:
			case SQL_TYPE_TIME:
			case SQL_TYPE_DATE:
			case SQL_TYPE_TIMESTAMP:
				{
					_Column<TimeTypePair>* t = new _Column<TimeTypePair>(colName, col);
					t->bind(hStmt, sqlcol, SQL_C_TYPE_TIMESTAMP, &t->value.sql(),
							sizeof(SQL_TIMESTAMP_STRUCT));
					columns[col] = t;
					break;
				}
			default:
				throw Error(
						"%s: Bad column type: idx=%d, name=%s, type=%d, size=%ld, dp=%d, null=%d",
						__FUNCTION__, col, colName, bindType, bindSize, dp, nullable);
				break;
		};
	}
}


const ODBC::Column&
ODBC::SelectCommand::operator[](unsigned int col) const
{
	return *columns[col];
}


