/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackingUtilities/eventdata/trajectories/CDCTrajectory2D.h>

#include <cdc/topology/WireLine.h>

#include <framework/logging/Logger.h>

#include <Math/Vector2D.h>
#include <Math/Vector3D.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace CDC;
using namespace TrackingUtilities;


TEST(TrackingUtilitiesTest, eventdata_trajectories_CDCTrajectory2D_constructorPosMomCharge)
{
  ROOT::Math::XYVector newMom2D(1.0, 2.0);
  ROOT::Math::XYVector newPos2D(1.0, 2.0);
  double newTime = 0.0;
  ESign newChargeSign = ESign::c_Plus;
  double bZ = 2.0;

  CDCTrajectory2D trajectory(newPos2D, newTime, newMom2D, newChargeSign, bZ);

  ROOT::Math::XYVector mom2D = trajectory.getMom2DAtSupport(bZ);
  ROOT::Math::XYVector pos2D = trajectory.getSupport();
  ESign chargeSign = trajectory.getChargeSign();

  EXPECT_NEAR(newMom2D.x(), mom2D.x(), 10e-7);
  EXPECT_NEAR(newMom2D.y(), mom2D.y(), 10e-7);

  EXPECT_NEAR(newPos2D.x(), pos2D.x(), 10e-7);
  EXPECT_NEAR(newPos2D.y(), pos2D.y(), 10e-7);

  EXPECT_NEAR(newChargeSign, chargeSign, 10e-7);
}


TEST(TrackingUtilitiesTest, eventdata_trajectories_CDCTrajectory2D_reconstruct)
{
  ROOT::Math::XYZVector forward(-1.0, 1.0, 10.0);
  ROOT::Math::XYZVector backward(1.0, 1.0, -10.0);
  WireLine wireLine(forward, backward, 0);

  double localPhi0 = M_PI / 3;
  double localCurv = -2.0;
  double localImpact = 0.0;
  UncertainPerigeeCircle localPerigeeCircle(localCurv, localPhi0, localImpact);
  ROOT::Math::XYZVector localOrigin(0.5, 1, -5);

  ROOT::Math::XYZVector positionOnWire = wireLine.nominalPos3DAtZ(localOrigin.z());
  EXPECT_NEAR(localOrigin.x(), positionOnWire.x(), 10e-7);
  EXPECT_NEAR(localOrigin.y(), positionOnWire.y(), 10e-7);
  EXPECT_NEAR(localOrigin.z(), positionOnWire.z(), 10e-7);

  CDCTrajectory2D trajectory2D(VectorUtil::getXYVector(localOrigin), localPerigeeCircle);
  double arcLength2D = trajectory2D.setLocalOrigin(ROOT::Math::XYVector(0.0, 0.0));

  // Check that the old origin is still on the line
  double distance = trajectory2D.getDist2D(VectorUtil::getXYVector(localOrigin));
  EXPECT_NEAR(0, distance, 10e-7);

  // Extrapolate back to the local origin
  ROOT::Math::XYVector extrapolation2D = trajectory2D.getPos2DAtArcLength2D(-arcLength2D);
  EXPECT_NEAR(localOrigin.x(), extrapolation2D.x(), 10e-7);
  EXPECT_NEAR(localOrigin.y(), extrapolation2D.y(), 10e-7);

  ROOT::Math::XYZVector recoPos3D = trajectory2D.reconstruct3D(wireLine);
  B2INFO(trajectory2D);
  B2INFO(recoPos3D);

  EXPECT_NEAR(localOrigin.x(), recoPos3D.x(), 10e-7);
  EXPECT_NEAR(localOrigin.y(), recoPos3D.y(), 10e-7);
  EXPECT_NEAR(localOrigin.z(), recoPos3D.z(), 10e-7);
}
