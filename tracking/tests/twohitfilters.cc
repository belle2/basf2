#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/vxdCaTracking/TwoHitFilters.h>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class TwoHitFiltersTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(TwoHitFiltersTest, testEmptyFilter)
  {
    TwoHitFilters aFilter = TwoHitFilters();

    EXPECT_DOUBLE_EQ(0., aFilter.calcDist3D());

    EXPECT_DOUBLE_EQ(0., aFilter.calcDistXY());

    EXPECT_DOUBLE_EQ(0., aFilter.calcDistZ());

    EXPECT_DOUBLE_EQ(0., aFilter.calcSlopeRZ());

    EXPECT_DOUBLE_EQ(0., aFilter.calcNormedDist3D());

  }

  TEST_F(TwoHitFiltersTest, testFilledFilter)
  {
    TVector3 innerHit(1, 2, 3);
    TVector3 outerHit(2, 3, 4);

    TwoHitFilters aFilter = TwoHitFilters(outerHit, innerHit); // correct order

    EXPECT_DOUBLE_EQ(3., aFilter.calcDist3D()); // does calc dist (outer - innerHit)^2!

    EXPECT_DOUBLE_EQ(2., aFilter.calcDistXY()); // does calc dist (outer - innerHit)^2!

    EXPECT_DOUBLE_EQ(1., aFilter.calcDistZ());

    EXPECT_DOUBLE_EQ(2., aFilter.calcSlopeRZ());

    EXPECT_DOUBLE_EQ(2. / 3., aFilter.calcNormedDist3D());

    TwoHitFilters bFilter = TwoHitFilters(); // initialising an empty filter first to check whether the resetting function is doing its job...

    bFilter.resetValues(innerHit, outerHit); // wrong order

    EXPECT_DOUBLE_EQ(aFilter.calcDist3D(), bFilter.calcDist3D()); // does calc dist (outer - innerHit)^2!

    EXPECT_DOUBLE_EQ(aFilter.calcDistXY(), bFilter.calcDistXY()); // does calc dist (outer - innerHit)^2!

    EXPECT_DOUBLE_EQ(-aFilter.calcDistZ(), bFilter.calcDistZ());

    EXPECT_DOUBLE_EQ(-aFilter.calcSlopeRZ(), bFilter.calcSlopeRZ());

    EXPECT_DOUBLE_EQ(aFilter.calcNormedDist3D(), bFilter.calcNormedDist3D());
  }
}  // namespace
