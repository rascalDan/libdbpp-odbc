#ifndef ODBC_ERROR_H
#define ODBC_ERROR_H

#include <sql.h>
#include <stdlib.h>
#include <exception.h>
#include <error.h>

namespace ODBC {
	class Error : public AdHoc::Exception<DB::Error> {
		public:
			Error(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle);

			std::string message() const throw() override;

		private:
			std::string msg;
	};
}

#endif
