#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/vxdCaTracking/FilterID.h>
#include <iostream>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class FilterIDTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(FilterIDTest, simpleTest)
  {
    // provides a usefull filterType
    Belle2::Tracking::FilterID aFilterIDTranslator;
    Belle2::Tracking::FilterID::filterTypes aFilterType = aFilterIDTranslator.getFilterType(Belle2::Tracking::FilterID::nameHelixFit);
    EXPECT_EQ(Belle2::Tracking::FilterID::nameHelixFit, aFilterIDTranslator.getFilterString(aFilterType));

    EXPECT_EQ(aFilterType, aFilterIDTranslator.getFilterType(Belle2::Tracking::FilterID::nameHelixFit));

  }
}
