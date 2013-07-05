#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/vxdCaTracking/TwoHitFilters.h>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

using namespace std;
using namespace Belle2::Tracking;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class TwoHitFiltersTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(TwoHitFiltersTest, simpleTest)
  {
    TwoHitFilters aFilter = TwoHitFilters();

    EXPECT_DOUBLE_EQ(0., aFilter.calcDist3D());

    EXPECT_DOUBLE_EQ(0., aFilter.calcDistXY());

    EXPECT_DOUBLE_EQ(0., aFilter.calcDistZ());

    EXPECT_DOUBLE_EQ(0., aFilter.calcSlopeRZ());

    EXPECT_DOUBLE_EQ(0., aFilter.calcNormedDist3D());

  }

}  // namespace
