#ifndef DSN_H
#define DSN_H

#include <string>

namespace ODBC {
	class DSN {
		public:
			DSN(const std::string &, const std::string &, const std::string &);
			virtual ~DSN();
			const std::string	dsn;            // DSN name for odbc.ini
			const std::string	username;       // User name
			const std::string	password;       // Password
	};
}

#endif

