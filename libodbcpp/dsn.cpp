#include "dsn.h"

ODBC::DSN::DSN(String d, String u, String p) :
	dsn(d),
	username(u),
	password(p)
{
}

ODBC::DSN::~DSN()
{
}

