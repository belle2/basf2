/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLocalTracking/geometry/Helix.h>

#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


TEST(CDCLocalTrackingTest, geometry_Helix_closest)
{
  // Tests the calculation of points of closest approach under various orientation of the helix.
  // Here this is done by moving the coordinate system and thereby changing the perigee point to different representations.
  // The point of closest approach should always lie at the position of the original origin / its transformed counterpart.
  std::vector<Vector3D> bys = {
    Vector3D(0.0, 0.0, 0.0),
    Vector3D(0.0, 0.0, 5.0),
    Vector3D(0.0, 3.0, 0.0),
    Vector3D(3.0, 0.0, 0.0),
    Vector3D(-1.0, -2.0, 3.0)
  };


  for (const Vector3D & by : bys) {
    FloatType curvature = +1.0 / 2.0;
    FloatType phi0 = -PI / 2.0;
    FloatType impact = +1.0;
    FloatType szSlope = 1.0 / 2.0;
    FloatType z0 = 0.0;

    Helix helix(curvature, phi0, impact, szSlope, z0);
    Vector3D point(0.0, 0.0, 0.0);

    Vector3D expectedClosest = helix.support();

    helix.passiveMoveBy(by);
    expectedClosest.passiveMoveBy(by);
    point.passiveMoveBy(by);

    Vector3D realClosest = helix.closest(point);

    EXPECT_NEAR(expectedClosest.x(), realClosest.x(), 10e-7) << "Test for displacement by " << by;
    EXPECT_NEAR(expectedClosest.y(), realClosest.y(), 10e-7) << "Test for displacement by " << by;
    EXPECT_NEAR(expectedClosest.z(), realClosest.z(), 10e-7) << "Test for displacement by " << by;
  }
}
