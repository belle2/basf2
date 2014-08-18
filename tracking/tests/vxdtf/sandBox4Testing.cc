#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <gtest/gtest.h>
#include <vector>

using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Testing autoAssignment of vectors for functions */
  class SandBox4TestingTest : public ::testing::Test {
  public:
    bool doStuffHere(vector<double> testVector = { 42}) {
      if (testVector[0] == 42) return true;
      return false;
    }

  protected:
  };

  /** test function call with auto-assigned value */
  TEST_F(SandBox4TestingTest, JustSomePlayingAroundWithfunction)
  {
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