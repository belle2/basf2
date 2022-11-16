/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/utilities/beamHelpers.h>
#include <gtest/gtest.h>
#include <cmath>

using namespace std;
using namespace Belle2;

namespace {

  /** Tests for dual numbers */
  class DualNumbersTests : public ::testing::Test {
  protected:

    /** dummy setup */
    virtual void SetUp() { }

    /** dummy */
    virtual void TearDown() { }

  };

  /** Tests for dual numbers */
  TEST_F(DualNumbersTests, TestDerivatives)
  {
    DualNumber x0(0, 1);

    EXPECT_NEAR(tan(x0).x,  0, 1e-15); // value of tan(x) for x = 0
    EXPECT_NEAR(tan(x0).dx, 1, 1e-15); // derivative of tan(x) for x = 0

    EXPECT_NEAR(atan(x0).x,  0,  1e-15);  // value of atan(x) for x = 0
    EXPECT_NEAR(atan(x0).dx, 1., 1e-15);  // derivative of atan(x) for x = 0

    DualNumber x4(4, 1);

    EXPECT_NEAR(sqrt(x4).x,  2,    1e-15);  // value of sqrt(x) for x = 4
    EXPECT_NEAR(sqrt(x4).dx, 1. / 4, 1e-15); // derivative of sqrt(x) for x = 4


    //derivative of product
    EXPECT_NEAR((tan(x0) * x0).x,  0, 1e-15);  // value of x tan(x) for x = 0
    EXPECT_NEAR((tan(x0) * x0).dx, 0, 1e-15);  // derivative of x tan(x) for x = 0

    //derivative of ratio
    DualNumber xPi(M_PI, 1);

    EXPECT_NEAR((tan(xPi) / xPi).x,   0,       1e-15);   // value of tan(x)/x for x = pi
    EXPECT_NEAR((tan(xPi) / xPi).dx,  1. / M_PI, 1e-15); // derivative of tan(x)/x for x = pi
  }


  /** Tests for dual numbers and vectors */
  TEST_F(DualNumbersTests, TestVectorDerivatives)
  {
    GeneralVector<DualNumber> r(DualNumber(1, 1), DualNumber(1, 0), DualNumber(1, 0));

    EXPECT_NEAR((-1.0 /  sqrt(r.norm2())).x, -1. / sqrt(3),       1e-15); //      -1/r   for r=(1,1,1)
    EXPECT_NEAR((-1.0 /  sqrt(r.norm2())).dx, 1. / (3 * sqrt(3)), 1e-15); // grad -1/r   for r=(1,1,1), x-component

    EXPECT_NEAR(dot(r, r).x,  r.norm2().x, 1e-15); //      dot(r,r) == |r|^2
    EXPECT_NEAR(dot(r, r).dx, 2.,          1e-15); // grad dot(r,r) == 2 for r =(1,1,1), x-component
  }

}
