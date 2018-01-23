/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/topology/WireLine.h>

#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;


TEST(TrackFindingCDCTest, eventdata_trajectories_CDCTrajectory2D_constructorPosMomCharge)
{
  Vector2D newMom2D(1.0, 2.0);
  Vector2D newPos2D(1.0, 2.0);
  double newTime = 0.0;
  ESign newChargeSign = ESign::c_Plus;
  double bZ = 2.0;

  CDCTrajectory2D trajectory(newPos2D, newTime, newMom2D, newChargeSign, bZ);

  Vector2D mom2D = trajectory.getMom2DAtSupport(bZ);
  Vector2D pos2D = trajectory.getSupport();
  ESign chargeSign = trajectory.getChargeSign();

  EXPECT_NEAR(newMom2D.x(), mom2D.x(), 10e-7);
  EXPECT_NEAR(newMom2D.y(), mom2D.y(), 10e-7);

  EXPECT_NEAR(newPos2D.x(), pos2D.x(), 10e-7);
  EXPECT_NEAR(newPos2D.y(), pos2D.y(), 10e-7);

  EXPECT_NEAR(newChargeSign, chargeSign, 10e-7);
}


TEST(TrackFindingCDCTest, eventdata_trajectories_CDCTrajectory2D_reconstruct)
{
  Vector3D forward(-1.0, 1.0, 10.0);
  Vector3D backward(1.0, 1.0, -10.0);
  WireLine wireLine(forward, backward, 0);

  double localPhi0 = M_PI / 3;
  double localCurv = -2.0;
  double localImpact = 0.0;
  PerigeeCircle localPerigeeCircle(localCurv, localPhi0, localImpact);
  Vector3D localOrigin(0.5, 1, -5);

  Vector3D positionOnWire = wireLine.nominalPos3DAtZ(localOrigin.z());
  EXPECT_NEAR(localOrigin.x(), positionOnWire.x(), 10e-7);
  EXPECT_NEAR(localOrigin.y(), positionOnWire.y(), 10e-7);
  EXPECT_NEAR(localOrigin.z(), positionOnWire.z(), 10e-7);

  CDCTrajectory2D trajectory2D(localOrigin.xy(), localPerigeeCircle);
  double arcLength2D = trajectory2D.setLocalOrigin(Vector2D(0.0, 0.0));

  // Check that the old origin is still on the line
  double distance = trajectory2D.getDist2D(localOrigin.xy());
  EXPECT_NEAR(0, distance, 10e-7);

  // Extrapolate back to the local origin
  Vector2D extrapolation2D = trajectory2D.getPos2DAtArcLength2D(-arcLength2D);
  EXPECT_NEAR(localOrigin.x(), extrapolation2D.x(), 10e-7);
  EXPECT_NEAR(localOrigin.y(), extrapolation2D.y(), 10e-7);

  Vector3D recoPos3D = trajectory2D.reconstruct3D(wireLine);
  B2INFO(trajectory2D);
  B2INFO(recoPos3D);

  EXPECT_NEAR(localOrigin.x(), recoPos3D.x(), 10e-7);
  EXPECT_NEAR(localOrigin.y(), recoPos3D.y(), 10e-7);
  EXPECT_NEAR(localOrigin.z(), recoPos3D.z(), 10e-7);
}
