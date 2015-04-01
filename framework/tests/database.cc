#include <framework/database/IntervalOfValidity.h>
#include <framework/dataobjects/EventMetaData.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {

  /** Test range check of validity intervals */
  TEST(DataBase, IntervalOfValidity)
  {
    EventMetaData event(0, 8, 15);  // experiment 15, run 8

    EXPECT_TRUE(IntervalOfValidity().empty());
    EXPECT_FALSE(IntervalOfValidity(1, 0, 0, 0).empty());
    EXPECT_FALSE(IntervalOfValidity(0, 0, 1, 0).empty());
    EXPECT_FALSE(IntervalOfValidity(1, 2, 3, 4).empty());

    EXPECT_FALSE(IntervalOfValidity().contains(event));
    EXPECT_FALSE(IntervalOfValidity(16, 1, 0, 0).contains(event));
    EXPECT_FALSE(IntervalOfValidity(16, 0, 0, 0).contains(event));
    EXPECT_FALSE(IntervalOfValidity(15, 9, 0, 0).contains(event));
    EXPECT_TRUE(IntervalOfValidity(15, 8, 0, 0).contains(event));
    EXPECT_TRUE(IntervalOfValidity(15, 8, 15, 0).contains(event));
    EXPECT_TRUE(IntervalOfValidity(15, 8, 15, 8).contains(event));
    EXPECT_TRUE(IntervalOfValidity(15, 0, 15, 8).contains(event));
    EXPECT_TRUE(IntervalOfValidity(0, 0, 15, 8).contains(event));
    EXPECT_FALSE(IntervalOfValidity(0, 0, 15, 7).contains(event));
    EXPECT_FALSE(IntervalOfValidity(0, 0, 14, 0).contains(event));
    EXPECT_FALSE(IntervalOfValidity(0, 0, 14, 1).contains(event));
  }

}  // namespace
