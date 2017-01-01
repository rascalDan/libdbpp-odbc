#ifndef ODBC_CONNECTION_H
#define ODBC_CONNECTION_H

#include <connection.h>
#include <error.h>
#include "odbc-dsn.h"
#include "odbc-error.h"
#include <sql.h>

namespace ODBC {
	class ConnectionError : public virtual Error, public virtual DB::ConnectionError {
		public:
			ConnectionError(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle);
	};

	class Connection : public DB::Connection {
		public:
			Connection(const DSN& d);
			Connection(const std::string & str);
			~Connection();
			SQLHENV 		env;
			SQLHDBC 		conn;

			void beginTxInt() override;
			void commitTxInt() override;
			void rollbackTxInt() override;
			void ping() const override;
			std::string		getAttrStr(SQLINTEGER) const;
			SQLINTEGER		getAttrInt(SQLINTEGER) const;
			DB::BulkDeleteStyle bulkDeleteStyle() const override;
			DB::BulkUpdateStyle bulkUpdateStyle() const override;

			DB::SelectCommand * newSelectCommand(const std::string & sql, const DB::CommandOptions *) override;
			DB::ModifyCommand * newModifyCommand(const std::string & sql, const DB::CommandOptions *) override;

		private:
			DB::BulkDeleteStyle thinkDelStyle;
			DB::BulkUpdateStyle thinkUpdStyle;

			void			connectPre();
			void			connectPost();
	};
}

#endif

