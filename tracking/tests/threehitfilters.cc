#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/vxdCaTracking/ThreeHitFilters.h>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

using namespace std;
using namespace Belle2::Tracking;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class ThreeHitFiltersTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(ThreeHitFiltersTest, simpleTest)
  {
    // testing deltaPt-calculator:
    TVector3 innerHit(1., 1., 0.), centerHit(3., 3., 0.), outerHitEvil(6., 3., 0.), outerHitSimple(6., 4., 0.);
//    B2INFO("outerHitEvil: \n outerHitSimple: \n centerHit: \n innerHit: \n"/* << innerHit.Print()*/)
//    outerHitSimple.Print();
//    outerHitEvil.Print();
//    centerHit.Print();
//    innerHit.Print();

//    B2INFO("now tests with 1.5T (and reset between each step): \n")

    ThreeHitFilters aFilter = ThreeHitFilters(outerHitSimple, centerHit, innerHit);
//    B2INFO("now comparing values of calcPt using simple outerHit: \n calcPt: " << aFilter.calcPt() )
//    aFilter.resetValues(outerHitSimple,centerHit,innerHit);
    EXPECT_DOUBLE_EQ(0.0169210118807579, aFilter.calcPt());

    aFilter.resetValues(outerHitEvil, centerHit, innerHit);
//    B2INFO("now comparing values of calcPt using evil outerHit: \n calcPt: " << aFilter.calcPt() )
//    aFilter.resetValues(outerHitEvil,centerHit,innerHit);
    EXPECT_DOUBLE_EQ(0.0098836160618849412, aFilter.calcPt());

    aFilter.resetValues(outerHitSimple, centerHit, innerHit);
//    B2INFO("now comparing values of calcCircleDist2IP using simple outerHit: \n calcPt: " << aFilter.calcCircleDist2IP())
//    aFilter.resetValues(outerHitSimple,centerHit,innerHit);
    EXPECT_DOUBLE_EQ(3.2003308750466548, aFilter.calcCircleDist2IP());

    aFilter.resetValues(outerHitEvil, centerHit, innerHit);
//    B2INFO("now comparing values of calcCircleDist2IP using evil outerHit: \n calcPt: " << aFilter.calcPt())
//    aFilter.resetValues(outerHitEvil,centerHit,innerHit);
    EXPECT_DOUBLE_EQ(2.329208242689889, aFilter.calcCircleDist2IP());



//    B2INFO("now tests with 0.976T (and reset between each step): \n")
    aFilter.resetMagneticField(0.976);
    aFilter.resetValues(outerHitSimple, centerHit, innerHit);
//    B2INFO("now comparing values of calcPt using simple outerHit: \n calcPt: " << aFilter.calcPt() )
//    aFilter.resetValues(outerHitSimple,centerHit,innerHit);
    EXPECT_DOUBLE_EQ(0.011009938397079805, aFilter.calcPt());

    aFilter.resetValues(outerHitEvil, centerHit, innerHit);
//    B2INFO("now comparing values of calcPt using evil outerHit: \n calcPt: " << aFilter.calcPt())
//    aFilter.resetValues(outerHitEvil,centerHit,innerHit);
    EXPECT_DOUBLE_EQ(0.0064309395175998008, aFilter.calcPt());

    aFilter.resetValues(outerHitSimple, centerHit, innerHit);
//    B2INFO("now comparing values of calcCircleDist2IP using simple outerHit: \n calcPt: " << aFilter.calcCircleDist2IP() )
//    aFilter.resetValues(outerHitSimple,centerHit,innerHit);
    EXPECT_DOUBLE_EQ(3.2003308750466548, aFilter.calcCircleDist2IP());

    aFilter.resetValues(outerHitEvil, centerHit, innerHit);
//    B2INFO("now comparing values of calcCircleDist2IP using evil outerHit: \n calcPt: " << aFilter.calcPt() )
//    aFilter.resetValues(outerHitEvil,centerHit,innerHit);
    EXPECT_DOUBLE_EQ(2.329208242689889, aFilter.calcCircleDist2IP());
  }

}  // namespace
