#ifndef ODBC_COLUMN_H
#define ODBC_COLUMN_H

#include <column.h>
#include <typeinfo>
#include <glibmm/ustring.h>
#include <algorithm>
#include "bind.h"
#include "param.h"

namespace ODBC {
	class SelectCommand;
	class Column : public virtual Bind, public virtual DB::Column {
		public:
			Column(SelectCommand *, const Glib::ustring & s, unsigned int i);
			virtual ~Column() = 0;
			void bind();
			virtual void * rwDataAddress() = 0;
			void rebind(DB::Command *, unsigned int idx) const;
			virtual bool resize();

			virtual operator int () const { throw std::bad_cast(); }
			virtual operator long () const { throw std::bad_cast(); }
			virtual operator long long () const { throw std::bad_cast(); }
			virtual operator unsigned int () const { throw std::bad_cast(); }
			virtual operator unsigned long () const { throw std::bad_cast(); }
			virtual operator unsigned long long () const { throw std::bad_cast(); }

			virtual operator double () const { throw std::bad_cast(); }
			virtual operator float () const { throw std::bad_cast(); }

			virtual operator std::string () const { throw std::bad_cast(); }
			virtual operator Glib::ustring () const { throw std::bad_cast(); }

			virtual operator struct tm () const { throw std::bad_cast(); }
			virtual operator SQL_TIMESTAMP_STRUCT () const { throw std::bad_cast(); }

			bool isNull() const;
			virtual void apply(DB::HandleField &) const = 0;

			const SelectCommand *	selectCmd;
		protected:
			virtual const Param * meAsAParam() const = 0;
	};
	class CharArrayColumn : public Column, public Param {
		public:
			typedef std::vector<char> CharArray;
			CharArrayColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i, SQLULEN sizeHint) :
				DB::Column(n, i),
				Column(sc, n, i)
			{
				data.resize(std::max<SQLULEN>(sizeHint, 64) + 1);
			}
			virtual SQLSMALLINT ctype() const { return SQL_C_CHAR; }
			virtual SQLSMALLINT stype() const { return SQL_CHAR; }
			virtual SQLULEN size() const { return data.size(); }
			virtual SQLINTEGER dp() const { return 0; }
			virtual const void * dataAddress() const { return &data.front(); }
			virtual void * rwDataAddress() { return &data.front(); }
			void operator=(const Glib::ustring & d);
			bool resize();
			virtual operator std::string () const { return std::string(&data.front(), bindLen); }
			virtual operator Glib::ustring () const { return std::string(&data.front(), bindLen); }
			virtual void apply(DB::HandleField &) const;
		protected:
			virtual const Param * meAsAParam() const { return this; }
			CharArray data;
	};
	class SignedIntegerColumn : public Column, public SignedIntegerParam {
		public:
			SignedIntegerColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
				DB::Column(n, i),
				Column(sc, n, i) { }
			virtual SQLSMALLINT ctype() const { return SignedIntegerParam::ctype(); }
			virtual SQLULEN size() const { return SignedIntegerParam::size(); }
			virtual void * rwDataAddress() { return &data; }
			virtual operator int () const { return data; }
			virtual operator long () const { return data; }
			virtual operator long long () const { return data; }
			virtual const Param * meAsAParam() const { return this; }
			virtual void apply(DB::HandleField &) const;
	};
#ifdef COMPLETENESS
	class UnsignedIntegerColumn : public Column, public UnsignedIntegerParam {
		public:
			UnsignedIntegerColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
				Column(sc, n, i) { }
			virtual const Param * meAsAParam() const { return this; }
	};
#endif
	class FloatingPointColumn : public Column, public FloatingPointParam {
		public:
			FloatingPointColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
				DB::Column(n, i),
				Column(sc, n, i) { }
			virtual SQLSMALLINT ctype() const { return FloatingPointParam::ctype(); }
			virtual SQLULEN size() const { return FloatingPointParam::size(); }
			virtual void * rwDataAddress() { return &data; }
			virtual operator double () const { return data; }
			virtual operator float () const { return data; }
			virtual const Param * meAsAParam() const { return this; }
			virtual void apply(DB::HandleField &) const;
	};
	class IntervalColumn : public Column, public IntervalParam {
		public:
			IntervalColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
				DB::Column(n, i),
				Column(sc, n, i) { }
			virtual SQLSMALLINT ctype() const { return IntervalParam::ctype(); }
			virtual SQLULEN size() const { return IntervalParam::size(); }
			virtual void * rwDataAddress() { return &data; }
			virtual operator boost::posix_time::time_duration () const;
			virtual const Param * meAsAParam() const { return this; }
			virtual void apply(DB::HandleField &) const;
	};
	class TimeStampColumn : public Column, public TimeStampParam {
		public:
			TimeStampColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
				DB::Column(n, i),
				Column(sc, n, i) { }
			virtual SQLSMALLINT ctype() const { return TimeStampParam::ctype(); }
			virtual SQLULEN size() const { return TimeStampParam::size(); }
			virtual void * rwDataAddress() { return &data; }
			virtual operator boost::posix_time::ptime () const;
			virtual const Param * meAsAParam() const { return this; }
			virtual void apply(DB::HandleField &) const;
	};
}

#endif

