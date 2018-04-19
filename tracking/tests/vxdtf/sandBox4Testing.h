#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <gtest/gtest.h>
#include <vector>

#include <math.h> // sqrt, atan, isinf, ...

namespace Belle2 {

  /** should behave differently for different verbosity-levels given - class*/
  template <int Verbosity> class VerbosityClass {
  public:
    VerbosityClass() /*: state(Verbosity)*/ {}


    /** should behave differently for different verbosity-levels given - function */
    void SomeCleverMethod()
    {
      double bla = 5;

      VerbosityClass<Verbosity>::GiveVerboseOutput(bla);
    }

    /** do nothing relevant here just needing a dummy functhion */
    static inline void GiveVerboseOutput(double& result) { /*state = */result++; }
  protected:
  };


  /** Testing autoAssignment of vectors for functions */
  class SandBox4TestingTest : public ::testing::Test {
  public:

    /** just a small dummy-function to test the expected behavior */
    bool doStuffHere(std::vector<double> testVector = { 42})
    {
      if (testVector[0] == 42) return true;
      return false;
    }

  protected:
  };

  /** test function call with auto-assigned value */
  TEST_F(SandBox4TestingTest, testingVerbosityViaTemplates)
  {
    VerbosityClass<4> class4;
    VerbosityClass<1> class1;

    class4.SomeCleverMethod();
    class1.SomeCleverMethod();
  }

  /** test function call with auto-assigned value */
  TEST_F(SandBox4TestingTest, JustSomePlayingAroundWithfunction)
  {
    EXPECT_TRUE(doStuffHere());
    EXPECT_FALSE(doStuffHere({23}));

    if (doStuffHere() == false) {
      B2WARNING("it didn't work!");
    } else {
      B2WARNING("yay, it worked!");
    }
  }


  /** shall show when to get nan and when to get inf (and that inf != nan) */
  TEST_F(SandBox4TestingTest, TestIsNanAndIsInfBehavior)
  {
    EXPECT_TRUE(std::isinf(1. / 0.));
    EXPECT_FALSE(std::isnan(1. / 0.));
    EXPECT_TRUE(std::isnan(std::sqrt(-1)));
    EXPECT_FALSE(std::isinf(std::sqrt(-1)));
    EXPECT_TRUE(std::isnan(0. / 0.));
    EXPECT_FALSE(std::isinf(0. / 0.));

//  EXPECT_FALSE(std::isnan(std::pow(0.,0.))); // this should be nan, but actually it is implementation dependent, therefore here we get for 0^0 = 1, which is mathematically not correct
    EXPECT_FALSE(std::isinf(std::pow(0., 0.)));
  }
}  // namespace
