#include <gtest/gtest.h>
#include <cmath>

#include <trg/trg/Clock.h>

using namespace Belle2;

namespace TRGTest {
  /** Test different time conversions. */
  TEST(TRGClockTest, TimeConversions)
  {
    double cycleBase = 8.; // cycle of base clock in ns
    int multi = 4;
    double cycle = cycleBase / multi;
    TRGClock baseClock("8ns clock", 0., 1000. / cycleBase); // base clock
    TRGClock syncClock("2ns clock (sync)", baseClock, multi); // synchronized to base clock
    TRGClock testClock("2ns clock", 0., 1000. / cycle); // independent test clock, same as syncClock
    for (double time = -2 * cycle; time <= 2 * cycle; time += cycle / 2.) {
      EXPECT_EQ(1, testClock.position(time + cycle) - testClock.position(time)) << "time " << time << " cycle " << cycle;
      EXPECT_EQ(testClock.overShoot(time), time - testClock.absoluteTime(testClock.position(time))) << "time " << time;
      EXPECT_LT(testClock.overShoot(time), cycle) << "time " << time;
      EXPECT_GE(testClock.overShoot(time), 0) << "time " << time;
      EXPECT_EQ(testClock.position(time), syncClock.position(time)) << "time " << time;
      EXPECT_EQ(testClock.overShoot(time), syncClock.overShoot(time)) << "time " << time;
    }
    for (int clock = -2; clock <= 2; ++clock) {
      EXPECT_EQ(cycle, testClock.absoluteTime(clock + 1) - testClock.absoluteTime(clock)) << "clock " << clock;
      EXPECT_EQ(clock, testClock.position(testClock.absoluteTime(clock))) << "clock " << clock;
      EXPECT_EQ(testClock.absoluteTime(clock), syncClock.absoluteTime(clock)) << "clock " << clock;
    }
  }
}
