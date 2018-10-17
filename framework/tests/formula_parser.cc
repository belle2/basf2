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
    EXPECT_FORMULA_OK("1", "1.000000");
    EXPECT_FORMULA_OK("2.", "2.000000");
    EXPECT_FORMULA_OK(".3", "0.300000");
    EXPECT_FORMULA_OK("+4", "4.000000");
    EXPECT_FORMULA_OK("-5", "-5.000000");
    EXPECT_FORMULA_OK("+6e0", "6.000000");
    EXPECT_FORMULA_OK("-7e0", "-7.000000");
    EXPECT_FORMULA_OK("8.e-1", "0.800000");
  }

  TEST_F(FormulaParserTest, OperatorPrecedence)
  {
    EXPECT_FORMULA_OK("a+b+c", "(('a'+'b')+'c')");
    EXPECT_FORMULA_OK("a+b*c", "('a'+('b'*'c'))");
    EXPECT_FORMULA_OK("a^b*c", "(('a'^'b')*'c')");
  }
}

