/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gian Luca Pinna Angioni                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/dbobjects/HardwareClockSettings.h>
#include <framework/gearbox/Const.h>
#include <gtest/gtest.h>
using namespace Belle2;

namespace {

  /** Testing the clocks setter and getter */
  TEST(HardwareClockSettings,  getClock)
  {

    Float_t topPrescaleTest  = 6;
    Float_t eclPrescaleTest  = 3 * 24;

    HardwareClockSettings testClock;

    testClock.setClockPrescale(Const::EDetector::TOP, "sampling", topPrescaleTest);
    testClock.setClockPrescale(Const::EDetector::ECL, "sampling", eclPrescaleTest);

    EXPECT_EQ(testClock.getClockPrescale(Const::EDetector::TOP, "sampling"), topPrescaleTest);
    EXPECT_EQ(testClock.getClockPrescale(Const::EDetector::ECL, "sampling"), eclPrescaleTest);
    ASSERT_THROW(testClock.getClockPrescale(Const::EDetector::SVD, "sampling"), std::out_of_range);
    ASSERT_THROW(testClock.getClockPrescale(Const::EDetector::ECL, "wrongname"), std::out_of_range);

  }
} // namespace
