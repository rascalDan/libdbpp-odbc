#ifndef ODBC_COMMAND_H
#define ODBC_COMMAND_H

#include "odbc-connection.h"
#include "odbc-param_fwd.h"
#include <command.h>
#include <glibmm/ustring.h>
#include <vector>

namespace ODBC {
	using ParamPtr = std::unique_ptr<Param>;
	class Command : public virtual DB::Command {
		using Params = std::vector<ParamPtr>;

	public:
		Command(const Connection &, const std::string & sql);

		void bindParamI(unsigned int i, int val) override;
		void bindParamI(unsigned int i, long val) override;
		void bindParamI(unsigned int i, long long val) override;
		void bindParamI(unsigned int i, unsigned int val) override;
		void bindParamI(unsigned int i, unsigned long int val) override;
		void bindParamI(unsigned int i, unsigned long long int val) override;

		void bindParamB(unsigned int i, bool val) override;

		void bindParamF(unsigned int i, double val) override;
		void bindParamF(unsigned int i, float val) override;

		void bindParamS(unsigned int i, const Glib::ustring &) override;
		void bindParamS(unsigned int i, const std::string_view &) override;

		void bindParamT(unsigned int i, const boost::posix_time::time_duration &) override;
		void bindParamT(unsigned int i, const boost::posix_time::ptime &) override;

		void bindNull(unsigned int i) override;

	protected:
		friend class Param;
		friend class Column;
		SQLHSTMT hStmt;
		const Connection & connection;

	private:
		Params params;

		template<class ParamType> ParamType * makeParam(unsigned int idx);
	};

}

#endif
