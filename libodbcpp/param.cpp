#include <sqlext.h>
#include "param.h"
#include "command.h"
#include "column.h"
#include "error.h"
#include <string.h>

ODBC::Param::Param() :
	bound(false)
{
}

ODBC::Param::~Param(){
}

template <class t>
ODBC::_Param<t>*
ODBC::Param::makeParam(ODBC::Param*& p)
{
	if (p) {
		_Param<t>* np = dynamic_cast<_Param<t>*>(p);
		if (np) {
			return np;
		}
		delete p;
	}
	_Param<t>* np = new _Param<t>();
	p = np;
	return np;
}

void
ODBC::Param::bind(SQLHANDLE hStmt, SQLUINTEGER col, SQLSMALLINT ctype, SQLSMALLINT stype,
		SQLINTEGER colsize, SQLINTEGER dp, const void* value, size_t buflen)
{
	RETCODE rc = SQLBindParameter(hStmt, col, SQL_PARAM_INPUT, ctype, stype,
			colsize, dp, (void*)value, buflen, &bindLen);
	if (rc != SQL_SUCCESS) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: Bind for column %lu",
				__FUNCTION__, col);
	}
}

void
ODBC::Command::bindParamI(unsigned int i, int val)
{
	if (i < params.size()) {
		_Param<SQLINTEGER>* p = Param::makeParam<SQLINTEGER>(params[i]);
		p->value = val;
		if (!p->bound) {
			p->bind(this->hStmt, i + 1, SQL_C_SLONG, SQL_C_LONG, 0, 0,
					&p->value, sizeof(SQLINTEGER));
			p->bound = true;
		}
		return;
	}
	throw Error("%s: Bind out of bounds", __FUNCTION__);
}
void
ODBC::Command::bindParamI(unsigned int i, long val)
{
	if (i < params.size()) {
		_Param<SQLINTEGER>* p = Param::makeParam<SQLINTEGER>(params[i]);
		p->value = val;
		if (!p->bound) {
			p->bind(this->hStmt, i + 1, SQL_C_SLONG, SQL_C_LONG, 0, 0,
					&p->value, sizeof(SQLINTEGER));
			p->bound = true;
		}
		return;
	}
	throw Error("%s: Bind out of bounds", __FUNCTION__);
}
void
ODBC::Command::bindParamI(unsigned int i, long long unsigned int val)
{
	if (i < params.size()) {
		_Param<SQLUINTEGER>* p = Param::makeParam<SQLUINTEGER>(params[i]);
		p->value = val;
		if (!p->bound) {
			p->bind(this->hStmt, i + 1, SQL_C_ULONG, SQL_C_ULONG, 0, 0,
					&p->value, sizeof(SQLUINTEGER));
			p->bound = true;
		}
		return;
	}
	throw Error("%s: Bind out of bounds", __FUNCTION__);
}
void
ODBC::Command::bindParamF(unsigned int i, double val)
{
	if (i < params.size()) {
		_Param<SQLDOUBLE>* p = Param::makeParam<SQLDOUBLE>(params[i]);
		p->value = val;
		if (!p->bound) {
			p->bind(this->hStmt, i + 1, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0,
					&p->value, sizeof(SQLDOUBLE));
			p->bound = true;
		}
		return;
	}
	throw Error("%s: Bind out of bounds", __FUNCTION__);
}
void
ODBC::Command::bindParamS(unsigned int i, const Glib::ustring & val)
{
	if (i < params.size()) {
		_Param<Glib::ustring>* p = Param::makeParam<Glib::ustring>(params[i]);
		p->value = val;
		p->bindLen = p->value.bytes();
		p->bind(this->hStmt, i + 1, SQL_C_CHAR, SQL_CHAR, 0, 0, p->value.data(), p->value.bytes());
		return;
	}
	throw Error("%s: Bind out of bounds", __FUNCTION__);
}
void
ODBC::Command::bindParamS(unsigned int i, const char * val, size_t len)
{
	if (i < params.size()) {
		_Param<Glib::ustring>* p = Param::makeParam<Glib::ustring>(params[i]);
		p->value.assign(val);
		p->bindLen = len;
		p->bind(this->hStmt, i + 1, SQL_C_CHAR, SQL_CHAR, 0, 0, p->value.data(), p->value.bytes());
		return;
	}
	throw Error("%s: Bind out of bounds", __FUNCTION__);
}
void
ODBC::Command::bindParamT(unsigned int i, const struct tm * val)
{
	if (i < params.size()) {
		_Param<SQL_TIMESTAMP_STRUCT>* p = Param::makeParam<SQL_TIMESTAMP_STRUCT>(params[i]);
		p->value << *val;
		if (!p->bound) {
			p->bind(this->hStmt, i + 1, SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP,
					sizeof(SQL_TIMESTAMP_STRUCT), 0, &p->value, sizeof(SQL_TIMESTAMP_STRUCT));
			p->bound = true;
		}
		return;
	}
	throw Error("%s: Bind out of bounds", __FUNCTION__);
}
void
ODBC::Command::bindParamT(unsigned int i, const SQL_TIMESTAMP_STRUCT & val)
{
	if (i < params.size()) {
		_Param<SQL_TIMESTAMP_STRUCT>* p = Param::makeParam<SQL_TIMESTAMP_STRUCT>(params[i]);
		p->value = val;
		if (!p->bound) {
			p->bind(this->hStmt, i + 1, SQL_C_TIMESTAMP, SQL_TIMESTAMP,
					sizeof(SQL_TIMESTAMP_STRUCT), 0, &p->value, sizeof(SQL_TIMESTAMP_STRUCT));
			p->bound = true;
		}
		return;
	}
	throw Error("%s: Bind out of bounds", __FUNCTION__);
}



// Wrappers for all the roughly compatable types

void
ODBC::Command::bindParamI(unsigned int i, long unsigned int val)
{
	bindParamI(i, (long long unsigned int)val);
}
void
ODBC::Command::bindParamI(unsigned int i, unsigned int val)
{
	bindParamI(i, (long long unsigned int)val);
}
void
ODBC::Command::bindParamS(unsigned int i, const char * val)
{
	bindParamS(i, val, strlen(val));
}
void
ODBC::Command::bindParamS(unsigned int i, const std::string & val)
{
	bindParamS(i, val.c_str(), val.length());
}
void
ODBC::Command::bindParamF(unsigned int i, float val)
{
	bindParamF(i, (double)val);
}
void
ODBC::Command::bindParamT(unsigned int i, time_t val)
{
	struct tm t;
	gmtime_r(&val, &t);
	bindParamT(i, &t);
}

