#ifndef ODBC_SELECTCOMMAND_H
#define ODBC_SELECTCOMMAND_H

#include "command.h"

namespace ODBC {
	class Column;
	class SelectCommand : public Command {
			typedef std::vector<Column*> Columns;
		public:
			SelectCommand (const Connection &, const std::string & sql);
			~SelectCommand();
			bool				fetch(SQLSMALLINT orientation = SQL_FETCH_NEXT, SQLLEN offset = 0);
			const Column &		operator[](unsigned int col) const;
			const Column &		operator[](const Glib::ustring &) const;
			unsigned int		columnCount() const;
			unsigned int		getOrdinal(const Glib::ustring &) const;
		private:
			void				execute();
			Columns				columns;			
	};
}

#endif

