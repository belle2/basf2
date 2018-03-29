
#include <framework/dataobjects/EventT0.h>
#include <cmath>
#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  /** Testing the event T0 uncertainty calculation */
  TEST(EventT0, UncertaintyCalculation)
  {
    EventT0 t0;

    ASSERT_FALSE(t0.hasEventT0());

    t0.setEventT0(-1, 42, Const::CDC);
    ASSERT_TRUE(t0.hasEventT0());
    ASSERT_EQ(t0.getEventT0(), -1);
    ASSERT_EQ(t0.getEventT0Uncertainty(), 42);

    t0.setEventT0(-2, 43, Const::CDC);
    ASSERT_TRUE(t0.hasEventT0());
    ASSERT_EQ(t0.getEventT0(), -2);
    ASSERT_EQ(t0.getEventT0Uncertainty(), 43);

    t0.addTemporaryEventT0(2, 3, Const::SVD);
    ASSERT_EQ(t0.getEventT0(), -2);
    ASSERT_EQ(t0.getEventT0Uncertainty(), 43);

    ASSERT_EQ(t0.getNumberOfTemporaryEventT0s(), 1);
    ASSERT_TRUE(t0.hasTemporaryEventT0());
    ASSERT_TRUE(t0.hasTemporaryEventT0(Const::SVD));
    ASSERT_FALSE(t0.hasTemporaryEventT0(Const::CDC));
    ASSERT_EQ(t0.getTemporaryDetectors(), Const::SVD);
    ASSERT_EQ(t0.getTemporaryEventT0s().front().eventT0, 2);
    ASSERT_EQ(t0.getTemporaryEventT0s().front().eventT0Uncertainty, 3);
    ASSERT_TRUE(t0.getTemporaryEventT0s().front().detectorSet.contains(Const::SVD));
  }

  /** Testing the event T0 retrieval of temporary values */
  TEST(EventT0, RetrievalOfTemporary)
  {
    EventT0 t0;

    ASSERT_FALSE(t0.hasEventT0());

    t0.addTemporaryEventT0(-1, 42, Const::CDC);
    t0.addTemporaryEventT0(-2, 43, Const::CDC);
    t0.addTemporaryEventT0(-3, 44, Const::ECL);
    t0.addTemporaryEventT0(-5, 45, Const::TOP);

    ASSERT_TRUE(t0.hasTemporaryEventT0(Const::CDC));
    ASSERT_TRUE(t0.hasTemporaryEventT0(Const::ECL));
    ASSERT_TRUE(t0.hasTemporaryEventT0(Const::TOP));
    ASSERT_FALSE(t0.hasTemporaryEventT0(Const::SVD));

    const auto cdcTemporaries = t0.getTemporaryEventT0s(Const::CDC);
    ASSERT_EQ(cdcTemporaries.size(), 2);

    ASSERT_DOUBLE_EQ(cdcTemporaries[0].eventT0, -1.0f);
    ASSERT_DOUBLE_EQ(cdcTemporaries[1].eventT0, -2.0f);
  }
}
