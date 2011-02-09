#ifndef ODBC_SELECTCOMMAND_H
#define ODBC_SELECTCOMMAND_H

#include "../libdbpp/selectcommand.h"
#include "command.h"

namespace ODBC {
	class Column;
	class SelectCommand : public Command, public DB::SelectCommand {
			typedef std::vector<Column*> Columns;
		public:
			SelectCommand (const Connection &, const std::string & sql);
			~SelectCommand();
			bool				fetch();
			void				execute();
			const DB::Column &	operator[](unsigned int col) const;
			const DB::Column &	operator[](const Glib::ustring &) const;
			unsigned int		columnCount() const;
			unsigned int		getOrdinal(const Glib::ustring &) const;
		private:
			bool				fetch(SQLSMALLINT orientation = SQL_FETCH_NEXT, SQLLEN offset = 0);
			Columns				columns;			
	};
}

#endif

