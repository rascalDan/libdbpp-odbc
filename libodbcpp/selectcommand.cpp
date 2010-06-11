#include "selectcommand.h"
#include "error.h"
#include "column.h"
#include <sqlext.h>
#include <stdio.h>

ODBC::SelectCommand::SelectCommand(const Connection & c, const std::string & s) :
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
	if (columns.size()) {
		RETCODE rc;
		if ((rc = SQLCloseCursor(hStmt)) != SQL_SUCCESS) {
			throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: SQLCloseCursor",
					__FUNCTION__);
		}
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
ODBC::_Column<t>::_Column(const Glib::ustring & n, unsigned int i) : Column(n, i)
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
		SQLCHAR _colName[300];
		SQLSMALLINT nameLen, dp, nullable, bindType;
		SQLUINTEGER bindSize;
		int sqlcol = col + 1;
		if ((rc = SQLDescribeCol(hStmt, sqlcol, _colName, sizeof(_colName), &nameLen, &bindType,
					&bindSize, &dp, &nullable)) != SQL_SUCCESS) {
			throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: SQLDescribeCol for %d",
					__FUNCTION__, col);
		}
		Glib::ustring colName((const char *)_colName, nameLen);
		switch (bindType) {
			case -9:
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
			case SQL_TIMESTAMP:
			case SQL_DATETIME:
			case SQL_TYPE_TIME:
			case SQL_TYPE_DATE:
			case SQL_TYPE_TIMESTAMP:
				{
					_Column<SQL_TIMESTAMP_STRUCT>* t = new _Column<SQL_TIMESTAMP_STRUCT>(colName, col);
					t->bind(hStmt, sqlcol, SQL_C_TIMESTAMP, &t->value, sizeof(SQL_TIMESTAMP_STRUCT));
					columns[col] = t;
					break;
				}
			default:
				throw Error(
						"%s: Bad column type: idx=%d, name=%s, type=%d, size=%ld, dp=%d, null=%d",
						__FUNCTION__, col, _colName, bindType, bindSize, dp, nullable);
				break;
		};
	}
}


const ODBC::Column&
ODBC::SelectCommand::operator[](unsigned int col) const
{
	if (col > columns.size()) {
		throw ODBC::Error("Column index (%u) out of range", col);
	}
	return *columns[col];
}

const ODBC::Column&
ODBC::SelectCommand::operator[](const Glib::ustring & colName) const
{
	for (Columns::const_iterator col = columns.begin(); col != columns.end(); col++) {
		if ((*col)->name == colName) {
			return **col;
		}
	}
	throw ODBC::Error("Column (%s) does not exist", colName.c_str());
}

unsigned int
ODBC::SelectCommand::getOrdinal(const Glib::ustring & colName) const
{
	unsigned int n = 0;
	for (Columns::const_iterator col = columns.begin(); col != columns.end(); col++) {
		if ((*col)->name == colName) {
			return n;
		}
		n += 1;
	}
	throw ODBC::Error("Column (%s) does not exist", colName.c_str());
}

unsigned int
ODBC::SelectCommand::columnCount() const
{
	return columns.size();
}


