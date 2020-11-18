#ifndef ODBC_PARAM_H
#define ODBC_PARAM_H

#include "odbc-param_fwd.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <glibmm/ustring.h>
#include <malloc.h>
#include <sqlext.h>

namespace ODBC {
	class BooleanParam : public Param {
	public:
		BooleanParam() : Param() { }
		BooleanParam(Command * c, unsigned int i) : Param(c, i)
		{
			bindLen = BooleanParam::size();
		}
		virtual SQLSMALLINT
		ctype() const override
		{
			return SQL_C_BIT;
		}
		virtual SQLSMALLINT
		stype() const override
		{
			return SQL_C_BIT;
		}
		virtual SQLULEN
		size() const override
		{
			return sizeof(SQLINTEGER);
		}
		virtual SQLINTEGER
		dp() const override
		{
			return 0;
		}
		virtual const void *
		dataAddress() const override
		{
			return &data;
		}
		BooleanParam &
		operator=(const SQLINTEGER & d)
		{
			data = d;
			return *this;
		}

	protected:
		SQLINTEGER data;
	};
	class SignedIntegerParam : public Param {
	public:
		SignedIntegerParam() : Param() { }
		SignedIntegerParam(Command * c, unsigned int i) : Param(c, i)
		{
			bindLen = SignedIntegerParam::size();
		}
		virtual SQLSMALLINT
		ctype() const override
		{
			return SQL_C_LONG;
		}
		virtual SQLSMALLINT
		stype() const override
		{
			return SQL_C_LONG;
		}
		virtual SQLULEN
		size() const override
		{
			return sizeof(SQLINTEGER);
		}
		virtual SQLINTEGER
		dp() const override
		{
			return 0;
		}
		virtual const void *
		dataAddress() const override
		{
			return &data;
		}
		SignedIntegerParam &
		operator=(const SQLINTEGER & d)
		{
			data = d;
			return *this;
		}

	protected:
		SQLINTEGER data;
	};
	class UnsignedIntegerParam : public Param {
	public:
		UnsignedIntegerParam() : Param() { }
		UnsignedIntegerParam(Command * c, unsigned int i) : Param(c, i)
		{
			bindLen = UnsignedIntegerParam::size();
		}
		virtual SQLSMALLINT
		ctype() const override
		{
			return SQL_C_ULONG;
		}
		virtual SQLSMALLINT
		stype() const override
		{
			return SQL_C_ULONG;
		}
		virtual SQLULEN
		size() const override
		{
			return sizeof(SQLUINTEGER);
		}
		virtual SQLINTEGER
		dp() const override
		{
			return 0;
		}
		virtual const void *
		dataAddress() const override
		{
			return &data;
		}
		UnsignedIntegerParam &
		operator=(const SQLUINTEGER & d)
		{
			data = d;
			return *this;
		}

	protected:
		SQLUINTEGER data;
	};
	class FloatingPointParam : public Param {
	public:
		FloatingPointParam() : Param() { }
		FloatingPointParam(Command * c, unsigned int i) : Param(c, i)
		{
			bindLen = FloatingPointParam::size();
		}
		virtual SQLSMALLINT
		ctype() const override
		{
			return SQL_C_DOUBLE;
		}
		virtual SQLSMALLINT
		stype() const override
		{
			return SQL_C_DOUBLE;
		}
		virtual SQLULEN
		size() const override
		{
			return sizeof(SQLDOUBLE);
		}
		virtual SQLINTEGER
		dp() const override
		{
			return 10;
		}
		virtual const void *
		dataAddress() const override
		{
			return &data;
		}
		FloatingPointParam &
		operator=(const SQLDOUBLE & d)
		{
			data = d;
			return *this;
		}

	protected:
		SQLDOUBLE data;
	};
	class StdStringParam : public Param {
	public:
		StdStringParam() : Param() { }
		StdStringParam(Command * c, unsigned int i) : Param(c, i)
		{
			bindLen = StdStringParam::size();
		}
		virtual SQLSMALLINT
		ctype() const override
		{
			return SQL_C_CHAR;
		}
		virtual SQLSMALLINT
		stype() const override
		{
			return SQL_CHAR;
		}
		virtual SQLULEN
		size() const override
		{
			return data.length();
		}
		virtual SQLINTEGER
		dp() const override
		{
			return 0;
		}
		virtual const void *
		dataAddress() const override
		{
			return data.data();
		}
		StdStringParam & operator=(const std::string_view & d);
		StdStringParam & operator=(const Glib::ustring & d);

	protected:
		std::string data;
	};
	class IntervalParam : public Param {
	public:
		IntervalParam() : Param() { }
		IntervalParam(Command * c, unsigned int i) : Param(c, i)
		{
			bindLen = IntervalParam::size();
		}
		virtual SQLSMALLINT
		ctype() const override
		{
			return SQL_C_INTERVAL_DAY_TO_SECOND;
		}
		virtual SQLSMALLINT
		stype() const override
		{
			return SQL_INTERVAL_DAY_TO_SECOND;
		}
		virtual SQLULEN
		size() const override
		{
			return sizeof(SQL_INTERVAL_STRUCT);
		}
		virtual SQLINTEGER
		dp() const override
		{
			return boost::posix_time::time_res_traits::num_fractional_digits();
		}
		virtual const void *
		dataAddress() const override
		{
			return &data;
		}
		IntervalParam & operator=(const boost::posix_time::time_duration & d);

	protected:
		SQL_INTERVAL_STRUCT data;
	};
	class TimeStampParam : public Param {
	public:
		TimeStampParam() : Param() { }
		TimeStampParam(Command * c, unsigned int i) : Param(c, i)
		{
			bindLen = TimeStampParam::size();
		}
		virtual SQLSMALLINT
		ctype() const override
		{
			return SQL_C_TYPE_TIMESTAMP;
		}
		virtual SQLSMALLINT
		stype() const override
		{
			return SQL_TYPE_TIMESTAMP;
		}
		virtual SQLULEN
		size() const override
		{
			return sizeof(SQL_TIMESTAMP_STRUCT);
		}
		virtual SQLINTEGER
		dp() const override
		{
			return boost::posix_time::time_res_traits::num_fractional_digits();
		}
		virtual const void *
		dataAddress() const override
		{
			return &data;
		}
		TimeStampParam & operator=(const boost::posix_time::ptime & d);

	protected:
		SQL_TIMESTAMP_STRUCT data;
	};
	class NullParam : public Param {
	public:
		NullParam() : Param() { }
		NullParam(Command * c, unsigned int i) : Param(c, i)
		{
			bindLen = SQL_NULL_DATA;
		}
		virtual SQLSMALLINT
		ctype() const override
		{
			return SQL_C_LONG;
		}
		virtual SQLSMALLINT
		stype() const override
		{
			return SQL_C_LONG;
		}
		virtual SQLULEN
		size() const override
		{
			return 0;
		}
		virtual SQLINTEGER
		dp() const override
		{
			return 0;
		}
		virtual const void *
		dataAddress() const override
		{
			return NULL;
		}
	};
}

#endif
