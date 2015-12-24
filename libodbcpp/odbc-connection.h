#ifndef ODBC_CONNECTION_H
#define ODBC_CONNECTION_H

#include <connection.h>
#include <error.h>
#include "odbc-dsn.h"
#include "odbc-error.h"
#include <sql.h>

namespace ODBC {
	class Connection : public DB::Connection {
		public:
			Connection(const DSN& d);
			Connection(const std::string & str);
			~Connection();
			SQLHENV 		env;
			SQLHDBC 		conn;

			void			finish() const;
			int				beginTx() const;
			int				commitTx() const;
			int				rollbackTx() const;
			void			abortTx() const;
			bool			txIsAborted() const;
			bool			inTx() const;
			void			ping() const;
			std::string		getAttrStr(SQLINTEGER) const;
			SQLINTEGER		getAttrInt(SQLINTEGER) const;
			DB::BulkDeleteStyle bulkDeleteStyle() const;
			DB::BulkUpdateStyle bulkUpdateStyle() const;

			DB::SelectCommand * newSelectCommand(const std::string & sql) const;
			DB::ModifyCommand * newModifyCommand(const std::string & sql) const;

			void	beginBulkUpload(const char *, const char *) const;
			void	endBulkUpload(const char *) const;
			size_t bulkUploadData(const char *, size_t) const;

		private:
			DB::BulkDeleteStyle thinkDelStyle;
			DB::BulkUpdateStyle thinkUpdStyle;

			void			connectPre();
			void			connectPost();
			mutable unsigned int	txDepth;
			mutable bool			txAborted;
	};
	class ConnectionError : public DB::ConnectionError, public virtual Error {
		public:
			ConnectionError(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle, char const * stage);
			ConnectionError(const ConnectionError &);
	};
}

#endif

