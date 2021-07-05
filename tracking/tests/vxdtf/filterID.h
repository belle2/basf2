#include <tracking/dataobjects/FilterID.h>
#include <gtest/gtest.h>

namespace Belle2 {



  /** Set up a few arrays and objects in the datastore */
  class FilterIDTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(FilterIDTest, simpleTest)
  {
    // provides a usefull filterType
    Belle2::FilterID aFilterIDTranslator;
    Belle2::FilterID::filterTypes aFilterType = aFilterIDTranslator.getFilterType(Belle2::FilterID::nameHelixParameterFit);
    EXPECT_EQ(Belle2::FilterID::nameHelixParameterFit, aFilterIDTranslator.getFilterString(aFilterType));

    EXPECT_EQ(aFilterType, aFilterIDTranslator.getFilterType(Belle2::FilterID::nameHelixParameterFit));

  }
}
