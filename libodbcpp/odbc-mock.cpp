#include "odbc-connection.h"
#include "odbc-mock.h"
#include <buffer.h>

namespace ODBC {

Mock::Mock(const std::string & b, const std::string & masterdb, const std::string & name, const std::vector<boost::filesystem::path> & ss) :
	MockServerDatabase(b + ";" + masterdb, name, "odbc"),
	base(b)
{
	CreateNewDatabase();
	PlaySchemaScripts(ss);
}

DB::Connection *
Mock::openConnection() const
{
	return new Connection(stringbf("%s;Database=%s", base, testDbName));
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

