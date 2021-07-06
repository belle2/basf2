/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/dataobjects/ECLDigit.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class ECLDigitTest : public ::testing::Test {
  };

  /** Test Constructors */
  TEST_F(ECLDigitTest, Constructors)
  {
    ECLDigit myECLDigit;
    EXPECT_EQ(myECLDigit.getCellId(), 0);
    EXPECT_EQ(myECLDigit.getAmp(), 0);
    EXPECT_EQ(myECLDigit.getTimeFit(), 0);
    EXPECT_EQ(myECLDigit.getQuality(), 0);
  } // Testcases for Constructors

  /** Test Setters and Getter. */
  TEST_F(ECLDigitTest, SettersAndGetters)
  {
    const int cellId = 1;
    const int amp = 1;
    const int timeFit = 1;
    const int quality = 1;

    ECLDigit myECLDigit;
    myECLDigit.setCellId(cellId);
    myECLDigit.setAmp(amp);
    myECLDigit.setTimeFit(timeFit);
    myECLDigit.setQuality(quality);
    EXPECT_EQ(myECLDigit.getCellId(), cellId);
    EXPECT_EQ(myECLDigit.getAmp(), amp);
    EXPECT_EQ(myECLDigit.getTimeFit(), timeFit);
    EXPECT_EQ(myECLDigit.getQuality(), quality);
  } // Testcases for Setters and Getters

}  // namespace
