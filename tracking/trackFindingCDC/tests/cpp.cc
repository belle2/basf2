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

#include <cmath>
#include <gtest/gtest.h>

using namespace std;

TEST(TrackFindingCDCTest, cpp_float)
{
  EXPECT_TRUE(std::signbit(-0.0));
  EXPECT_FALSE(std::signbit(0.0));
  EXPECT_FALSE(std::signbit(NAN));
}


TEST(TrackFindingCDCTest, cpp_max)
{
  float value = 1.0;
  double maximum = std::max(NAN, value);
  EXPECT_TRUE(std::isnan(maximum));

  double maximum2 = std::max(value, NAN);
  EXPECT_EQ(value, maximum2);

}
