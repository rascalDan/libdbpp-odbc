#ifndef ODBC_SELECTCOMMAND_H
#define ODBC_SELECTCOMMAND_H

#include <selectcommand.h>
#include "command.h"

namespace ODBC {
	class Column;
	class SelectCommand : public Command, public DB::SelectCommand {
		public:
			SelectCommand (const Connection &, const std::string & sql);
			~SelectCommand();
			bool				fetch();
			void				execute();

		private:
			bool				fetch(SQLSMALLINT orientation = SQL_FETCH_NEXT, SQLLEN offset = 0);
	};
}

#endif

