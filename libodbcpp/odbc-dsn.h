#ifndef DSN_H
#define DSN_H

#include <string>

namespace ODBC {
	class DSN {
	public:
		DSN(std::string, std::string, std::string);
		virtual ~DSN() = default;
		const std::string dsn; // DSN name for odbc.ini
		const std::string username; // User name
		const std::string password; // Password
	};
}

#endif
