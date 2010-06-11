#include "dsn.h"

ODBC::DSN::DSN(const std::string & d, const std::string & u, const std::string & p) :
	dsn(d),
	username(u),
	password(p)
{
}

ODBC::DSN::~DSN()
{
}

