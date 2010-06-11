#ifndef CONNECTION_H
#define CONNECTION_H

#include "dsn.h"
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
		private:
			mutable unsigned int	txDepth;
			mutable bool			txAborted;
	};
}

#endif

