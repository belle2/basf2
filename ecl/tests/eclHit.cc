/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/dataobjects/ECLHit.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class ECLHitTest : public ::testing::Test {
  };

  /** Test Constructors. */
  TEST_F(ECLHitTest, Constructors)
  {
    ECLHit myHit;
    EXPECT_EQ(myHit.getCellId(), 0);
    EXPECT_EQ(myHit.getEnergyDep(), 0);
    EXPECT_EQ(myHit.getTimeAve(), 0);

    const int cellId = 1;
    const float edep = 1.234;
    const float timeAve = 4.321;
    ECLHit myHit2(cellId, edep, timeAve);
    EXPECT_EQ(myHit2.getCellId(), cellId);
    EXPECT_EQ(myHit2.getEnergyDep(), edep);
    EXPECT_EQ(myHit2.getTimeAve(), timeAve);
  } // Testcases for Something

  /** Test Setters and Getters. */
  TEST_F(ECLHitTest, SettersAndGetters)
  {
    const int cellId = 1;
    const float edep = 1.234;
    const float timeAve = 4.321;
    const float deltaTimeShift = 2.5;
    ECLHit myHit;
    myHit.setCellId(cellId);
    myHit.setEnergyDep(edep);
    myHit.setTimeAve(timeAve);
    EXPECT_EQ(myHit.getCellId(), cellId);
    EXPECT_EQ(myHit.getEnergyDep(), edep);
    EXPECT_EQ(myHit.getTimeAve(), timeAve);
    myHit.shiftInTime(deltaTimeShift);
    EXPECT_EQ(myHit.getTimeAve(), timeAve + deltaTimeShift);

  } // Testcases for Setters and Getters

}  // namespace
