#include <framework/dbobjects/Clocks.h>
#include <gtest/gtest.h>
#include <framework/gearbox/Const.h>

using namespace Belle2;

namespace {

  /** Testing the clocks setter and getter */
  TEST(Clocks,  getClock)
  {

    Float_t topPrescaleTest  = 6;
    Float_t eclPrescaleTest  = 3 * 24;

    Clocks testClock;

    testClock.setClockPrescale(Const::EDetector::TOP, "sampling", topPrescaleTest);
    testClock.setClockPrescale(Const::EDetector::ECL, "sampling", eclPrescaleTest);

    EXPECT_EQ(testClock.getClockPrescale(Const::EDetector::TOP, "sampling"), topPrescaleTest);
    EXPECT_EQ(testClock.getClockPrescale(Const::EDetector::ECL, "sampling"), eclPrescaleTest);
  }
} // namespace
