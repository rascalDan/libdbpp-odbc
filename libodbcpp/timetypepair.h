#ifndef ODBC_TIMETYPEPAIR_H
#define ODBC_TIMETYPEPAIR_H

#include <time.h>
#include <sql.h>

namespace ODBC {
	class TimeTypePair {
			typedef SQL_TIMESTAMP_STRUCT SQL_TS;
		public:
			TimeTypePair	();
			TimeTypePair	(const tm&);
			TimeTypePair	(const SQL_TS&);

			const SQL_TS&	set(const tm&);
			const tm&		set(const SQL_TS&);
			SQL_TS&			sql() { return _sql; }
			tm&				c() { return _c; }
			const SQL_TS&	sql() const { return _sql; }
			const tm&		c() const { return _c; }
			void			sql2c() const;
			void			c2sql() const;
		private:
			mutable SQL_TS	_sql;
			mutable tm		_c;
	};
};

#endif

