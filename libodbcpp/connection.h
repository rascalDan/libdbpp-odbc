#ifndef CONNECTION_H
#define CONNECTION_H

#include "dsn.h"
#include <sql.h>

namespace ODBC {
	class Connection : private DSN {
		public:
			Connection(const DSN& d);
			~Connection();
			SQLHENV 		env;
			SQLHDBC 		conn;
	};
}

#endif

