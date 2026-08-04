/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/utilities/MathHelpers.h>

#include <cmath>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {
  TEST(MathHelpers, square)
  {
    const double pi = 3.14159265358979323846;
    const double e  = 2.71828182845904523536;
    const double r  = (1 + std::sqrt(5)) / 2.0;
    EXPECT_NEAR(square(pi), std::pow(pi, 2), 1e-10);
    EXPECT_NEAR(square(e),  std::pow(e, 2),  1e-10);
    EXPECT_NEAR(square(r),  std::pow(r, 2),  1e-10);
  }

  TEST(MathHelpers, cube)
  {
    const double pi = 3.14159265358979323846;
    const double e  = 2.71828182845904523536;
    const double r  = (1 + std::sqrt(5)) / 2.0;
    EXPECT_NEAR(cube(pi), std::pow(pi, 3), 1e-10);
    EXPECT_NEAR(cube(e),  std::pow(e, 3),  1e-10);
    EXPECT_NEAR(cube(r),  std::pow(r, 3),  1e-10);
  }

  TEST(MathHelpers, pow4)
  {
    const double pi = 3.14159265358979323846;
    const double e  = 2.71828182845904523536;
    const double r  = (1 + std::sqrt(5)) / 2.0;
    EXPECT_NEAR(pow4(pi), std::pow(pi, 4), 1e-10);
    EXPECT_NEAR(pow4(e),  std::pow(e, 4),  1e-10);
    EXPECT_NEAR(pow4(r),  std::pow(r, 4),  1e-10);
  }

  TEST(MathHelpers, pow5)
  {
    const double pi = 3.14159265358979323846;
    const double e  = 2.71828182845904523536;
    const double r  = (1 + std::sqrt(5)) / 2.0;
    EXPECT_NEAR(pow5(pi), std::pow(pi, 5), 1e-10);
    EXPECT_NEAR(pow5(e),  std::pow(e, 5),  1e-10);
    EXPECT_NEAR(pow5(r),  std::pow(r, 5),  1e-10);
  }
}
