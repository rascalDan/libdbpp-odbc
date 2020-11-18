#ifndef ODBC_SELECTCOMMAND_H
#define ODBC_SELECTCOMMAND_H

#include "odbc-command.h"
#include <selectcommand.h>

namespace ODBC {
	class Column;
	class SelectCommand : public Command, public DB::SelectCommand {
	public:
		SelectCommand(const Connection &, const std::string & sql);
		~SelectCommand();
		bool fetch() override;
		void execute() override;

	private:
		bool fetch(SQLSMALLINT orientation = SQL_FETCH_NEXT, SQLLEN offset = 0);
		typedef std::shared_ptr<Column> ColumnPtr;
		typedef std::set<Column *> Columns;
		Columns largeColumns;
	};
}

#endif
