/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/utilities/sqlite.h>
#include <framework/geometry/B2Vector3.h>

#include <gtest/gtest.h>
#include <set>

using namespace Belle2;

namespace {
  /** Fixture to test sqlite interface */
  class SQLiteTest: public ::testing::Test {
  protected:
    /** in memory sqlite database */
    sqlite3* m_connection{nullptr};
    /** Create a small test database in memory */
    void SetUp() override
    {
      int result = sqlite3_open_v2(":memory:", &m_connection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
      ASSERT_EQ(result, SQLITE_OK);
      std::string sql = R"DOC(
          CREATE TABLE test(name TEXT, x FLOAT, y FLOAT, z FLOAT);
          INSERT INTO test VALUES ('IP', 0, 0, 0);
          INSERT INTO test VALUES ('X', 1, 0, 0);
          INSERT INTO test VALUES ('Y', 0, 1, 0);
          INSERT INTO test VALUES ('Z', 0, 0, 1);
      )DOC";
      result = sqlite3_exec(m_connection, sql.c_str(), nullptr, nullptr, nullptr);
      ASSERT_EQ(result, SQLITE_OK);
    }

    /** And close it at the end */
    void TearDown() override
    {
      int result = sqlite3_close_v2(m_connection);
      ASSERT_EQ(result, SQLITE_OK);
    }
  };

  /** Test a simple sqlite statement returning std::tuple */
  TEST_F(SQLiteTest, Statement)
  {
    sqlite::Statement<std::string, double> stmt(m_connection, "SELECT name, x from test ORDER by name DESC", false);
    std::vector<std::tuple<std::string, double>> expected{{"IP", 0.}, {"X", 1.}, {"Y", 0.}, {"Z", 0.}};
    for (auto && row : stmt) {
      ASSERT_FALSE(expected.empty());
      ASSERT_EQ(row, expected.back());
      expected.pop_back();
    }
    ASSERT_TRUE(expected.empty());
  }

  /** Test a simple sqlite statement converting the types from float to string */
  TEST_F(SQLiteTest, StatementTypeConversion)
  {
    sqlite::Statement<std::string, std::string> stmt(m_connection, "SELECT name, x from test ORDER by name DESC", false);
    std::vector<std::tuple<std::string, std::string>> expected{{"IP", "0.0"}, {"X", "1.0"}, {"Y", "0.0"}, {"Z", "0.0"}};
    for (auto && row : stmt) {
      ASSERT_FALSE(expected.empty());
      ASSERT_EQ(row, expected.back());
      expected.pop_back();
    }
    ASSERT_TRUE(expected.empty());
  }

  /** Test a simple sqlite statement converting the types from float to int */
  TEST_F(SQLiteTest, StatementTypeConversion2)
  {
    sqlite::Statement<std::string, int> stmt(m_connection, "SELECT name, x from test ORDER by name DESC", false);
    std::vector<std::tuple<std::string, int>> expected{{"IP", 0}, {"X", 1}, {"Y", 0}, {"Z", 0}};
    for (auto && row : stmt) {
      ASSERT_FALSE(expected.empty());
      ASSERT_EQ(row, expected.back());
      expected.pop_back();
    }
    ASSERT_TRUE(expected.empty());
  }

  /** test a one column statement with simple type and one parameter */
  TEST_F(SQLiteTest, SimpleStatement)
  {
    sqlite::SimpleStatement<std::string> stmt(m_connection, "SELECT name from test WHERE Y=?", false);
    std::set<std::string> expected{"IP", "X", "Z"};
    stmt.execute(0.);
    std::set<std::string> actual(stmt.begin(), stmt.end());
    ASSERT_EQ(expected, actual);
    expected = {"Y"};
    stmt.execute(1.);
    actual = std::set<std::string>(stmt.begin(), stmt.end());
    ASSERT_EQ(expected, actual);
  }

  /** test a one column statement with simple type and conversion of the parameter */
  TEST_F(SQLiteTest, SimpleStatementParameterConversion)
  {
    sqlite::SimpleStatement<std::string> stmt(m_connection, "SELECT name from test WHERE Y=?", false);
    std::set<std::string> expected{"IP", "X", "Z"};
    stmt.execute("0");
    std::set<std::string> actual(stmt.begin(), stmt.end());
    ASSERT_EQ(expected, actual);
    expected = {"Y"};
    stmt.execute(1);
    actual = std::set<std::string>(stmt.begin(), stmt.end());
    ASSERT_EQ(expected, actual);
  }

  /** test returning objects */
  TEST_F(SQLiteTest, ObjectStatement)
  {
    sqlite::ObjectStatement<B2Vector3D, double, double, double> stmt(m_connection, "SELECT x,y,z from test ORDER by name", false);
    auto it = stmt.begin();
    ASSERT_NE(it, stmt.end());
    ASSERT_EQ(*it, B2Vector3D(0, 0, 0));
    ASSERT_NE(++it, stmt.end());
    ASSERT_EQ(*it, B2Vector3D(1, 0, 0));
    ASSERT_NE(++it, stmt.end());
    ASSERT_EQ(*it, B2Vector3D(0, 1, 0));
    ASSERT_NE(++it, stmt.end());
    ASSERT_EQ(*it, B2Vector3D(0, 0, 1));
    ASSERT_EQ(++it, stmt.end());
  }

  /** test checking the number of parameters */
  TEST_F(SQLiteTest, NotEnoughParams)
  {
    sqlite::SimpleStatement<std::string> stmt(m_connection, "SELECT name from test WHERE Y=?", false);
    ASSERT_THROW(stmt.execute(), std::runtime_error);
  }

  /** test checking the number of parameters */
  TEST_F(SQLiteTest, TooManyParams)
  {
    sqlite::SimpleStatement<std::string> stmt(m_connection, "SELECT name from test WHERE Y=?", false);
    ASSERT_THROW(stmt.execute(0, "foo"), std::runtime_error);
  }

  /** test checking the number of columns */
  TEST_F(SQLiteTest, NotEnoughColumns)
  {
    auto construct =  [this]() { return sqlite::Statement<std::string, int>(m_connection, "SELECT name from test ORDER by name DESC", false);};
    ASSERT_THROW(construct(), std::runtime_error);
  }

  /** test checking the number of columns */
  TEST_F(SQLiteTest, TooManyColumns)
  {
    auto construct =  [this]() { return sqlite::Statement<std::string>(m_connection, "SELECT name, x from test ORDER by name DESC", false);};
    ASSERT_THROW(construct(), std::runtime_error);
  }
}
