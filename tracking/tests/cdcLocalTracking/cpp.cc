/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/*
This file contains test to check the behaviour of the c++ programming language.
Its purpose is mainly to asure the programmer that his assumptions about run time behaviour are correct.
*/

#include <gtest/gtest.h>
#include "CDCLocalTrackingTest.h"

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


TEST_F(CDCLocalTrackingTest, cpp_float)
{
  EXPECT_TRUE(signbit(-0.0));
  EXPECT_FALSE(signbit(0.0));
  EXPECT_FALSE(signbit(NAN));
}


