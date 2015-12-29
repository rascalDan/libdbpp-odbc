#ifndef ODBC_MODIFYCOMMAND_H
#define ODBC_MODIFYCOMMAND_H

#include <modifycommand.h>
#include "odbc-command.h"

namespace ODBC {
	class ModifyCommand : public Command, public DB::ModifyCommand {
		public:
			ModifyCommand(const Connection &, const std::string & sql);
			~ModifyCommand();
			// Execute the command and return effected row count
			unsigned int		execute(bool allowNoChange = true) override;
	};
}

#endif

