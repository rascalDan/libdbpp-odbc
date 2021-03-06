#ifndef ODBC_CONNECTION_H
#define ODBC_CONNECTION_H

#include "command_fwd.h"
#include "odbc-error.h"
#include <connection.h>
#include <sql.h>
#include <string>

namespace ODBC {
	class DSN;
	class ConnectionError : public virtual Error, public virtual DB::ConnectionError {
	public:
		ConnectionError(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle);
	};

	class Connection : public DB::Connection {
	public:
		explicit Connection(const DSN & d);
		explicit Connection(const std::string & str);
		~Connection() override;
		SQLHENV env;
		SQLHDBC conn;

		void beginTxInt() override;
		void commitTxInt() override;
		void rollbackTxInt() override;
		void ping() const override;
		std::string getAttrStr(SQLINTEGER) const;
		SQLINTEGER getAttrInt(SQLINTEGER) const;
		DB::BulkDeleteStyle bulkDeleteStyle() const override;
		DB::BulkUpdateStyle bulkUpdateStyle() const override;

		DB::SelectCommandPtr select(const std::string & sql, const DB::CommandOptionsCPtr &) override;
		DB::ModifyCommandPtr modify(const std::string & sql, const DB::CommandOptionsCPtr &) override;

	private:
		DB::BulkDeleteStyle thinkDelStyle;
		DB::BulkUpdateStyle thinkUpdStyle;

		void connectPre();
		void connectPost();
	};
}

#endif
