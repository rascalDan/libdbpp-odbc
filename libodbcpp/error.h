#ifndef ODBC_ERROR_H
#define ODBC_ERROR_H

#include <sql.h>

namespace ODBC {
	class Error {
		public:
			Error(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle, char const * action, ...)
				__attribute__((format(printf, 5, 6)));
			Error(char const * action, ...)
				__attribute__((format(printf, 2, 3)));
	};
}

#endif
