#ifndef DSN_H
#define DSN_H

#include "string.h"

namespace ODBC {
	class DSN {
		public:
			DSN(String, String, String);
			virtual ~DSN();
			String     	dsn;            // DSN name for odbc.ini
			String     	username;       // User name
			String     	password;       // Password
	};
}

#endif

