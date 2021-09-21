#ifndef ODBC_ERROR_H
#define ODBC_ERROR_H

#include <error.h> // IWYU pragma: keep
#include <exception.h>
#include <sql.h>
#include <string>

namespace ODBC {
	class Error : public AdHoc::Exception<DB::Error> {
	public:
		Error(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle);

		std::string message() const noexcept override;

	private:
		std::string msg;
	};
}

#endif
