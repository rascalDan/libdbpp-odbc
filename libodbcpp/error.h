#ifndef ODBC_ERROR_H
#define ODBC_ERROR_H

#include <sql.h>
#include <stdlib.h>
#include <exception>
#include "../libdbpp/error.h"

namespace ODBC {
	class Error : public DB::Error {
		public:
			Error(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle, char const * action);
			Error(char const * action);
			~Error() throw();

			const char * what() const throw();
		protected:
			Error(char * msg);
		private:
			char * msg;
	};
}

#endif
