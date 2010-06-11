#include <sqlext.h>
#include "command.h"
#include "param.h"

ODBC::BindBase::BindBase() :
	bindLen(0)
{
}

ODBC::Bind<unsigned char *>::~Bind()
{
	if (value) {
		delete[] value;
	}
}


