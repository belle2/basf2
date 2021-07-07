/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/dataobjects/ECLHitAssignment.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class ECLHitAssignmentTest : public ::testing::Test {
  };

  /** Test Constructors. */
  TEST_F(ECLHitAssignmentTest, Constructors)
  {
    ECLHitAssignment myECLHitAssignment;
    EXPECT_EQ(myECLHitAssignment.getShowerId(), 0);
    EXPECT_EQ(myECLHitAssignment.getCellId(), 0);
    const int showerId = 1;
    const int cellId = 2;
    ECLHitAssignment myECLHitAssignment2(showerId, cellId);
    EXPECT_EQ(myECLHitAssignment2.getShowerId(), showerId);
    EXPECT_EQ(myECLHitAssignment2.getCellId(), cellId);
  } // Testcases for Something

  /** Test Setters and Getter. */
  TEST_F(ECLHitAssignmentTest, SettersAndGetters)
  {
    const int showerId = 1;
    const int cellId = 2;
    ECLHitAssignment myECLHitAssignment;
    myECLHitAssignment.setShowerId(showerId);
    myECLHitAssignment.setCellId(cellId);
    EXPECT_EQ(myECLHitAssignment.getShowerId(), showerId);
    EXPECT_EQ(myECLHitAssignment.getCellId(), cellId);
  } // Testcases for Setters and Getters
}  // namespace
