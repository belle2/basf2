/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/dbobjects/HardwareClockSettings.h>
#include <framework/gearbox/Const.h>
#include <gtest/gtest.h>
#include <framework/utilities/TestHelpers.h>

using namespace Belle2;

namespace {

  /** Testing the clocks setter and getter */
  TEST(HardwareClockSettings,  getClock)
  {

    Int_t topPrescaleTest  = 6;
    Int_t eclPrescaleTest  = 3 * 24;
    Float_t RF = 508.887;
    Float_t dummyFrequency = 999.99;

    HardwareClockSettings testClock;

    testClock.setAcceleratorRF(RF);

    testClock.setClockPrescale(Const::EDetector::TOP, "sampling", topPrescaleTest);
    testClock.setClockPrescale(Const::EDetector::ECL, "sampling", eclPrescaleTest);
    testClock.setClockFrequency(Const::EDetector::ECL, "dummy", dummyFrequency);
    testClock.setClockFrequency(Const::EDetector::ARICH, "dummy", dummyFrequency);

    EXPECT_EQ(testClock.getAcceleratorRF(), RF);

    EXPECT_EQ(testClock.getClockPrescale(Const::EDetector::TOP, "sampling"), topPrescaleTest);
    EXPECT_EQ(testClock.getClockPrescale(Const::EDetector::ECL, "sampling"), eclPrescaleTest);

    EXPECT_EQ(testClock.getClockFrequency(Const::EDetector::ECL, "sampling"), RF / 4. / eclPrescaleTest);
    EXPECT_EQ(testClock.getClockFrequency(Const::EDetector::ECL, "dummy"), dummyFrequency);
    EXPECT_EQ(testClock.getClockFrequency(Const::EDetector::ARICH, "dummy"), dummyFrequency);

    EXPECT_EQ(testClock.isPrescaleAvailable(Const::EDetector::TOP, "sampling"), true);
    EXPECT_EQ(testClock.isPrescaleAvailable(Const::EDetector::TOP, "wrongname"), false);
    EXPECT_EQ(testClock.isPrescaleAvailable(Const::EDetector::SVD, "wrongname"), false);

    EXPECT_EQ(testClock.isFrequencyAvailable(Const::EDetector::TOP, "sampling"), true);
    EXPECT_EQ(testClock.isFrequencyAvailable(Const::EDetector::ECL, "dummy"), true);
    EXPECT_EQ(testClock.isFrequencyAvailable(Const::EDetector::ARICH, "wrongname"), false);
    EXPECT_EQ(testClock.isFrequencyAvailable(Const::EDetector::SVD, "wrongname"), false);


    //Try to get prescale from detector not present in the prescale map
    EXPECT_B2ERROR(testClock.getClockPrescale(Const::EDetector::SVD, "sampling"));

    //Try to get prescale from label not present in the prescale map
    EXPECT_B2ERROR(testClock.getClockPrescale(Const::EDetector::ECL, "wrongname"));

    //Try to get clock from detector not present in both prescale and frequency map
    EXPECT_B2ERROR(testClock.getClockFrequency(Const::EDetector::KLM, "wrongname"));

    //Try to get clock from label not present in the maps and detector present in both prescale and frequency map
    EXPECT_B2ERROR(testClock.getClockFrequency(Const::EDetector::ECL, "wrongname"));

    //Try to get clock from label not present in the maps and detector present only in frequency map
    EXPECT_B2ERROR(testClock.getClockFrequency(Const::EDetector::ARICH, "wrongname"));

    //Try to get clock from label not present in the maps and detector present only in prescale map
    EXPECT_B2ERROR(testClock.getClockFrequency(Const::EDetector::TOP, "wrongname"));
  }
} // namespace
