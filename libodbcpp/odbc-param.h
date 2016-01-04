#ifndef ODBC_PARAM_H
#define ODBC_PARAM_H

#include <malloc.h>
#include <sqlext.h>
#include <glibmm/ustring.h>
#include "odbc-bind.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>

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

	class BooleanParam : public Param {
		public:
			BooleanParam() : Param() { }
			BooleanParam(Command * c, unsigned int i) : Param(c, i) { bindLen = size(); }
			virtual SQLSMALLINT ctype() const override { return SQL_C_BIT; }
			virtual SQLSMALLINT stype() const override { return SQL_C_BIT; }
			virtual SQLULEN size() const override { return sizeof(SQLINTEGER); }
			virtual SQLINTEGER dp() const override { return 0; }
			virtual const void * dataAddress() const override { return &data; }
			void operator=(const SQLINTEGER & d) { data = d; }
		protected:
			SQLINTEGER data;
	};
	class SignedIntegerParam : public Param {
		public:
			SignedIntegerParam() : Param() { }
			SignedIntegerParam(Command * c, unsigned int i) : Param(c, i) { bindLen = size(); }
			virtual SQLSMALLINT ctype() const override { return SQL_C_LONG; }
			virtual SQLSMALLINT stype() const override { return SQL_C_LONG; }
			virtual SQLULEN size() const override { return sizeof(SQLINTEGER); }
			virtual SQLINTEGER dp() const override { return 0; }
			virtual const void * dataAddress() const override { return &data; }
			void operator=(const SQLINTEGER & d) { data = d; }
		protected:
			SQLINTEGER data;
	};
	class UnsignedIntegerParam : public Param {
		public:
			UnsignedIntegerParam() : Param() { }
			UnsignedIntegerParam(Command * c, unsigned int i) : Param(c, i) { bindLen = size(); }
			virtual SQLSMALLINT ctype() const override { return SQL_C_ULONG; }
			virtual SQLSMALLINT stype() const override { return SQL_C_ULONG; }
			virtual SQLULEN size() const override { return sizeof(SQLUINTEGER); }
			virtual SQLINTEGER dp() const override { return 0; }
			virtual const void * dataAddress() const override { return &data; }
			void operator=(const SQLUINTEGER & d) { data = d; }
		protected:
			SQLUINTEGER data;
	};
	class FloatingPointParam : public Param {
		public:
			FloatingPointParam() : Param() { }
			FloatingPointParam(Command * c, unsigned int i) : Param(c, i) { bindLen = size(); }
			virtual SQLSMALLINT ctype() const override { return SQL_C_DOUBLE; }
			virtual SQLSMALLINT stype() const override { return SQL_C_DOUBLE; }
			virtual SQLULEN size() const override { return sizeof(SQLDOUBLE); }
			virtual SQLINTEGER dp() const override { return 10; }
			virtual const void * dataAddress() const override { return &data; }
			void operator=(const SQLDOUBLE & d) { data = d; }
		protected:
			SQLDOUBLE data;
	};
	class GlibUstringParam : public Param {
		public:
			GlibUstringParam() : Param() { }
			GlibUstringParam(Command * c, unsigned int i) : Param(c, i) { bindLen = size(); }
			virtual SQLSMALLINT ctype() const override { return SQL_C_CHAR; }
			virtual SQLSMALLINT stype() const override { return SQL_CHAR; }
			virtual SQLULEN size() const override { return data.bytes(); }
			virtual SQLINTEGER dp() const override { return 0; }
			virtual const void * dataAddress() const override { return data.data(); }
			void operator=(const Glib::ustring & d);
		protected:
			Glib::ustring data;
	};
	class IntervalParam : public Param {
		public:
			IntervalParam() : Param() { }
			IntervalParam(Command * c, unsigned int i) : Param(c, i) { bindLen = size(); }
			virtual SQLSMALLINT ctype() const override { return SQL_C_INTERVAL_DAY_TO_SECOND; }
			virtual SQLSMALLINT stype() const override { return SQL_INTERVAL_DAY_TO_SECOND; }
			virtual SQLULEN size() const override { return sizeof(SQL_INTERVAL_STRUCT); }
			virtual SQLINTEGER dp() const override { return boost::posix_time::time_res_traits::num_fractional_digits(); }
			virtual const void * dataAddress() const override { return &data; }
			void operator=(const boost::posix_time::time_duration & d);
		protected:
			SQL_INTERVAL_STRUCT data;
	};
	class TimeStampParam : public Param {
		public:
			TimeStampParam() : Param() { }
			TimeStampParam(Command * c, unsigned int i) : Param(c, i) { bindLen = size(); }
			virtual SQLSMALLINT ctype() const override { return SQL_C_TYPE_TIMESTAMP; }
			virtual SQLSMALLINT stype() const override { return SQL_TYPE_TIMESTAMP; }
			virtual SQLULEN size() const override { return sizeof(SQL_TIMESTAMP_STRUCT); }
			virtual SQLINTEGER dp() const override { return boost::posix_time::time_res_traits::num_fractional_digits(); }
			virtual const void * dataAddress() const override { return &data; }
			void operator=(const boost::posix_time::ptime & d);
		protected:
			SQL_TIMESTAMP_STRUCT data;
	};
	class NullParam : public Param {
		public:
			NullParam() : Param() { }
			NullParam(Command * c, unsigned int i) : Param(c, i) { bindLen = SQL_NULL_DATA; }
			virtual SQLSMALLINT ctype() const override { return SQL_C_LONG; }
			virtual SQLSMALLINT stype() const override { return SQL_C_LONG; }
			virtual SQLULEN size() const override { return 0; }
			virtual SQLINTEGER dp() const override { return 0; }
			virtual const void * dataAddress() const override { return NULL; }
	};
}

#endif
