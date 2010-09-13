#ifndef CONNECTION_H
#define CONNECTION_H

#include "dsn.h"
#include "error.h"
#include <sql.h>

namespace ODBC {
	class Connection {
		public:
			Connection(const DSN& d);
			Connection(const std::string & str);
			~Connection();
			SQLHENV 		env;
			SQLHDBC 		conn;

			int				beginTx() const;
			int				commitTx() const;
			int				rollbackTx() const;
			void			abortTx() const;
			bool			txIsAborted() const;
			bool			inTx() const;
			std::string		getAttrStr(SQLINTEGER) const;
			SQLINTEGER		getAttrInt(SQLINTEGER) const;

		private:
			void			connectPre();
			void			connectPost();
			mutable unsigned int	txDepth;
			mutable bool			txAborted;
	};
	class ConnectionError : public Error {
		public:
			ConnectionError(RETCODE err, SQLSMALLINT handletype, SQLHANDLE handle, char const * stage);
			ConnectionError(const ConnectionError &);

			const time_t FailureTime;
	};
}

#endif

