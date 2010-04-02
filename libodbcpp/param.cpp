#include <sqlext.h>
#include "param.h"
#include "command.h"
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
			colsize, dp, (void*)value, buflen, (SQLINTEGER*)&bindLen);
	if (rc != SQL_SUCCESS) {
		throw Error(rc, SQL_HANDLE_STMT, hStmt, "%s: Bind for column %lu",
				__FUNCTION__, col);
	}
}

template <class T>
void
ODBC::Param::makeBindLen(T*& p, size_t newLen)
{
	if (bindSize <= newLen) {
		if (bindSize) {
			delete p;
		}
		bindSize = newLen;
		p = new T[newLen];
	}
	bindLen = newLen;
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
		}
		return;
	}
	throw Error("%s: Bind out of bounds", __FUNCTION__);
}
void
ODBC::Command::bindParamS(unsigned int i, const unsigned char * val, size_t length)
{
	if (i < params.size()) {
		_Param<SQLCHAR*>* p = Param::makeParam<SQLCHAR*>(params[i]);
		p->makeBindLen(p->value, length);
		memcpy(p->value, val, length);
		if (!p->bound) {
			p->bind(this->hStmt, i + 1, SQL_C_CHAR, SQL_CHAR, 0, 0, p->value, length);
		}
		return;
	}
	throw Error("%s: Bind out of bounds", __FUNCTION__);
}
void
ODBC::Command::bindParamT(unsigned int i, struct tm * val)
{
	if (i < params.size()) {
		_Param<TimeTypePair>* p = Param::makeParam<TimeTypePair>(params[i]);
		p->value.set(*val);
		if (!p->bound) {
			p->bind(this->hStmt, i + 1, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,
					sizeof(SQL_TIMESTAMP_STRUCT), 0, &p->value.sql(), sizeof(SQL_TIMESTAMP_STRUCT));
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
ODBC::Command::bindParamS(unsigned int i, String val)
{
	bindParamS(i, val.c_str(), val.size());
}
void
ODBC::Command::bindParamS(unsigned int i, const unsigned char * val)
{
	const unsigned char * x = val;
	while (*val++) ;
	bindParamS(i, val, val - x);
}
void
ODBC::Command::bindParamS(unsigned int i, const char * val)
{
	bindParamS(i, (unsigned char *)val, strlen(val));
}
void
ODBC::Command::bindParamS(unsigned int i, std::string val)
{
	bindParamS(i, (unsigned char *)val.c_str(), val.size());
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

