#ifndef ODBC_COLUMN_H
#define ODBC_COLUMN_H

#include "ustring.h"
#include "bind.h"

namespace ODBC {
	class Column : public BindBase {
		public:
			Column(String, unsigned int);
			virtual			~Column();
			void			bind(SQLHANDLE, SQLUINTEGER, SQLSMALLINT, void*, size_t);
			operator int () const;
			operator unsigned int () const;
			operator long long () const;
			operator unsigned long long () const;
			operator double () const;
			operator float () const;
			operator const unsigned char * () const;
			operator const char * () const;
			operator std::string () const;
			operator String () const;
			operator const struct tm & () const;

			const unsigned int		colNo;
			const String	name;
		private:
			mutable bool	fresh;
			friend class SelectCommand;
	};
	template <class t>
	class _Column : public Bind<t>, public Column {
		public:
			_Column(String, unsigned int);
			~_Column() {}
	};
}

#endif

