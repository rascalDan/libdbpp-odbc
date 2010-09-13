#ifndef ODBC_COLUMN_H
#define ODBC_COLUMN_H

#include <glibmm/ustring.h>
#include "bind.h"

namespace ODBC {
	class Column : public BindBase {
		public:
			Column(const Glib::ustring &, unsigned int);
			virtual			~Column();
			void			bind(SQLHANDLE, SQLUINTEGER, SQLSMALLINT, void*, size_t);
			virtual void	resize(SQLHANDLE);
			operator int () const;
			operator unsigned int () const;
			operator long long () const;
			operator unsigned long long () const;
			operator double () const;
			operator float () const;
			operator const unsigned char * () const;
			operator const char * () const;
			operator std::string () const;
			operator Glib::ustring () const;
			operator struct tm () const;
			virtual void rebind(Command *, unsigned int col) const = 0;
			virtual const Glib::ustring & compose() const = 0;
			virtual Glib::ustring compose(const Glib::ustring & fmt) const = 0;
			virtual int writeToBuf(char ** buf) const = 0;
			virtual int writeToBuf(char ** buf, const char * fmt) const = 0;
			bool isNull() const;

			const unsigned int		colNo;
			const Glib::ustring		name;
		protected:
			mutable Glib::ustring * composeCache;
			SQLLEN			bindSize;			// Allocated memory
			friend class SelectCommand;
	};
	template <class t>
	class _Column : public Bind<t>, public Column {
		public:
			_Column(const Glib::ustring &, unsigned int);
			~_Column() {}
			void			rebind(Command *, unsigned int col) const;
			const Glib::ustring &	compose() const;
			Glib::ustring	compose(const Glib::ustring & fmt) const;
			int				writeToBuf(char ** buf) const;
			int				writeToBuf(char ** buf, const char * fmt) const;
	};
	class StringColumn : public _Column<SQLCHARVEC> {
		public:
			StringColumn(const Glib::ustring & n, unsigned int i) :
				_Column<SQLCHARVEC>(n, i) { }
			void resize(SQLHANDLE);
	};
}

void operator << (SQL_TIMESTAMP_STRUCT & target, const struct tm &);
void operator << (struct tm &, const SQL_TIMESTAMP_STRUCT & target);

#endif

