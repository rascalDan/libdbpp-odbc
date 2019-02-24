#include "odbc-selectcommand.h"
#include "odbc-error.h"
#include "odbc-column.h"
#include <sqlext.h>
#include <cstring>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>

ODBC::SelectCommand::SelectCommand(const Connection & c, const std::string & s) :
	DB::Command(s),
	ODBC::Command(c, s),
	DB::SelectCommand(s)
{
}

ODBC::SelectCommand::~SelectCommand()
{
	if (columnCount()) {
		SQLCloseCursor(hStmt);
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
	if (!columnCount()) {
		execute();
	}
	RETCODE rc = SQLFetchScroll(hStmt, orientation, offset);
	switch (rc) {
		case SQL_SUCCESS_WITH_INFO:
		default:
			{
				SQLCHAR sqlstatus[6];
				// NOLINTNEXTLINE(hicpp-no-array-decay)
				RETCODE diagrc = SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, sqlstatus, nullptr, nullptr, 0, nullptr);
				if (SQL_SUCCEEDED(diagrc)) {
					if (!strncmp((const char*)sqlstatus, "01004", 5)) {
						for (const auto & c : largeColumns) {
							c->resize();
						}
						return fetch(SQL_FETCH_RELATIVE, 0);
					}
				}
			}
			[[ fallthrough ]];
		case SQL_SUCCESS:
			{
				bool resized = false;
				for (const auto & c : largeColumns) {
					resized |= c->resize();
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
		throw Error(rc, SQL_HANDLE_STMT, hStmt);
	}
	SQLSMALLINT colCount;
	if (!SQL_SUCCEEDED(rc = SQLNumResultCols(hStmt, &colCount))) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt);
	}
	for (int col = 0; col < colCount; col++) {
		SQLCHAR _colName[300];
		SQLSMALLINT nameLen, dp, nullable, bindType;
		SQLULEN bindSize;
		int sqlcol = col + 1;
		// NOLINTNEXTLINE(hicpp-no-array-decay)
		if (!SQL_SUCCEEDED(rc = SQLDescribeCol(hStmt, sqlcol, _colName, sizeof(_colName), &nameLen, &bindType,
						&bindSize, &dp, &nullable))) {
			throw Error(rc, SQL_HANDLE_STMT, hStmt);
		}
		Glib::ustring colName((const char *)_colName, nameLen);
		DB::Column * ncol;
		switch (bindType) {
			case SQL_DECIMAL:
			case SQL_NUMERIC:
			case SQL_REAL:
			case SQL_FLOAT:
			case SQL_DOUBLE:
				ncol = insertColumn(std::make_unique<FloatingPointColumn>(this, colName, col)).get();
				break;
			case SQL_SMALLINT:
			case SQL_INTEGER:
			case SQL_TINYINT:
			case SQL_BIGINT:
				ncol = insertColumn(std::make_unique<SignedIntegerColumn>(this, colName, col)).get();
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
				ncol = insertColumn(std::make_unique<IntervalColumn>(this, colName, col)).get();
				break;
			case SQL_TIMESTAMP:
			case SQL_DATETIME:
			case SQL_TYPE_DATE:
			case SQL_TYPE_TIMESTAMP:
				ncol = insertColumn(std::make_unique<TimeStampColumn>(this, colName, col)).get();
				break;
			case SQL_UNKNOWN_TYPE:
				throw DB::ColumnTypeNotSupported();
			default:
				SQLLEN octetSize = 0;
				if (!SQL_SUCCEEDED(rc = SQLColAttribute(hStmt, sqlcol, SQL_DESC_OCTET_LENGTH, nullptr, 0, nullptr, &octetSize))) {
					throw Error(rc, SQL_HANDLE_STMT, hStmt);
				}
				bindSize = octetSize;
				auto lc = std::make_unique<CharArrayColumn>(this, colName, col, bindSize);
				largeColumns.insert(lc.get());
				ncol = insertColumn(std::move(lc)).get();
				break;
		};
		dynamic_cast<Column *>(ncol)->bind();
	}
}

