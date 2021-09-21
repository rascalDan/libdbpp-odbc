#ifndef ODBC_SELECTCOMMAND_H
#define ODBC_SELECTCOMMAND_H

#include "odbc-command.h"
#include <memory>
#include <selectcommand.h>
#include <set>
#include <sql.h>
#include <string>

namespace ODBC {
	class Connection;
	class Column;
	class SelectCommand : public Command, public DB::SelectCommand {
	public:
		SelectCommand(const Connection &, const std::string & sql);
		~SelectCommand();
		bool fetch() override;
		void execute() override;

	private:
		bool fetch(SQLSMALLINT orientation = SQL_FETCH_NEXT, SQLLEN offset = 0);
		using ColumnPtr = std::shared_ptr<Column>;
		using Columns = std::set<Column *>;
		Columns largeColumns;
	};
}

#endif
