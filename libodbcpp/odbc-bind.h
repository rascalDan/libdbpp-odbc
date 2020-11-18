#ifndef ODBC_BIND_H
#define ODBC_BIND_H

#include <sql.h>
#include <vector>

namespace ODBC {
	class Bind {
	public:
		virtual ~Bind() = default;

		virtual SQLSMALLINT ctype() const = 0; // The C type ID
		virtual SQLULEN size() const = 0; // The size of the data
	protected:
		mutable SQLLEN bindLen; // How much data the driver wants to store
	};
}

#endif
