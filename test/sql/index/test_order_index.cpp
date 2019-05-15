#include "catch.hpp"
#include "common/file_system.hpp"
#include "dbgen.hpp"
#include "test_helpers.hpp"

using namespace duckdb;
using namespace std;

TEST_CASE("Index creation on an expression", "[orderidx-index]") {
	unique_ptr<QueryResult> result;
	DuckDB db(nullptr);

	Connection con(db);

	// create a table
	REQUIRE_NO_FAIL(con.Query("CREATE TABLE integers(i INTEGER, j INTEGER)"));
	for (size_t i = 0; i < 3000; i++) {
		REQUIRE_NO_FAIL(
		    con.Query("INSERT INTO integers VALUES (" + to_string((int)(i - 1500)) + ", " + to_string(i + 12) + ")"));
	}

	result = con.Query("SELECT j FROM integers WHERE abs(i)=1");
	REQUIRE(CHECK_COLUMN(result, 0, {1511, 1513}));

	REQUIRE_NO_FAIL(con.Query("CREATE INDEX i_index ON integers using order_index(abs(i))"));

	result = con.Query("SELECT j FROM integers WHERE abs(i)=1");
	REQUIRE(CHECK_COLUMN(result, 0, {1511, 1513}));
}

TEST_CASE("Drop Index", "[orderidx-drop]") {
	unique_ptr<QueryResult> result;
	DuckDB db(nullptr);

	Connection con(db);

	// create a table
	REQUIRE_NO_FAIL(con.Query("CREATE TABLE integers(i INTEGER)"));
	REQUIRE_NO_FAIL(con.Query("INSERT INTO integers VALUES (7)"));
	REQUIRE_NO_FAIL(con.Query("INSERT INTO integers VALUES (2)"));
	REQUIRE_NO_FAIL(con.Query("INSERT INTO integers VALUES (5)"));
	result = con.Query("SELECT * FROM integers WHERE i=2");
	REQUIRE(CHECK_COLUMN(result, 0, {2}));
	REQUIRE_NO_FAIL(con.Query("INSERT INTO integers VALUES (8)"));
	REQUIRE_NO_FAIL(con.Query("CREATE INDEX i_index ON integers using order_index(i)"));
	result = con.Query("SELECT * FROM integers WHERE i=2");
	REQUIRE(CHECK_COLUMN(result, 0, {2}));
	result = con.Query("DROP INDEX i_index;");
	result = con.Query("SELECT * FROM integers WHERE i=2");
	REQUIRE(CHECK_COLUMN(result, 0, {2}));
	REQUIRE_NO_FAIL(con.Query("CREATE INDEX i_index ON integers using order_index(i)"));
	result = con.Query("SELECT * FROM integers WHERE i=2");
	REQUIRE(CHECK_COLUMN(result, 0, {2}));
}

TEST_CASE("Open Range Queries", "[orderidx-openrange]") {
	unique_ptr<QueryResult> result;
	DuckDB db(nullptr);

	Connection con(db);
	REQUIRE_NO_FAIL(con.Query("CREATE TABLE integers(i INTEGER)"));
	for (size_t i = 0; i < 10; i++) {
		REQUIRE_NO_FAIL(con.Query("INSERT INTO integers VALUES (" + to_string(i) + ")"));
	}
	REQUIRE_NO_FAIL(con.Query("CREATE INDEX i_index ON integers using order_index(i)"));

	result = con.Query("SELECT sum(i) FROM integers WHERE i>9");
	REQUIRE(CHECK_COLUMN(result, 0, {Value()}));
	result = con.Query("SELECT sum(i) FROM integers WHERE 9<i");
	REQUIRE(CHECK_COLUMN(result, 0, {Value()}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i>=10");
	REQUIRE(CHECK_COLUMN(result, 0, {Value()}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i>7");
	REQUIRE(CHECK_COLUMN(result, 0, {17}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i>=7");
	REQUIRE(CHECK_COLUMN(result, 0, {24}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i<3");
	REQUIRE(CHECK_COLUMN(result, 0, {3}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i<=3");
	REQUIRE(CHECK_COLUMN(result, 0, {6}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i<0");
	REQUIRE(CHECK_COLUMN(result, 0, {Value()}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i=0");
	REQUIRE(CHECK_COLUMN(result, 0, {0}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i > 7 and  i>3");
	REQUIRE(CHECK_COLUMN(result, 0, {17}));
	result = con.Query("SELECT sum(i) FROM integers WHERE  i >= 7 and i > 7");
	REQUIRE(CHECK_COLUMN(result, 0, {17}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i<=3 and i < 3");
	REQUIRE(CHECK_COLUMN(result, 0, {3}));
}

TEST_CASE("Closed Range Queries", "[orderidx-closerange]") {
	unique_ptr<QueryResult> result;
	DuckDB db(nullptr);

	Connection con(db);
	REQUIRE_NO_FAIL(con.Query("CREATE TABLE integers (i INTEGER)"));
	for (size_t i = 0; i < 3000; i++) {
		REQUIRE_NO_FAIL(con.Query("INSERT INTO integers VALUES (" + to_string(i) + ")"));
	}
	REQUIRE_NO_FAIL(con.Query("CREATE INDEX i_index ON integers using order_index (i)"));
	result = con.Query("SELECT sum(i) FROM integers WHERE i> 5 and i<9 ");
	REQUIRE(CHECK_COLUMN(result, 0, {21}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i>=5 and i<9 ");
	REQUIRE(CHECK_COLUMN(result, 0, {26}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i>=5 and i<=9 ");
	REQUIRE(CHECK_COLUMN(result, 0, {35}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i>=5 and i<=4 ");
	REQUIRE(CHECK_COLUMN(result, 0, {Value()}));
	result = con.Query("SELECT sum(i) FROM integers WHERE i>=5 and i<=2000 ");
	REQUIRE(CHECK_COLUMN(result, 0, {2000990}));
}
