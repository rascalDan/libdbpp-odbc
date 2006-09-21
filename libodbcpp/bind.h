#ifndef ODBC_BIND_H
#define ODBC_BIND_H

#include <sql.h>

namespace ODBC {
	class BindBase {
		public:
			BindBase();
			virtual			~BindBase() {}
			SQLINTEGER		length() const;
			SQLUINTEGER		size() const;
			bool			isNull() const;
		private:
			SQLUINTEGER		bindSize;			// Allocated memory
			SQLINTEGER		bindLen;			// Used memory
			friend class Param;
			friend class Column;
	};
	template <class t>
	class Bind {
		public:
			virtual			~Bind() {}
			t				value;
	};
	template <>
	class Bind<unsigned char*> {
		public:
			virtual			~Bind();
			unsigned char *	value;
	};
}

#endif

