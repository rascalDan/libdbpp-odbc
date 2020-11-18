#ifndef ODBC_MODIFYCOMMAND_H
#define ODBC_MODIFYCOMMAND_H

#include "odbc-command.h"
#include <modifycommand.h>

namespace ODBC {
	class ModifyCommand : public Command, public DB::ModifyCommand {
	public:
		ModifyCommand(const Connection &, const std::string & sql);
		// Execute the command and return effected row count
		unsigned int execute(bool allowNoChange = true) override;
	};
}

#endif
