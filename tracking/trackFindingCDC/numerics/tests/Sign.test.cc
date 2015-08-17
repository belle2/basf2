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


TEST(TrackFindingCDCTest, numerics_Sign_dominant)
{
  EXPECT_EQ(Sign::c_Plus, Sign::dominant(Sign::c_Plus, Sign::c_Plus));
  EXPECT_EQ(Sign::c_Zero, Sign::dominant(Sign::c_Plus, Sign::c_Zero));
  EXPECT_EQ(Sign::c_Zero, Sign::dominant(Sign::c_Plus, Sign::c_Minus));
  EXPECT_EQ(Sign::c_Plus, Sign::dominant(Sign::c_Plus, Sign::c_Invalid));

  EXPECT_EQ(Sign::c_Minus, Sign::dominant(Sign::c_Minus, Sign::c_Minus));
  EXPECT_EQ(Sign::c_Zero, Sign::dominant(Sign::c_Minus, Sign::c_Zero));
  EXPECT_EQ(Sign::c_Zero, Sign::dominant(Sign::c_Minus, Sign::c_Plus));
  EXPECT_EQ(Sign::c_Minus, Sign::dominant(Sign::c_Minus, Sign::c_Invalid));

  EXPECT_EQ(Sign::c_Zero, Sign::dominant(Sign::c_Zero, Sign::c_Zero));
  EXPECT_EQ(Sign::c_Zero, Sign::dominant(Sign::c_Zero, Sign::c_Minus));
  EXPECT_EQ(Sign::c_Zero, Sign::dominant(Sign::c_Zero, Sign::c_Plus));
  EXPECT_EQ(Sign::c_Zero, Sign::dominant(Sign::c_Zero, Sign::c_Invalid));

  EXPECT_EQ(Sign::c_Plus, Sign::dominant(Sign::c_Invalid, Sign::c_Plus));
  EXPECT_EQ(Sign::c_Minus, Sign::dominant(Sign::c_Invalid, Sign::c_Minus));
  EXPECT_EQ(Sign::c_Zero, Sign::dominant(Sign::c_Invalid, Sign::c_Zero));
  EXPECT_FALSE(Sign::dominant(Sign::c_Invalid, Sign::c_Invalid).isValid());
}
