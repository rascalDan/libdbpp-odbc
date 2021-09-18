#include "odbc-selectcommand.h"
#include "odbc-column.h"
#include "odbc-error.h"
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <cstring>
#include <numeric>
#include <sqlext.h>

ODBC::SelectCommand::SelectCommand(const Connection & c, const std::string & s) :
	DB::Command(s), ODBC::Command(c, s), DB::SelectCommand(s)
{
}

ODBC::SelectCommand::~SelectCommand()
{
	if (columnCount()) {
		SQLCloseCursor(hStmt);
	}
}

constexpr std::array<SQLCHAR, 6> truncated = {'0', '1', '0', '0', '4', '\0'};
bool
ODBC::SelectCommand::fetch()
{
	if (!columnCount()) {
		execute();
	}
	for (RETCODE rc = SQLFetchScroll(hStmt, SQL_FETCH_NEXT, 0); rc != SQL_NO_DATA;
			rc = SQLFetchScroll(hStmt, SQL_FETCH_RELATIVE, 0)) {
		if (!SQL_SUCCEEDED(rc)) {
			throw Error(rc, SQL_HANDLE_STMT, hStmt);
		}
		if (rc != SQL_SUCCESS) {
			std::array<SQLCHAR, 6> sqlstatus {};
			RETCODE diagrc = SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, sqlstatus.data(), nullptr, nullptr, 0, nullptr);
			if (SQL_SUCCEEDED(diagrc)) {
				if (sqlstatus == truncated) {
					for (const auto & c : largeColumns) {
						c->resize();
					}
					continue;
				}
			}
		}
		if (std::accumulate(largeColumns.begin(), largeColumns.end(), false, [](auto && resized, auto && c) {
				return resized |= c->resize();
			})) {
			continue;
		}
		return true;
	}
	return false;
}

void
ODBC::SelectCommand::execute()
{
	SQLRETURN rc = SQLExecute(hStmt);
	if (!SQL_SUCCEEDED(rc)) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt);
	}
	SQLSMALLINT colCount;
	if (!SQL_SUCCEEDED(rc = SQLNumResultCols(hStmt, &colCount))) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt);
	}
	for (SQLUSMALLINT col = 0; col < colCount; col++) {
		std::array<SQLCHAR, 300> _colName {};
		SQLSMALLINT nameLen, dp, nullable, bindType;
		SQLULEN bindSize;
		SQLUSMALLINT sqlcol = col + 1;
		// NOLINTNEXTLINE(hicpp-no-array-decay)
		if (!SQL_SUCCEEDED(rc = SQLDescribeCol(hStmt, sqlcol, _colName.data(), _colName.size(), &nameLen, &bindType,
								   &bindSize, &dp, &nullable))) {
			throw Error(rc, SQL_HANDLE_STMT, hStmt);
		}
		Glib::ustring colName(
				reinterpret_cast<const char *>(_colName.data()), static_cast<Glib::ustring::size_type>(nameLen));
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
				if (!SQL_SUCCEEDED(rc
							= SQLColAttribute(hStmt, sqlcol, SQL_DESC_OCTET_LENGTH, nullptr, 0, nullptr, &octetSize))) {
					throw Error(rc, SQL_HANDLE_STMT, hStmt);
				}
				bindSize = static_cast<decltype(bindSize)>(octetSize);
				auto lc = std::make_unique<CharArrayColumn>(this, colName, col, bindSize);
				largeColumns.insert(lc.get());
				ncol = insertColumn(std::move(lc)).get();
				break;
		};
		dynamic_cast<Column *>(ncol)->bind();
	}
}
