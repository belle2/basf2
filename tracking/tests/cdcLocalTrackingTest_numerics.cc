/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>
#include "cdcLocalTrackingTest.h"

#include <tracking/cdcLocalTracking/numerics/numerics.h>


using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


TEST_F(CDCLocalTrackingTest, numerics_sign)
{
  EXPECT_EQ(PLUS, sign(0.0));
  EXPECT_EQ(MINUS, sign(-0.0));
  EXPECT_EQ(INVALID_SIGN, sign(NAN));

  EXPECT_TRUE(isValidSign(PLUS));
  EXPECT_TRUE(isValidSign(MINUS));
  EXPECT_TRUE(isValidSign(ZERO));

  EXPECT_FALSE(isValidSign(INVALID_SIGN));
  EXPECT_FALSE(isValidSign(7));

  EXPECT_EQ(MINUS, oppositeSign(PLUS));
  EXPECT_EQ(PLUS, oppositeSign(MINUS));
  EXPECT_EQ(ZERO, oppositeSign(ZERO));
  EXPECT_EQ(INVALID_SIGN, oppositeSign(INVALID_SIGN));
}


