#ifndef ODBC_COMMAND_H
#define ODBC_COMMAND_H

#include <vector>
#include "connection.h"
#include "timetypepair.h"

namespace ODBC {
	class Param;
	class Command {
			typedef std::vector<Param*> Params;
		public:
			Command(const Connection&, String sql);
			virtual ~Command() = 0;

			void				bindParamI(unsigned int i, int val);
			void				bindParamI(unsigned int i, long val);
			void				bindParamI(unsigned int i, unsigned int val);
			void				bindParamI(unsigned int i, long unsigned int val);
			void				bindParamI(unsigned int i, long long unsigned int val);
			void				bindParamF(unsigned int i, double val);
			void				bindParamF(unsigned int i, float val);
			void				bindParamS(unsigned int i, const char *);
			void				bindParamS(unsigned int i, const unsigned char *);
			void				bindParamS(unsigned int i, const unsigned char *, size_t);
			void				bindParamS(unsigned int i, std::string);
			void				bindParamS(unsigned int i, String);
			void				bindParamT(unsigned int i, const struct tm *);
			void				bindParamT(unsigned int i, time_t);
			void				bindParamT(unsigned int i, const TimeTypePair & p);

			const String		sql;
		protected:
			friend class BindBase;
			SQLHSTMT        	hStmt;
			const Connection&	connection;
		private:
			Params				params;
	};

}

#endif

