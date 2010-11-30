#ifndef ODBC_COMMAND_H
#define ODBC_COMMAND_H

#include <vector>
#include "connection.h"
#include <glibmm/ustring.h>

namespace ODBC {
	class Param;
	class Command {
			typedef std::vector<Param*> Params;
		public:
			Command(const Connection &, const std::string & sql);
			virtual ~Command() = 0;

			void				bindParamI(unsigned int i, int val);
			void				bindParamI(unsigned int i, long val);
			void				bindParamI(unsigned int i, long long val);
			void				bindParamI(unsigned int i, unsigned int val);
			void				bindParamI(unsigned int i, unsigned long int val);
			void				bindParamI(unsigned int i, unsigned long long int val);

			void				bindParamF(unsigned int i, double val);
			void				bindParamF(unsigned int i, float val);

			void				bindParamS(unsigned int i, const Glib::ustring &);

			void				bindParamT(unsigned int i, const struct tm *);
			void				bindParamT(unsigned int i, const SQL_TIMESTAMP_STRUCT &);
			void				bindParamT(unsigned int i, time_t);

			void				bindNull(unsigned int i);

			const std::string	sql;
		protected:
			friend class Param;
			friend class Column;
			SQLHSTMT        	hStmt;
			const Connection&	connection;
		private:
			Params				params;

			template <class ParamType>
				ParamType *
				makeParam(unsigned int idx);
	};

}

#endif

