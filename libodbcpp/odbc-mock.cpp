#include "odbc-connection.h"
#include "odbc-mock.h"
#include <compileTimeFormatter.h>

namespace ODBC {

Mock::Mock(const std::string & b, const std::string & masterdb, const std::string & name, const std::vector<boost::filesystem::path> & ss) :
	MockServerDatabase(b + ";" + masterdb, name, "odbc"),
	base(b)
{
	CreateNewDatabase();
	PlaySchemaScripts(ss);
}

AdHocFormatter(MockConnStr, "%?;Database=%?");
DB::ConnectionPtr
Mock::openConnection() const
{
	return std::make_shared<Connection>(MockConnStr::get(base, testDbName));
}

Mock::~Mock()
{
	DropDatabase();
}

void Mock::DropDatabase() const
{
	MockServerDatabase::DropDatabase();
}

}

