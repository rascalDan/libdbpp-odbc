#ifndef MOCKODBCDATASOURCE_H
#define MOCKODBCDATASOURCE_H

#include <mockDatabase.h>
#include <boost/filesystem/path.hpp>
#include <visibility.h>

namespace ODBC {

class DLL_PUBLIC Mock : public DB::MockServerDatabase {
	public:
		Mock(const std::string & base, const std::string & master, const std::string & name, const std::vector<boost::filesystem::path> & ss);
		~Mock();

		const std::string base;

	protected:
		void DropDatabase() const override;

	private:
		DB::ConnectionPtr openConnection() const override;
};

}

#endif

