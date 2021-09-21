#include "odbc-dsn.h"
#include <utility>

ODBC::DSN::DSN(std::string d, std::string u, std::string p) :
	dsn(std::move(d)), username(std::move(u)), password(std::move(p))
{
}
