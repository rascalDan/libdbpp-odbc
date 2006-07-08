#ifndef ODBC_PARAM_H
#define ODBC_PARAM_H

#include <malloc.h>
#include "bind.h"

namespace ODBC {
	template <class> class _Param;
	class Param : public BindBase {
		public:
			Param();
			virtual			~Param();
			bool			bound;				// Has SqlBind... been called?
			void			bind(SQLHANDLE, SQLUINTEGER, SQLSMALLINT, SQLSMALLINT, SQLINTEGER,
					SQLINTEGER, const void*, size_t);
			template <class T>
			void			makeBindLen(T*&, size_t newSize);
			template <class t>
			static ODBC::_Param<t>*
							makeParam(ODBC::Param*& p);
	};
	template <class t>
	class _Param : public Bind<t>, public Param {
		public:
			~_Param() {}
	};
}

#endif

