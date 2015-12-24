#include "odbc-connection.h"
#include "odbc-mock.h"
#include <buffer.h>

namespace ODBC {

Mock::Mock(const std::string & masterdb, const std::string & name, const std::vector<boost::filesystem::path> & ss) :
	MockServerDatabase(masterdb, name, "odbc")
{
	CreateNewDatabase();
	PlaySchemaScripts(ss);
}

DB::Connection *
Mock::openConnection() const
{
	return new Connection(stringbf("Driver=postgresql;Database=%s;uid=postgres;servername=/run/postgresql", testDbName));
}

Mock::~Mock()
{
	DropDatabase();
}

void Mock::DropDatabase() const
{
	master->execute("SELECT pg_terminate_backend(pid) FROM pg_stat_activity WHERE datname = '" + testDbName + "'");
	MockServerDatabase::DropDatabase();
}

}

