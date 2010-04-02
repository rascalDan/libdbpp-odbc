#include "column.h"
#include "command.h"
#include "error.h"
#include "timetypepair.h"

ODBC::Column::Column(String n, unsigned int i) :
	colNo(i),
	name(n),
	fresh(false)
{
}

ODBC::Column::~Column()
{
}

#define ODBC_DEFAULT_COLUMN_CAST(ctype, rtype) \
	ODBC::Column::operator rtype() const { \
		return (dynamic_cast<const _Column<ctype>& >(*this)).value; \
	}

ODBC_DEFAULT_COLUMN_CAST(SQLINTEGER, unsigned int);
ODBC_DEFAULT_COLUMN_CAST(SQLINTEGER, unsigned long long);
ODBC_DEFAULT_COLUMN_CAST(SQLINTEGER, long long);
ODBC_DEFAULT_COLUMN_CAST(SQLINTEGER, int);
ODBC_DEFAULT_COLUMN_CAST(SQLDOUBLE, double);
ODBC_DEFAULT_COLUMN_CAST(SQLDOUBLE, float);
ODBC_DEFAULT_COLUMN_CAST(SQLCHAR*, const unsigned char *);
ODBC_DEFAULT_COLUMN_CAST(SQLCHAR*, String);
ODBC::Column::operator std::string() const {
	return (const char*)((dynamic_cast<const _Column<SQLCHAR*>& >(*this)).value);
}
ODBC::Column::operator const char * () const {
	return (const char*)((dynamic_cast<const _Column<SQLCHAR*>& >(*this)).value);
}
ODBC::Column::operator const struct tm & () const {
	const _Column<TimeTypePair>& c = dynamic_cast<const _Column<TimeTypePair>& >(*this);
	if (c.fresh) {
		c.value.sql2c();
		c.fresh = false;
	}
	return c.value.c();
}

void
ODBC::Column::bind(SQLHANDLE hStmt, SQLUINTEGER col, SQLSMALLINT ctype, void * buf, size_t size)
{
	bindSize = size;
	RETCODE rc = SQLBindCol(hStmt, col, ctype, buf, bindSize, &bindLen);
	if (rc != SQL_SUCCESS) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: Bind column %lu", __FUNCTION__, col);
	}
}

#define REBIND(t, p) \
	template<> void _Column<t>::rebind(Command * cmd, unsigned int col) const \
	{ \
		cmd->p(col, value); \
	}
namespace ODBC {
	REBIND(int, bindParamI)
	REBIND(long, bindParamI)
	REBIND(unsigned int, bindParamI)
	REBIND(long unsigned int, bindParamI)
	REBIND(long long unsigned int, bindParamI)
	REBIND(double, bindParamF)
	REBIND(float, bindParamF)
	REBIND(TimeTypePair, bindParamT)
	REBIND(unsigned char *, bindParamS)
}

