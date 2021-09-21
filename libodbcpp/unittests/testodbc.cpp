#define BOOST_TEST_MODULE TestODBC
#include <boost/test/unit_test.hpp>

#include "mockDatabase.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <connection.h>
#include <cstdint>
#include <definedDirs.h>
#include <filesystem>
#include <memory>
#include <modifycommand.h>
#include <odbc-mock.h>
#include <selectcommand.h>
#include <selectcommandUtil.impl.h>
#include <string>
#include <string_view>
#include <testCore.h>
#include <type_traits>

class StandardMockDatabase : public DB::PluginMock<ODBC::Mock> {
public:
	StandardMockDatabase() :
		DB::PluginMock<ODBC::Mock>("odbcmock", {rootDir / "odbcschema.sql"},
				"Driver=psqlodbcw.so;uid=postgres;servername=/run/postgresql", "Database=postgres")
	{
	}
};

BOOST_GLOBAL_FIXTURE(StandardMockDatabase);

BOOST_FIXTURE_TEST_SUITE(Core, DB::TestCore)

BOOST_AUTO_TEST_CASE(transactions)
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
}

BOOST_AUTO_TEST_CASE(bindAndSend)
{
	auto rw = DB::MockDatabase::openConnectionTo("odbcmock");

	auto mod = rw->modify("INSERT INTO test VALUES(?, ?, ?, ?, ?)");
	mod->bindParamI(0, testInt);
	mod->bindParamF(1, testDouble);
	mod->bindParamS(2, testString);
	mod->bindParamB(3, testBool);
	mod->bindParamT(4, testDateTime);
	// cppcheck-suppress assertWithSideEffect
	BOOST_CHECK_EQUAL(1, mod->execute());
}

BOOST_AUTO_TEST_CASE(bindAndSelect)
{
	auto ro = DB::MockDatabase::openConnectionTo("odbcmock");

	auto select = ro->select("SELECT * FROM test WHERE id = ?");
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
	BOOST_REQUIRE_EQUAL(1, rows);
}

BOOST_AUTO_TEST_CASE(bindAndSelectOther)
{
	auto ro = DB::MockDatabase::openConnectionTo("odbcmock");

	auto select = ro->select("SELECT * FROM test WHERE id != ?");
	select->bindParamI(0, testInt);
	select->execute();
	int rows = 0;
	while (select->fetch()) {
		assertColumnValueHelper(*select, 0, 4);
		assertColumnValueHelper(*select, 1, 123.45);
		assertColumnValueHelper(*select, 2, std::string_view("some text"));
		// assertColumnValueHelper(*select, 3, true);
		assertColumnValueHelper(
				*select, 4, boost::posix_time::ptime_from_tm({3, 6, 23, 27, 3, 115, 0, 0, 0, 0, nullptr}));
		rows += 1;
	}
	BOOST_REQUIRE_EQUAL(1, rows);
}

BOOST_AUTO_TEST_CASE(multibyte)
{
	auto ro = DB::MockDatabase::openConnectionTo("odbcmock");

	auto select = ro->select("SELECT LPAD('', n, '£'), n FROM GENERATE_SERIES(1, 50000, 200) n");
	select->execute();
	int rows = 0;
	for (const auto [s, n] : select->as<std::string, int64_t>()) {
		BOOST_CHECK_EQUAL(s.length(), n * 2);
		rows += 1;
	}
	BOOST_REQUIRE_EQUAL(250, rows);
}

BOOST_AUTO_TEST_SUITE_END()
