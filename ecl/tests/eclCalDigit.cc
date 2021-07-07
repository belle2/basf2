/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/dataobjects/ECLCalDigit.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class ECLCalDigitTest : public ::testing::Test {
  };

  /** Test Constructors */
  TEST_F(ECLCalDigitTest, Constructors)
  {
    ECLCalDigit myECLCalDigit;
    EXPECT_EQ(myECLCalDigit.getCellId(), 0);
    EXPECT_EQ(myECLCalDigit.getEnergy(), 0);
    EXPECT_EQ(myECLCalDigit.getTime(), 0);
    EXPECT_EQ(myECLCalDigit.getTimeResolution(), 0);
  } // Testcases for Constructors

  /** Test Setters and Getter. */
  TEST_F(ECLCalDigitTest, SettersAndGetters)
  {
    const int cellId = 1;
    const double energy = 2.1;
    const double time = 2.2;
    const double timeres = 2.3;

    ECLCalDigit myECLCalDigit;
    myECLCalDigit.setCellId(cellId);
    myECLCalDigit.setEnergy(energy);
    myECLCalDigit.setTime(time);
    myECLCalDigit.setTimeResolution(timeres);

    EXPECT_EQ(myECLCalDigit.getCellId(), cellId);
    EXPECT_EQ(myECLCalDigit.getEnergy(), energy);
    EXPECT_EQ(myECLCalDigit.getTime(), time);
    EXPECT_EQ(myECLCalDigit.getTimeResolution(), timeres);
  } // Testcases for Setters and Getters

}  // namespace
