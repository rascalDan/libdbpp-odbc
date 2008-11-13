#ifndef ODBC_ERROR_H
#define ODBC_ERROR_H

#include <sql.h>
#include <exception>

namespace ODBC {
	class Error : public std::exception {
		public:
			Error(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle, char const * action, ...)
				__attribute__((format(printf, 5, 6)));
			Error(char const * action, ...)
				__attribute__((format(printf, 2, 3)));
			~Error() throw();

			const char * what() const throw();
		private:
			char * msg;
	};
}

#endif
