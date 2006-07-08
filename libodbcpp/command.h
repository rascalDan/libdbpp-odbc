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
			void				bindParamI(unsigned int i, unsigned int val);
			void				bindParamF(unsigned int i, double val);
			void				bindParamF(unsigned int i, float val);
			void				bindParamS(unsigned int i, const char *);
			void				bindParamS(unsigned int i, const unsigned char *);
			void				bindParamS(unsigned int i, const unsigned char *, size_t);
			void				bindParamS(unsigned int i, std::string);
			void				bindParamS(unsigned int i, String);
			void				bindParamT(unsigned int i, struct tm *);
			void				bindParamT(unsigned int i, time_t);

			const String		sql;
		protected:
			SQLHSTMT        	hStmt;
		private:
			Params				params;
	};

}

#endif

