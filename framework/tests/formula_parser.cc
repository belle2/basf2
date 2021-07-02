/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/utilities/FormulaParser.h>
#include <gtest/gtest.h>

using namespace Belle2;

namespace {
  class FormulaParserTest: public ::testing::Test {
  protected:
    FormulaParser<StringFormulaConstructor> m_parser;
    std::string parse(const std::string& s) { return m_parser.parse(s); }

  };

#define EXPECT_FORMULA_OK(a, b) EXPECT_NO_THROW({EXPECT_EQ(parse(a), b);})
#define EXPECT_FORMULA_FLOAT_EQ(a, b) EXPECT_NO_THROW({EXPECT_FLOAT_EQ(std::stof(parse(a)), b);})
#define EXPECT_FORMULA_FAIL(a) EXPECT_THROW(parse(a), std::runtime_error)

  TEST_F(FormulaParserTest, Simple)
  {
    EXPECT_FORMULA_OK("a", "'a'");
    EXPECT_FORMULA_OK("(a)", "'a'");
    EXPECT_FORMULA_OK("[a]", "'a'");
    EXPECT_FORMULA_OK("([a])", "'a'");
    EXPECT_FORMULA_OK("[(a)]", "'a'");
    EXPECT_FORMULA_OK("a + b", "('a'+'b')");
    EXPECT_FORMULA_OK("a - b", "('a'-'b')");
    EXPECT_FORMULA_OK("a * b", "('a'*'b')");
    EXPECT_FORMULA_OK("a / b", "('a'/'b')");
    EXPECT_FORMULA_OK("a ^ b", "('a'^'b')");
    EXPECT_FORMULA_FAIL("(a");
    EXPECT_FORMULA_FAIL("a)");
    EXPECT_FORMULA_FAIL("[a");
    EXPECT_FORMULA_FAIL("a]");
    EXPECT_FORMULA_FAIL("(a]");
    EXPECT_FORMULA_FAIL("[a)");
    EXPECT_FORMULA_FAIL("([a)]");
    EXPECT_FORMULA_FAIL("[(a])");
    EXPECT_FORMULA_FAIL("+");
    EXPECT_FORMULA_FAIL("+a");
    EXPECT_FORMULA_FAIL("a+");
    EXPECT_FORMULA_FAIL("a +- b");
    EXPECT_FORMULA_FAIL("a +* b");
    EXPECT_FORMULA_FAIL("a +/ b");
    EXPECT_FORMULA_FAIL("a +^ b");
  }
  TEST_F(FormulaParserTest, Numbers)
  {
    EXPECT_FORMULA_OK("0", "0.000000");
    EXPECT_FORMULA_OK("1", "1.000000");
    EXPECT_FORMULA_OK("2.", "2.000000");
    EXPECT_FORMULA_OK(".3", "0.300000");
    EXPECT_FORMULA_OK("+4", "4.000000");
    EXPECT_FORMULA_OK("-5", "-5.000000");
    EXPECT_FORMULA_OK("+6e0", "6.000000");
    EXPECT_FORMULA_OK("-7e0", "-7.000000");
    EXPECT_FORMULA_OK("8.e-1", "0.800000");
    EXPECT_FORMULA_FAIL("/9.0");
    EXPECT_FORMULA_FAIL("*10.0");
    EXPECT_FORMULA_OK("11**2", "121.000000");
    EXPECT_FORMULA_OK("12^2", "144.000000");
  }

  TEST_F(FormulaParserTest, OperatorPrecedence)
  {
    EXPECT_FORMULA_OK("a+b+c", "(('a'+'b')+'c')");
    EXPECT_FORMULA_OK("a+b*c", "('a'+('b'*'c'))");
    EXPECT_FORMULA_OK("a^b*c", "(('a'^'b')*'c')");

    EXPECT_FORMULA_OK("10.58-2+4", "12.580000"); // c/o. Racha
    EXPECT_FORMULA_OK("10.58-(2+4)", "4.580000");
    EXPECT_FORMULA_OK("10.58-[2+4]", "4.580000");
  }

  TEST_F(FormulaParserTest, MiscMaths)
  {
    EXPECT_FORMULA_OK("2^(1/2)", "1.414214");
    EXPECT_FORMULA_OK("2**(1/2)", "1.414214");
  }

  TEST_F(FormulaParserTest, OrdersOfMagnitude)
  {
    EXPECT_FORMULA_OK("1e10", "10000000000.000000");
    EXPECT_FORMULA_OK("1E10", "10000000000.000000");
    EXPECT_FORMULA_OK("1*10**10", "10000000000.000000");
    EXPECT_FORMULA_OK("1*10^10", "10000000000.000000");
    EXPECT_FORMULA_FLOAT_EQ("((-3+5) * 10 ^ 2 / 3e-12)", 66666666666666.66);
  }
}
