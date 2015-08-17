/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/numerics/Sign.h>
#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;


TEST(TrackFindingCDCTest, numerics_Sign_sweep)
{
  EXPECT_EQ(c_PlusSign, Sign::sweep(c_PlusSign, c_PlusSign));
  EXPECT_EQ(c_ZeroSign, Sign::sweep(c_PlusSign, c_ZeroSign));
  EXPECT_EQ(c_ZeroSign, Sign::sweep(c_PlusSign, c_MinusSign));
  EXPECT_EQ(c_PlusSign, Sign::sweep(c_PlusSign, c_InvalidSign));

  EXPECT_EQ(c_MinusSign, Sign::sweep(c_MinusSign, c_MinusSign));
  EXPECT_EQ(c_ZeroSign, Sign::sweep(c_MinusSign, c_ZeroSign));
  EXPECT_EQ(c_ZeroSign, Sign::sweep(c_MinusSign, c_PlusSign));
  EXPECT_EQ(c_MinusSign, Sign::sweep(c_MinusSign, c_InvalidSign));

  EXPECT_EQ(c_ZeroSign, Sign::sweep(c_ZeroSign, c_ZeroSign));
  EXPECT_EQ(c_ZeroSign, Sign::sweep(c_ZeroSign, c_MinusSign));
  EXPECT_EQ(c_ZeroSign, Sign::sweep(c_ZeroSign, c_PlusSign));
  EXPECT_EQ(c_ZeroSign, Sign::sweep(c_ZeroSign, c_InvalidSign));

  EXPECT_EQ(c_PlusSign, Sign::sweep(c_InvalidSign, c_PlusSign));
  EXPECT_EQ(c_MinusSign, Sign::sweep(c_InvalidSign, c_MinusSign));
  EXPECT_EQ(c_ZeroSign, Sign::sweep(c_InvalidSign, c_ZeroSign));
  EXPECT_TRUE(Sign::sweep(c_InvalidSign, c_InvalidSign).isnan());
}


TEST(TrackFindingCDCTest, numerics_Sign_reverse)
{
  EXPECT_TRUE(c_InvalidSign.reversed().isnan());
  EXPECT_EQ(c_ZeroSign, c_ZeroSign.reversed());

  EXPECT_EQ(c_MinusSign, c_PlusSign.reversed());
  EXPECT_EQ(c_PlusSign, c_MinusSign.reversed());
}
