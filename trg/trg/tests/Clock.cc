/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <gtest/gtest.h>

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

  TEST(TRGClockTest, ClockConversions)
  {
    const double tdcBinWidth = 1.0;
    const TRGClock _clock("CDCTrigger system clock", 0, 125. / tdcBinWidth);
    const TRGClock _clockFE("CDCFE TDC clock", _clock, 8);
    const TRGClock _clockTDC("CDCTrigger TDC clock (after mergers)",
                             _clock,
                             4);
    const TRGClock _clockD("CDCTrigger data clock", _clock, 1, 4);

    const double tfe0 = 0.;
    const double tfe255 = 255;

    EXPECT_EQ(_clockFE.position(tfe0), _clockD.position(tfe0));
    EXPECT_EQ(_clockFE.position(tfe255) / 8, _clock.position(tfe255));
    EXPECT_EQ(_clockD.position(tfe255), _clockFE.position(tfe255) / 32);

    EXPECT_EQ(_clockFE.positionInSourceClock(tfe0),
              _clockD.positionInSourceClock(tfe0));
  }
}
