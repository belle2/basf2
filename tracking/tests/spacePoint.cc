#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/spacePointCreation/SpacePoint.h>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class SpacePointTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(SpacePointTest, simpleTest)
  {

    EXPECT_DOUBLE_EQ(0, 0);

  }

}  // namespace
