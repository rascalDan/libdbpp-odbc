#ifndef ODBC_COLUMN_H
#define ODBC_COLUMN_H

#include <typeinfo>
#include <glibmm/ustring.h>
#include "bind.h"
#include "param.h"

namespace ODBC {
	class SelectCommand;
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

			virtual const Glib::ustring & compose() const = 0;
			virtual Glib::ustring compose(const Glib::ustring & fmt) const = 0;
			virtual int writeToBuf(char ** buf) const = 0;
			virtual int writeToBuf(char ** buf, const char * fmt) const = 0;
			bool isNull() const;

			const unsigned int		colNo;
			const Glib::ustring		name;
			const SelectCommand *	selectCmd;
		protected:
			virtual const Param * meAsAParam() const = 0;
			mutable Glib::ustring * composeCache;
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
			virtual SQLINTEGER size() const { return data.size(); }
			virtual SQLINTEGER dp() const { return 0; }
			virtual const void * dataAddress() const { return &data.front(); }
			virtual void * rwDataAddress() { return &data.front(); }
			void operator=(const Glib::ustring & d);
			void resize(SQLHANDLE);
			virtual const Glib::ustring & compose() const;
			virtual Glib::ustring compose(const Glib::ustring & fmt) const;
			virtual int writeToBuf(char ** buf) const;
			virtual int writeToBuf(char ** buf, const char * fmt) const;
			virtual operator std::string () const { return std::string(&data.front(), bindLen); }
			virtual operator Glib::ustring () const { return std::string(&data.front(), bindLen); }
		protected:
			virtual const Param * meAsAParam() const { return this; }
			CharArray data;
	};
	class SignedIntegerColumn : public Column, public SignedIntegerParam {
		public:
			SignedIntegerColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
				Column(sc, n, i) { }
			virtual SQLSMALLINT ctype() const { return SignedIntegerParam::ctype(); }
			virtual SQLLEN size() const { return SignedIntegerParam::size(); }
			virtual void * rwDataAddress() { return &data; }
			virtual const Glib::ustring & compose() const;
			virtual Glib::ustring compose(const Glib::ustring & fmt) const;
			virtual int writeToBuf(char ** buf) const;
			virtual int writeToBuf(char ** buf, const char * fmt) const;
			virtual operator int () const { return data; }
			virtual operator long () const { return data; }
			virtual operator long long () const { return data; }
			virtual const Param * meAsAParam() const { return this; }
	};
#ifdef COMPLETENESS
	class UnsignedIntegerColumn : public Column, public UnsignedIntegerParam {
		public:
			UnsignedIntegerColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
				Column(sc, n, i) { }
			virtual const Glib::ustring & compose() const;
			virtual Glib::ustring compose(const Glib::ustring & fmt) const;
			virtual int writeToBuf(char ** buf) const;
			virtual int writeToBuf(char ** buf, const char * fmt) const;
			virtual const Param * meAsAParam() const { return this; }
	};
#endif
	class FloatingPointColumn : public Column, public FloatingPointParam {
		public:
			FloatingPointColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
				Column(sc, n, i) { }
			virtual SQLSMALLINT ctype() const { return FloatingPointParam::ctype(); }
			virtual SQLLEN size() const { return FloatingPointParam::size(); }
			virtual void * rwDataAddress() { return &data; }
			virtual const Glib::ustring & compose() const;
			virtual Glib::ustring compose(const Glib::ustring & fmt) const;
			virtual int writeToBuf(char ** buf) const;
			virtual int writeToBuf(char ** buf, const char * fmt) const;
			virtual operator double () const { return data; }
			virtual operator float () const { return data; }
			virtual const Param * meAsAParam() const { return this; }
	};
	class TimeStampColumn : public Column, public TimeStampParam {
		public:
			TimeStampColumn(SelectCommand * sc, const Glib::ustring & n, unsigned int i) :
				Column(sc, n, i) { }
			virtual SQLSMALLINT ctype() const { return TimeStampParam::ctype(); }
			virtual SQLLEN size() const { return TimeStampParam::size(); }
			virtual void * rwDataAddress() { return &data; }
			virtual const Glib::ustring & compose() const;
			virtual Glib::ustring compose(const Glib::ustring & fmt) const;
			virtual int writeToBuf(char ** buf) const;
			virtual int writeToBuf(char ** buf, const char * fmt) const;
			virtual operator struct tm () const;
			virtual operator SQL_TIMESTAMP_STRUCT () const { return data; }
			virtual const Param * meAsAParam() const { return this; }
	};
}

#endif

