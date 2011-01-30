#ifndef ODBC_PARAM_H
#define ODBC_PARAM_H

#include <malloc.h>
#include <sqlext.h>
#include <glibmm/ustring.h>
#include "bind.h"

namespace ODBC {
	class Command;
	class Param : public virtual Bind {
		public:
			Param();
			Param(Command *, unsigned int idx);
			virtual	~Param() = 0;
			void bind() const;

			virtual SQLSMALLINT stype() const = 0; // The SQL type ID
			virtual SQLINTEGER dp() const = 0; // The decimal place count
			virtual const void * dataAddress() const = 0; // The address of the data

		protected:
			friend class Column;
			mutable Command * paramCmd;
			mutable unsigned int paramIdx;
			mutable bool paramBound;				// Has SqlBind(...) been called since last change of address?
			SQLLEN dataLength;
	};

	class SignedIntegerParam : public Param {
		public:
			SignedIntegerParam() : Param() { }
			SignedIntegerParam(Command * c, unsigned int i) : Param(c, i) { bindLen = size(); }
			virtual SQLSMALLINT ctype() const { return SQL_C_LONG; }
			virtual SQLSMALLINT stype() const { return SQL_C_LONG; }
			virtual SQLULEN size() const { return sizeof(SQLINTEGER); }
			virtual SQLINTEGER dp() const { return 0; }
			virtual const void * dataAddress() const { return &data; }
			void operator=(const SQLINTEGER & d) { data = d; }
		protected:
			SQLINTEGER data;
	};
	class UnsignedIntegerParam : public Param {
		public:
			UnsignedIntegerParam() : Param() { }
			UnsignedIntegerParam(Command * c, unsigned int i) : Param(c, i) { bindLen = size(); }
			virtual SQLSMALLINT ctype() const { return SQL_C_ULONG; }
			virtual SQLSMALLINT stype() const { return SQL_C_ULONG; }
			virtual SQLULEN size() const { return sizeof(SQLUINTEGER); }
			virtual SQLINTEGER dp() const { return 0; }
			virtual const void * dataAddress() const { return &data; }
			void operator=(const SQLUINTEGER & d) { data = d; }
		protected:
			SQLUINTEGER data;
	};
	class FloatingPointParam : public Param {
		public:
			FloatingPointParam() : Param() { }
			FloatingPointParam(Command * c, unsigned int i) : Param(c, i) { bindLen = size(); }
			virtual SQLSMALLINT ctype() const { return SQL_C_DOUBLE; }
			virtual SQLSMALLINT stype() const { return SQL_C_DOUBLE; }
			virtual SQLULEN size() const { return sizeof(SQLDOUBLE); }
			virtual SQLINTEGER dp() const { return 10; }
			virtual const void * dataAddress() const { return &data; }
			void operator=(const SQLDOUBLE & d) { data = d; }
		protected:
			SQLDOUBLE data;
	};
	class GlibUstringParam : public Param {
		public:
			GlibUstringParam() : Param() { }
			GlibUstringParam(Command * c, unsigned int i) : Param(c, i) { bindLen = size(); }
			virtual SQLSMALLINT ctype() const { return SQL_C_CHAR; }
			virtual SQLSMALLINT stype() const { return SQL_CHAR; }
			virtual SQLULEN size() const { return data.bytes(); }
			virtual SQLINTEGER dp() const { return 0; }
			virtual const void * dataAddress() const { return data.data(); }
			void operator=(const Glib::ustring & d);
		protected:
			Glib::ustring data;
	};
	class TimeStampParam : public Param {
		public:
			TimeStampParam() : Param() { }
			TimeStampParam(Command * c, unsigned int i) : Param(c, i) { bindLen = size(); }
			virtual SQLSMALLINT ctype() const { return SQL_C_TYPE_TIMESTAMP; }
			virtual SQLSMALLINT stype() const { return SQL_TYPE_TIMESTAMP; }
			virtual SQLULEN size() const { return sizeof(SQL_TIMESTAMP_STRUCT); }
			virtual SQLINTEGER dp() const { return 0; }
			virtual const void * dataAddress() const { return &data; }
			void operator=(const time_t & d);
			void operator=(const struct tm * d) { data << *d; }
			void operator=(const SQL_TIMESTAMP_STRUCT & d) { data = d; }
		protected:
			SQL_TIMESTAMP_STRUCT data;
	};
	class NullParam : public Param {
		public:
			NullParam() : Param() { }
			NullParam(Command * c, unsigned int i) : Param(c, i) { bindLen = SQL_NULL_DATA; }
			virtual SQLSMALLINT ctype() const { return SQL_C_LONG; }
			virtual SQLSMALLINT stype() const { return SQL_C_LONG; }
			virtual SQLULEN size() const { return 0; }
			virtual SQLINTEGER dp() const { return 0; }
			virtual const void * dataAddress() const { return NULL; }
	};
}

#endif

