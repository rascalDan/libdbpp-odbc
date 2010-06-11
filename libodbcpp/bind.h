#ifndef ODBC_BIND_H
#define ODBC_BIND_H

#include <sql.h>

namespace ODBC {
	class Command;
	class BindBase {
		public:
			BindBase();
			virtual			~BindBase() {}
		protected:
			SQLINTEGER		bindLen;			// Used memory
			friend class Param;
			friend class Column;
			friend class Command;
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

