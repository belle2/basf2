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

#include <tracking/cdcLocalTracking/geometry/Vector2D.h>
#include <tracking/cdcLocalTracking/geometry/PerigeeCircle.h>
#include <tracking/cdcLocalTracking/geometry/GeneralizedCircle.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;



TEST_F(CDCLocalTrackingTest, PerigeeCircle_n)
{

  FloatType curvature = 1.0;
  FloatType tangtialPhi = PI / 4.0;
  FloatType impact = 1.0;

  // Checks if the normal parameters n follow the same sign convention
  PerigeeCircle perigeeCircle(curvature, tangtialPhi, impact);

  GeneralizedCircle generalizedCircle(perigeeCircle.n0(), perigeeCircle.n12(), perigeeCircle.n3());

  EXPECT_NEAR(curvature, generalizedCircle.signedCurvature(), 10e-7);
  EXPECT_NEAR(impact, generalizedCircle.impact(), 10e-7);
  EXPECT_NEAR(perigeeCircle.tangential().x(), generalizedCircle.tangential().x(), 10e-7);
  EXPECT_NEAR(perigeeCircle.tangential().y(), generalizedCircle.tangential().y(), 10e-7);

}


TEST_F(CDCLocalTrackingTest, PerigeeCircle_minimalPolarR)
{
  FloatType curvature = 1.0 / 2.0;
  FloatType tangtialPhi = PI / 4.0;
  FloatType impact = -1.0;

  // Checks if the normal parameters n follow the same sign convention
  PerigeeCircle perigeeCircle(curvature, tangtialPhi, impact);

  EXPECT_EQ(1, perigeeCircle.minimalPolarR());
}


TEST_F(CDCLocalTrackingTest, PerigeeCircle_maximalPolarR)
{
  FloatType curvature = 1.0 / 2.0;
  FloatType tangtialPhi = PI / 4.0;
  FloatType impact = -1.0;

  // Checks if the normal parameters n follow the same sign convention
  PerigeeCircle perigeeCircle(curvature, tangtialPhi, impact);

  EXPECT_EQ(3, perigeeCircle.maximalPolarR());
}
