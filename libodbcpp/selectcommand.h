#ifndef ODBC_SELECTCOMMAND_H
#define ODBC_SELECTCOMMAND_H

#include "command.h"

namespace ODBC {
	class Column;
	class SelectCommand : public Command {
			typedef std::vector<Column*> Columns;
		public:
			SelectCommand (const Connection&, String sql);
			~SelectCommand();
			bool				fetch();
			const Column &		operator[](unsigned int col) const;
			const Column &		operator[](const String &) const;
			unsigned int		columnCount() const;
		private:
			void				execute();
			Columns				columns;			
	};
}

#endif

