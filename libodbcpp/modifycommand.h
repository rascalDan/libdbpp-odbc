#ifndef ODBC_MODIFYCOMMAND_H
#define ODBC_MODIFYCOMMAND_H

#include "command.h"

namespace ODBC {
	class ModifyCommand : public Command {
		public:
			ModifyCommand(const Connection &, const std::string & sql);
			~ModifyCommand();
			// Execute the command and return effected row count
			unsigned int		execute(bool allowNoChange = true);
	};
}

#endif

