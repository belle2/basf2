#include <framework/dataobjects/EventT0.h>
#include <cmath>
#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  /** Testing the event T0 uncertainty calculation */
  TEST(EventT0, UncertaintyCalculation)
  {
    EventT0 t0;

    auto extractedT0 = t0.getEventT0WithUncertainty();
    ASSERT_EQ(extractedT0.first, 0);
    ASSERT_EQ(extractedT0.second, 0);

    ASSERT_EQ(t0.getDetectors().size(), 0);

    // Add a first event t0
    t0.addEventT0(2, 1, Const::CDC);

    extractedT0 = t0.getEventT0WithUncertainty();
    ASSERT_EQ(extractedT0.first, 2);
    ASSERT_EQ(extractedT0.second, 1);

    extractedT0 = t0.getEventT0WithUncertainty(Const::PXD);
    ASSERT_EQ(extractedT0.first, 0);
    ASSERT_EQ(extractedT0.second, 0);

    ASSERT_EQ(t0.getDetectors().size(), 1);

    // Add a second event t0
    t0.addEventT0(1, 0.5, Const::PXD);

    extractedT0 = t0.getEventT0WithUncertainty();
    ASSERT_EQ(extractedT0.first, 1.2);
    ASSERT_EQ(extractedT0.second, 1 / std::sqrt(5));

    ASSERT_EQ(t0.getDetectors().size(), 2);

    extractedT0 = t0.getEventT0WithUncertainty(Const::CDC);
    ASSERT_EQ(extractedT0.first, 2);
    ASSERT_EQ(extractedT0.second, 1);

    extractedT0 = t0.getEventT0WithUncertainty(Const::PXD);
    ASSERT_EQ(extractedT0.first, 1);
    ASSERT_EQ(extractedT0.second, 0.5);

    ASSERT_EQ(t0.getDetectors().size(), 2);

    ASSERT_TRUE(t0.hasEventT0(Const::PXD));
    ASSERT_TRUE(t0.hasEventT0(Const::CDC));
  }
}
