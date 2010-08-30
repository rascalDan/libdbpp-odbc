#ifndef ODBC_BIND_H
#define ODBC_BIND_H

#include <sql.h>
#include <vector>

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
			mutable t		value;
	};
	typedef std::vector<char> SQLCHARVEC;
	template <>
	class Bind<SQLCHARVEC> {
		public:
			virtual			~Bind() {}
			mutable SQLCHARVEC	value;
	};
}

#endif

