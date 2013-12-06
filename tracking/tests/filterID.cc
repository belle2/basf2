#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/vxdCaTracking/FilterID.h>
#include <iostream>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class FilterIDTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(FilterIDTest, simpleTest)
  {
    // provides a usefull filterType
    Belle2::FilterID aFilterIDTranslator;
    Belle2::FilterID::filterTypes aFilterType = aFilterIDTranslator.getFilterType(Belle2::FilterID::nameHelixFit);
    EXPECT_EQ(Belle2::FilterID::nameHelixFit, aFilterIDTranslator.getFilterString(aFilterType));

    EXPECT_EQ(aFilterType, aFilterIDTranslator.getFilterType(Belle2::FilterID::nameHelixFit));

  }
}
