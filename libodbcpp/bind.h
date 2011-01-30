#ifndef ODBC_BIND_H
#define ODBC_BIND_H

#include <sql.h>
#include <vector>

namespace ODBC {
	class Bind {
		public:
			Bind();
			virtual	~Bind() = 0;

			virtual SQLSMALLINT ctype() const = 0; // The C type ID
			virtual SQLULEN size() const = 0; // The size of the data
		protected:
			mutable SQLLEN bindLen;	// How much data the driver wants to store
	};
}

void operator << (SQL_TIMESTAMP_STRUCT & target, const struct tm &);
void operator << (struct tm &, const SQL_TIMESTAMP_STRUCT & target);

#endif

