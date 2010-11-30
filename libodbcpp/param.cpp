#include <sqlext.h>
#include "param.h"
#include "command.h"
#include "error.h"
#include <string.h>

ODBC::Param::Param() :
	paramCmd(NULL),
	paramIdx(0),
	paramBound(false)
{
}

ODBC::Param::Param(Command * c, unsigned int i) :
	paramCmd(c),
	paramIdx(i),
	paramBound(false)
{
}

ODBC::Param::~Param(){
}

template <class ParamType>
ParamType *
ODBC::Command::makeParam(unsigned int idx)
{
	if (idx >= params.size()) {
		throw Error("%s: Bind out of bounds", __FUNCTION__);
	}
	Param * & p = params[idx];
	if (p) {
		ParamType * np = dynamic_cast<ParamType *>(p);
		if (np) {
			return np;
		}
		delete p;
	}
	ParamType * np = new ParamType(this, idx);
	p = np;
	return np;
}

void
ODBC::Param::bind() const
{
	if (!paramBound) {
		RETCODE rc = SQLBindParameter(paramCmd->hStmt, paramIdx + 1, SQL_PARAM_INPUT, ctype(), stype(),
				size(), dp(), const_cast<void *>(dataAddress()), size(), &bindLen);
		if (!SQL_SUCCEEDED(rc)) {
			throw Error(rc, SQL_HANDLE_STMT, paramCmd->hStmt, "%s: Bind for parameter %u",
					__FUNCTION__, paramIdx);
		}
		paramBound = true;
	}
}

#define SIMPLEBINDER(ctype, otype, suf) \
void \
ODBC::Command::bindParam##suf(unsigned int i, ctype val) \
{ \
	ODBC::otype * p = makeParam<ODBC::otype>(i); \
	*p = val; \
	p->bind(); \
}
SIMPLEBINDER(int, SignedIntegerParam, I);
SIMPLEBINDER(long, SignedIntegerParam, I);
SIMPLEBINDER(long long, SignedIntegerParam, I);
SIMPLEBINDER(unsigned int, UnsignedIntegerParam, I);
SIMPLEBINDER(unsigned long int, UnsignedIntegerParam, I);
SIMPLEBINDER(unsigned long long int, UnsignedIntegerParam, I);

SIMPLEBINDER(double, FloatingPointParam, F);
SIMPLEBINDER(float, FloatingPointParam, F);

SIMPLEBINDER(const Glib::ustring &, GlibUstringParam, S);

SIMPLEBINDER(const struct tm *, TimeStampParam, T);
SIMPLEBINDER(const SQL_TIMESTAMP_STRUCT &, TimeStampParam, T);
SIMPLEBINDER(time_t, TimeStampParam, T);

void
ODBC::Command::bindNull(unsigned int i)
{
	makeParam<NullParam>(i)->bind();
}

void
ODBC::GlibUstringParam::operator=(Glib::ustring const & d)
{
	const char * addr = data.data();
	data = d;
	bindLen = d.bytes();
	paramBound &= (addr == data.data());
	if (!paramBound) {
		paramBound = false;
		bind();
	}
}

void
ODBC::TimeStampParam::operator=(long const & d)
{
	struct tm t;
	gmtime_r(&d, &t);
	data << t;
}

