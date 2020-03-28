#ifndef MOCKODBCDATASOURCE_H
#define MOCKODBCDATASOURCE_H

#include <mockDatabase.h>
#include <filesystem>
#include <visibility.h>
#include <c++11Helpers.h>

namespace ODBC {

class DLL_PUBLIC Mock : public DB::MockServerDatabase {
	public:
		Mock(const std::string & base, const std::string & master, const std::string & name, const std::vector<std::filesystem::path> & ss);
		~Mock() override;

		SPECIAL_MEMBERS_MOVE_RO(Mock);

		const std::string base;

	protected:
		void DropDatabase() const override;

	private:
		[[nodiscard]] DB::ConnectionPtr openConnection() const override;
};

}

#endif

