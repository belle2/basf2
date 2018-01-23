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
 * This file contains test to check the behaviour of the c++ programming language.
 * Its purpose is mainly to asure the programmer that his assumptions about run time behaviour are
 * correct.
 */

#include <cmath>
#include <gtest/gtest.h>

#include <boost/optional.hpp>

#include <functional>

#include <map>
#include <vector>

TEST(TrackFindingCDCTest, cpp_float)
{
  EXPECT_TRUE(std::signbit(-0.0));
  EXPECT_FALSE(std::signbit(0.0));
  EXPECT_FALSE(std::signbit(NAN));
}

TEST(TrackFindingCDCTest, cpp_max)
{
  float value = 1.0;

  // If the values are incomparable the first one is always returned.
  double maximum = std::max(NAN, value);
  EXPECT_TRUE(std::isnan(maximum));

  double maximum2 = std::max(value, NAN);
  EXPECT_EQ(value, maximum2);
}

TEST(TrackFindingCDCTest, cpp_array_init)
{
  float values[10] = {};
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(0, values[i]);
  }
}

TEST(TrackFindingCDCTest, cpp_char_is_signed)
{
  char isSigned = -1;
  EXPECT_GT(0, isSigned);
}

TEST(TrackFindingCDCTest, cpp_stringstream_copy)
{
  // Howto copy a string stream even if its constant.
  std::stringstream filled_non_const;
  filled_non_const << "filled "
                   << "with "
                   << "stuff.";

  const std::stringstream& filled = filled_non_const;

  std::stringstream copy1;
  copy1 << filled.rdbuf();
  filled.rdbuf()->pubseekpos(0, std::ios_base::in);

  std::stringstream copy2;
  copy2 << filled.rdbuf();
  filled.rdbuf()->pubseekpos(0, std::ios_base::in);

  EXPECT_EQ(filled.str(), copy1.str());
  EXPECT_EQ(filled.str(), copy2.str());
}

TEST(TrackFindingCDCTest, cpp_map_insert)
{
  std::map<int, int> defaults{{1, 1}, {2, 2}};

  std::map<int, int> concret{{1, 10}};

  concret.insert(defaults.begin(), defaults.end());

  // Does not overwrite a value that was present before.
  EXPECT_EQ(10, concret[1]);

  // Inserts new value.
  EXPECT_EQ(2, concret[2]);
}

TEST(TrackFindingCDCTest, cpp_remainder)
{
  // Test if remainder brings a value to the range [-1, 1]
  // proving that remainder is the best function to transform an angle to range [-M_PI, M_PI]
  {
    double value = 3.0 / 2.0;
    double reduced_value = std::remainder(value, 2.0);

    EXPECT_FLOAT_EQ(-1.0 / 2.0, reduced_value);
  }

  {
    double value = -3.0 / 2.0;
    double reduced_value = std::remainder(value, 2.0);

    EXPECT_FLOAT_EQ(1.0 / 2.0, reduced_value);
  }
}

TEST(TrackFindingCDCTest, boost_optional_reference)
{
  std::vector<int> integers({1});
  boost::optional<std::vector<int>&> ref_integers(integers);
  int i = ref_integers->at(0);
  EXPECT_EQ(1, i);
}
