#ifndef CONNECTION_H
#define CONNECTION_H

#include "dsn.h"
#include <sql.h>

namespace ODBC {
	class Connection {
		public:
			Connection(const DSN& d);
			Connection(const String& str);
			~Connection();
			SQLHENV 		env;
			SQLHDBC 		conn;

			int				beginTx();
			int				commitTx();
			int				rollbackTx();
			void			abortTx() const;
			bool			txIsAborted() const;
		private:
			mutable unsigned int	txDepth;
			mutable bool			txAborted;
	};
}

#endif

