#define BOOST_TEST_MODULE TestODBC
#include <boost/test/unit_test.hpp>

#include <definedDirs.h>
#include <dbpp/modifycommand.h>
#include <dbpp/selectcommand.h>
#include <dbpp/column.h>
#include <dbpp/error.h>
#include <mock.h>
#include <testCore.h>
#include <definedDirs.h>
#include <boost/date_time/posix_time/posix_time.hpp>

class StandardMockDatabase : public ODBC::Mock {
	public:
		StandardMockDatabase() : ODBC::Mock("Driver=postgresql;Database=postgres;uid=postgres;servername=/run/postgresql", "odbcmock", {
				rootDir / "odbcschema.sql" })
		{
		}
};

BOOST_GLOBAL_FIXTURE( StandardMockDatabase );

BOOST_FIXTURE_TEST_SUITE( Core, DB::TestCore );

BOOST_AUTO_TEST_CASE( transactions )
{
	auto ro = DB::MockDatabase::openConnectionTo("odbcmock");

	BOOST_REQUIRE_EQUAL(false, ro->inTx());
	ro->beginTx();
	BOOST_REQUIRE_EQUAL(true, ro->inTx());
	ro->rollbackTx();
	BOOST_REQUIRE_EQUAL(false, ro->inTx());

	ro->beginTx();
	BOOST_REQUIRE_EQUAL(true, ro->inTx());
	ro->commitTx();
	BOOST_REQUIRE_EQUAL(false, ro->inTx());

	delete ro;
}

BOOST_AUTO_TEST_CASE( bindAndSend )
{
	auto rw = DB::MockDatabase::openConnectionTo("odbcmock");

	auto mod = rw->newModifyCommand("INSERT INTO test VALUES(?, ?, ?, ?, ?)");
	mod->bindParamI(0, testInt);
	mod->bindParamF(1, testDouble);
	mod->bindParamS(2, testString);
	mod->bindParamB(3, testBool);
	mod->bindParamT(4, testDateTime);
	mod->execute();
	delete mod;
	delete rw;
}

BOOST_AUTO_TEST_CASE( bindAndSelect )
{
	auto ro = DB::MockDatabase::openConnectionTo("odbcmock");

	auto select = ro->newSelectCommand("SELECT * FROM test WHERE id = ?");
	select->bindParamI(0, testInt);
	select->execute();
	int rows = 0;
	while (select->fetch()) {
		assertColumnValueHelper(*select, 0, testInt);
		assertColumnValueHelper(*select, 1, testDouble);
		assertColumnValueHelper(*select, 2, testString);
		// assertColumnValueHelper(*select, 3, testBool);
		assertColumnValueHelper(*select, 4, testDateTime);
		rows += 1;
	}
	delete select;
	BOOST_REQUIRE_EQUAL(1, rows);
	delete ro;
}

BOOST_AUTO_TEST_CASE( bindAndSelectOther )
{
	auto ro = DB::MockDatabase::openConnectionTo("odbcmock");

	auto select = ro->newSelectCommand("SELECT * FROM test WHERE id != ?");
	select->bindParamI(0, testInt);
	select->execute();
	int rows = 0;
	while (select->fetch()) {
		assertColumnValueHelper(*select, 0, 4);
		assertColumnValueHelper(*select, 1, 123.45);
		assertColumnValueHelper(*select, 2, std::string("some text"));
		// assertColumnValueHelper(*select, 3, true);
		assertColumnValueHelper(*select, 4, boost::posix_time::ptime_from_tm({ 3, 6, 23, 27, 3, 115, 0, 0, 0, 0, 0}));
		rows += 1;
	}
	delete select;
	BOOST_REQUIRE_EQUAL(1, rows);
	delete ro;
}

BOOST_AUTO_TEST_SUITE_END();

