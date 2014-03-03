#include "selectcommand.h"
#include "error.h"
#include "column.h"
#include <sqlext.h>
#include <stdio.h>
#include <string.h>

ODBC::SelectCommand::SelectCommand(const Connection & c, const std::string & s) :
	DB::Command(s),
	ODBC::Command(c, s),
	DB::SelectCommand(s)
{
}

ODBC::SelectCommand::~SelectCommand()
{
	for (Columns::iterator i = columns.begin(); i != columns.end(); ++i) {
		if (*i) {
			delete *i;
		}
	}
	if (columns.size()) {
		RETCODE rc = SQLCloseCursor(hStmt);
		if (!SQL_SUCCEEDED(rc)) {
			throw Error(rc, SQL_HANDLE_STMT, hStmt, "ODBC::SelectCommand::~SelectCommand SQLCloseCursor");
		}
	}
}

bool
ODBC::SelectCommand::fetch()
{
	return fetch(SQL_FETCH_NEXT, 0);
}

bool
ODBC::SelectCommand::fetch(SQLSMALLINT orientation, SQLLEN offset)
{
	if (columns.empty()) {
		execute();
	}
	RETCODE rc = SQLFetchScroll(hStmt, orientation, offset);
	switch (rc) {
		case SQL_SUCCESS_WITH_INFO:
		default:
			{
				SQLCHAR sqlstatus[6];
				RETCODE diagrc = SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, sqlstatus, NULL, NULL, 0, NULL);
				if (SQL_SUCCEEDED(diagrc)) {
					if (!strncmp((const char*)sqlstatus, "01004", 5)) {
						for (Columns::iterator i = columns.begin(); i != columns.end(); ++i) {
							(*i)->resize();
						}
						return fetch(SQL_FETCH_RELATIVE, 0);
					}
				}
				if (rc != SQL_SUCCESS_WITH_INFO) {
					throw Error(rc, SQL_HANDLE_STMT, hStmt, "ODBC::SelectCommand::fetch SQLFetch");
				}
			}
		case SQL_SUCCESS:
			{
				bool resized = false;
				for (Columns::iterator i = columns.begin(); i != columns.end(); ++i) {
					resized |= (*i)->resize();
				}
				if (resized) {
					return fetch(SQL_FETCH_RELATIVE, 0);
				}
				return true;
			}
		case SQL_NO_DATA:
			return false;
	}
}

void
ODBC::SelectCommand::execute()
{
	RETCODE rc = SQLExecute(hStmt); 
	if (!SQL_SUCCEEDED(rc)) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "ODBC::SelectCommand::execute SQLExecute");
	}
	SQLSMALLINT colCount;
	if (!SQL_SUCCEEDED(rc = SQLNumResultCols(hStmt, &colCount))) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "ODBC::SelectCommand::execute SQLNumResultCols");
	}
	if (colCount < 1) {
		throw Error("ODBC::SelectCommand::execute No result columns");
	}
	columns.resize(colCount);
	for (int col = 0; col < colCount; col++) {
		SQLCHAR _colName[300];
		SQLSMALLINT nameLen, dp, nullable, bindType;
		SQLULEN bindSize;
		int sqlcol = col + 1;
		if (!SQL_SUCCEEDED(rc = SQLDescribeCol(hStmt, sqlcol, _colName, sizeof(_colName), &nameLen, &bindType,
						&bindSize, &dp, &nullable))) {
			throw Error(rc, SQL_HANDLE_STMT, hStmt, "ODBC::SelectCommand::execute SQLDescribeCol for %d");
		}
		Glib::ustring colName((const char *)_colName, nameLen);
		switch (bindType) {
			case SQL_DECIMAL:
			case SQL_NUMERIC:
			case SQL_REAL:
			case SQL_FLOAT:
			case SQL_DOUBLE:
				columns[col] = new FloatingPointColumn(this, colName, col);
				break;
			case SQL_SMALLINT:
			case SQL_INTEGER:
			case SQL_TINYINT:
			case SQL_BIGINT:
				columns[col] = new SignedIntegerColumn(this, colName, col);
				break;
			case SQL_TYPE_TIME:
			case SQL_INTERVAL_YEAR:
			case SQL_INTERVAL_MONTH:
			case SQL_INTERVAL_DAY:
			case SQL_INTERVAL_HOUR:
			case SQL_INTERVAL_MINUTE:
			case SQL_INTERVAL_SECOND:
			case SQL_INTERVAL_YEAR_TO_MONTH:
			case SQL_INTERVAL_DAY_TO_HOUR:
			case SQL_INTERVAL_DAY_TO_MINUTE:
			case SQL_INTERVAL_DAY_TO_SECOND:
			case SQL_INTERVAL_HOUR_TO_MINUTE:
			case SQL_INTERVAL_HOUR_TO_SECOND:
			case SQL_INTERVAL_MINUTE_TO_SECOND:
				columns[col] = new IntervalColumn(this, colName, col);
				break;
			case SQL_TIMESTAMP:
			case SQL_DATETIME:
			case SQL_TYPE_DATE:
			case SQL_TYPE_TIMESTAMP:
				columns[col] = new TimeStampColumn(this, colName, col);
				break;
			default:
				columns[col] = new CharArrayColumn(this, colName, col, bindSize);
				break;
		};
		columns[col]->bind();
	}
}


const DB::Column&
ODBC::SelectCommand::operator[](unsigned int col) const
{
	if (col > columns.size()) {
		throw ODBC::Error("Column index out of range");
	}
	return *columns[col];
}

const DB::Column&
ODBC::SelectCommand::operator[](const Glib::ustring & colName) const
{
	for (Columns::const_iterator col = columns.begin(); col != columns.end(); ++col) {
		if ((*col)->name == colName) {
			return **col;
		}
	}
	throw ODBC::Error("Column does not exist");
}

unsigned int
ODBC::SelectCommand::getOrdinal(const Glib::ustring & colName) const
{
	unsigned int n = 0;
	for (Columns::const_iterator col = columns.begin(); col != columns.end(); ++col) {
		if ((*col)->name == colName) {
			return n;
		}
		n += 1;
	}
	throw ODBC::Error("Column does not exist");
}

unsigned int
ODBC::SelectCommand::columnCount() const
{
	return columns.size();
}


