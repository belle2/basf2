#include <framework/dataobjects/BinnedEventT0.h>
#include <cmath>
#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  /** Testing the binned event t0 object */
  TEST(BinnedEventT0, Basic)
  {
    BinnedEventT0 t0;

    ASSERT_FALSE(t0.hasBinnedEventT0(Const::SVD));
    ASSERT_EQ(t0.getBinnedEventT0(Const::SVD), 0);

    t0.addBinnedEventT0(-10, Const::SVD);
    ASSERT_TRUE(t0.hasBinnedEventT0(Const::SVD));
    ASSERT_FALSE(t0.hasBinnedEventT0(Const::CDC));
    ASSERT_EQ(t0.getBinnedEventT0(Const::SVD), -10);
    ASSERT_EQ(t0.getBinnedEventT0(Const::CDC), 0);

    t0.clear();
    ASSERT_FALSE(t0.hasBinnedEventT0(Const::SVD));
    ASSERT_EQ(t0.getBinnedEventT0(Const::SVD), 0);
  }
}
