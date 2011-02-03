#ifndef ODBC_COLUMN_H
#define ODBC_COLUMN_H

#include <typeinfo>
#include <glibmm/ustring.h>
#include "bind.h"
#include "param.h"

namespace ODBC {
	class SelectCommand;
	class HandleField {
		public:
			virtual void null() = 0;
			virtual void string(const std::vector<char> &, size_t len) = 0;
			virtual void integer(SQLINTEGER) = 0;
			virtual void floatingpoint(SQLDOUBLE) = 0;
			virtual void timestamp(const SQL_TIMESTAMP_STRUCT &) = 0;
	};
	class Column : public virtual Bind {
		public:
			Column(SelectCommand *, const Glib::ustring &, unsigned int);
			virtual ~Column() = 0;
			void bind();
			virtual void * rwDataAddress() = 0;
			void rebind(Command *, unsigned int idx) const;
			virtual void resize(SQLHANDLE);
			virtual void onScroll();

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
			virtual void apply(HandleField &) const = 0;

			const unsigned int		colNo;
			const Glib::ustring		name;
			const SelectCommand *	selectCmd;
		protected:
			virtual const Param * meAsAParam() const = 0;
	};
	class CharArrayColumn : public Column, public Param {
		public:
			typedef std::vector<char> CharArray;
			CharArrayColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
				Column(sc, n, i)
			{
				data.resize(256);
			}
			virtual SQLSMALLINT ctype() const { return SQL_C_CHAR; }
			virtual SQLSMALLINT stype() const { return SQL_CHAR; }
			virtual SQLULEN size() const { return data.size(); }
			virtual SQLINTEGER dp() const { return 0; }
			virtual const void * dataAddress() const { return &data.front(); }
			virtual void * rwDataAddress() { return &data.front(); }
			void operator=(const Glib::ustring & d);
			void resize(SQLHANDLE);
			virtual operator std::string () const { return std::string(&data.front(), bindLen); }
			virtual operator Glib::ustring () const { return std::string(&data.front(), bindLen); }
			virtual void apply(HandleField &) const;
		protected:
			virtual const Param * meAsAParam() const { return this; }
			CharArray data;
	};
	class SignedIntegerColumn : public Column, public SignedIntegerParam {
		public:
			SignedIntegerColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
				Column(sc, n, i) { }
			virtual SQLSMALLINT ctype() const { return SignedIntegerParam::ctype(); }
			virtual SQLULEN size() const { return SignedIntegerParam::size(); }
			virtual void * rwDataAddress() { return &data; }
			virtual operator int () const { return data; }
			virtual operator long () const { return data; }
			virtual operator long long () const { return data; }
			virtual const Param * meAsAParam() const { return this; }
			virtual void apply(HandleField &) const;
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
				Column(sc, n, i) { }
			virtual SQLSMALLINT ctype() const { return FloatingPointParam::ctype(); }
			virtual SQLULEN size() const { return FloatingPointParam::size(); }
			virtual void * rwDataAddress() { return &data; }
			virtual operator double () const { return data; }
			virtual operator float () const { return data; }
			virtual const Param * meAsAParam() const { return this; }
			virtual void apply(HandleField &) const;
	};
	class TimeStampColumn : public Column, public TimeStampParam {
		public:
			TimeStampColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
				Column(sc, n, i) { }
			virtual SQLSMALLINT ctype() const { return TimeStampParam::ctype(); }
			virtual SQLULEN size() const { return TimeStampParam::size(); }
			virtual void * rwDataAddress() { return &data; }
			virtual operator struct tm () const;
			virtual operator SQL_TIMESTAMP_STRUCT () const { return data; }
			virtual const Param * meAsAParam() const { return this; }
			virtual void apply(HandleField &) const;
	};
}

#endif

