#ifndef ODBC_PARAM_FWD_H
#define ODBC_PARAM_FWD_H

#include "odbc-bind.h"
#include <sqlext.h>

namespace ODBC {
	class Command;
	class Param : public virtual Bind {
	public:
		Param();
		Param(Command *, unsigned int idx);
		void bind() const;

		virtual SQLSMALLINT stype() const = 0; // The SQL type ID
		virtual SQLSMALLINT dp() const = 0; // The decimal place count
		virtual const void * dataAddress() const = 0; // The address of the data

	protected:
		friend class Column;
		mutable Command * paramCmd;
		mutable unsigned int paramIdx;
		mutable bool paramBound; // Has SqlBind(...) been called since last change of address?
		SQLLEN dataLength;
	};
}

#endif
