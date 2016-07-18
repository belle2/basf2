#include <framework/utilities/GeneralCut.h>
#include <gtest/gtest.h>

using namespace Belle2;
namespace {
  /// Class to mock objects for out variable manager.
  struct MockObjectType {
    /// Stupid singlevalued object.
    double value = 4.2;
  };

  /**
   *  Class to mock variables for out variable manager.
   *  This is also the minimal interface a variable must have,
   *  to be used in the GeneralCut.
   */
  class MockVariableType {
  public:
    /// Function of the variable which always returns the value of the object.
    double function(const MockObjectType* object) const
    {
      return object->value;
    }

    /// Name of the variable.
    const std::string name = "mocking_variable";
  };

  /**
   *  Mock a variable manager for tests. This is also the minimal interface a
   *  class must have to be used as a VariableManager in the GeneralCut tenplate.
   */
  class MockVariableManager {
  public:
    /// Use MockObjectType as Objects.
    typedef MockObjectType Object;
    /// Use MockvariableType as Variables.
    typedef MockVariableType Var;

    /// Singleton.
    static MockVariableManager& Instance()
    {
      static MockVariableManager instance;
      return instance;
    }

    /// Return the single mocking variable we have in cases the correct name is given.
    Var* getVariable(const std::string& name)
    {
      if (name == "mocking_variable") {
        return &m_mocking_variable;
      } else {
        return nullptr;
      }
    }

    /// The only variable we have in this test.
    Var m_mocking_variable;
  };

  /// Type of a mocked general cut.
  typedef GeneralCut<MockVariableManager> MockGeneralCut;

  /// Test for the general cut: Try to compile some cuts and check their result using a mocked variable manager.
  TEST(GeneralCutTest, cutCheck)
  {
    MockObjectType testObject;

    std::unique_ptr<MockGeneralCut> a = MockGeneralCut::Compile("1.2 < 1.5 ");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile(" 1.5<1.2");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::Compile(" 12.3 >1.5 ");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("12 > 15");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::Compile("1.2 == 1.2");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile(" 1.5!=1.2");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("1.2 == 1.2 == 1.2");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile(" 1.5 == 1.5!=1.2");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile(" 1.5 == 1.5!=1.5");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::Compile("1.0 < 1.2 == 1.2");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile(" 1.5 < 1.6 != 1.6");
    EXPECT_FALSE(a->check(&testObject));
    a = MockGeneralCut::Compile(" 1.5 < 1.6 != 1.7");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile(" [12 >= 12 ]");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("[ 15>= 16 ]");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::Compile(" [12 <= 12 ]");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("  [ 17<= 16.7 ]");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::Compile(" [12 <= 12 < 13]");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("  [ 17<= 16.7 < 18 ]");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::Compile(" [12 >= 12 < 13]");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("  [ 15> 16.7 <= 18 ]");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::Compile("mocking_variable > 1.0");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("1.0 < mocking_variable <= mocking_variable");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("mocking_variable < 100.0");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("mocking_variable <= mocking_variable <= mocking_variable");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("1 < 2 and 3 < 4");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("1 < 2 and 4 < 3");
    EXPECT_FALSE(a->check(&testObject));
    a = MockGeneralCut::Compile("2 < 1 and 4 < 3");
    EXPECT_FALSE(a->check(&testObject));
    a = MockGeneralCut::Compile("2 < 1 and 3 < 4");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::Compile("1 < 2 or 3 < 4");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("1 < 2 or 4 < 3");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("2 < 1 or 4 < 3");
    EXPECT_FALSE(a->check(&testObject));
    a = MockGeneralCut::Compile("2 < 1 or 3 < 4");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("1 < 2 and 3 < 4 and [ 5 < 6 or 7 > 6 ]");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("[1 < 2 < 3 or 3 > 4 ] and [ 5 < 6 or 7 > 6 ]");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("[1 < 2 < 3 or 3 > 4 ] or [ 5 < 6 and 7 > 6 ]");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("1 < 2 and 3 < 4 or 5 > 6");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("1 < 2 or 3 < 4 and 5 > 6");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("1 < 2 and 4 < 3 or 6 > 5");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("1 < 2 or 4 < 3 and 6 > 5");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("1 != 2 and 3 < 4 or 5 > 6");
    EXPECT_TRUE(a->check(&testObject));
    a = MockGeneralCut::Compile("1 < 2 or 3 != 4 and 5 > 6");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("1 != 2 and 3 == 4 or 5 > 6");
    EXPECT_FALSE(a->check(&testObject));
    a = MockGeneralCut::Compile("1 < 2 or 3 == 4 and 5 > 6");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("15 != 0x15");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("15 == 0xF");
    EXPECT_TRUE(a->check(&testObject));

    // Should throw an exception
    EXPECT_THROW(a = MockGeneralCut::Compile("15 == 15.0 bla"), std::runtime_error);
    EXPECT_TRUE(a->check(&testObject));
    EXPECT_THROW(a = MockGeneralCut::Compile("15 == other_var"), std::runtime_error);
    EXPECT_TRUE(a->check(&testObject));
    EXPECT_THROW(a = MockGeneralCut::Compile("15 == 15e1000"), std::out_of_range);
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("1e-3 < 1e3");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("1e-3 == 0.001");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("1000 < infinity");
    EXPECT_TRUE(a->check(&testObject));

    a = MockGeneralCut::Compile("1000 > infinity");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::Compile("1000 < nan");
    EXPECT_FALSE(a->check(&testObject));

    a = MockGeneralCut::Compile("1000 > nan");
    EXPECT_FALSE(a->check(&testObject));
  }

  /// Test for the general cut: Try to compile some cuts and check if decompiling gives back more or less the same string (except for [ and ]).
  TEST(GeneralCutTest, CompileAndDecompile)
  {

    std::unique_ptr<MockGeneralCut> a = MockGeneralCut::Compile("1 < 2");
    EXPECT_EQ(a->decompile(), "[1 < 2]");

    a = MockGeneralCut::Compile("[1 < 2]");
    EXPECT_EQ(a->decompile(), "[1 < 2]");

    a = MockGeneralCut::Compile("1 < 2 < 3");
    EXPECT_EQ(a->decompile(), "[[1 < 2] and [2 < 3]]");

    a = MockGeneralCut::Compile("[1 < 2 < 3] or [[ 2 < 4] and [  mocking_variable < 4.4231 and [1 < 3 and 4 < mocking_variable]]]");
    EXPECT_EQ(a->decompile(),
              "[[[1 < 2] and [2 < 3]] or [[2 < 4] and [[mocking_variable < 4.4231] and [[1 < 3] and [4 < mocking_variable]]]]]");
  }


}  // namespace
