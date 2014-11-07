#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <../../externals/v00-05-02/include/boost/concept_check.hpp>
#include <gtest/gtest.h>
#include <vector>

using namespace std;

namespace Belle2 {





  template <int Verbosity> class VerbosityClass {
  public:
    VerbosityClass() : state(Verbosity) {}


    void SomeCleverMethod() {
      int bla = 5;

      VerbosityClass<Verbosity>::GiveVerboseOutput(bla);
    }

    static inline void GiveVerboseOutput(double result) { /* do nothing here */ }
  protected:
    int state;
  };



//   template <>
//   class VerbosityClass< 1 >
//   {
//   public:
//  void SomeCleverMethod() { VerbosityClass<0>::SomeCleverMethod(); }
//
//  static inline void  GiveVerboseOutput(double result) {
//    B2WARNING("result is " << result)
//  }
//   };
//


  /** Testing autoAssignment of vectors for functions */
  class SandBox4TestingTest : public ::testing::Test {
  public:

    /** just a small dummy-function to test the expected behavior */
    bool doStuffHere(vector<double> testVector = { 42}) {
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



//   template< int i >
//   class FACTOR{
//   public:
//  FACTOR() { hansi = 23; }
//
//  enum {RESULT = i * FACTOR<i-1>::RESULT};
//
//  int hansi;
//   };
//
// template <>
//   class FACTOR< 1 >{
//   public:
//  enum {RESULT = 1};
//   };


  /** test function call with auto-assigned value */
  TEST_F(SandBox4TestingTest, JustSomePlayingAroundWithfunction)
  {
    EXPECT_TRUE(doStuffHere());
    EXPECT_FALSE(doStuffHere({23}));

    if (doStuffHere() == false) {
      B2WARNING("it didn't work!")
    } else {
      B2WARNING("yay, it worked!")
    }
  }



}  // namespace


/**
 * genfit::PlanarMeasurement bla returnRecoHit(SpacePoint* sPoint)
 * if type == Const:PXD
 *  return new PXDRecoHit(sPoint->getrelationTo());
 * else if type == Const::tel
 *  return new TELRecoHit(sPoint->getrelationTo());
 *
 *
 *
 *
 * */