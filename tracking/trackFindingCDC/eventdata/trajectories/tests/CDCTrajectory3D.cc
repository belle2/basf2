/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLocalTracking/eventdata/trajectories/CDCTrajectory3D.h>

#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


TEST(CDCLocalTrackingTest, eventdata_trajectories_CDCTrajectory3D_constructorPosMomCharge)
{
  Vector3D newMom3D(1.0, 2.0, 1.0);
  Vector3D newPos3D(1.0, 2.0, 1.0);
  SignType newChargeSign = PLUS;

  CDCTrajectory3D trajectory(newPos3D, newMom3D, newChargeSign);

  Vector3D mom3D = trajectory.getMom3DAtSupport();
  Vector3D pos3D = trajectory.getSupport();
  SignType chargeSign = trajectory.getChargeSign();


  EXPECT_NEAR(newMom3D.x(), mom3D.x(), 10e-7);
  EXPECT_NEAR(newMom3D.y(), mom3D.y(), 10e-7);
  EXPECT_NEAR(newMom3D.z(), mom3D.z(), 10e-7);

  EXPECT_NEAR(newPos3D.x(), pos3D.x(), 10e-7);
  EXPECT_NEAR(newPos3D.y(), pos3D.y(), 10e-7);
  EXPECT_NEAR(newPos3D.z(), pos3D.z(), 10e-7);

  EXPECT_NEAR(newChargeSign, chargeSign, 10e-7);
}



TEST(CDCLocalTrackingTest, CDCTrajectory3D_clear)
{
  Vector3D newMom3D(1.0, 2.0, 1.0);
  Vector3D newPos3D(1.0, 2.0, 1.0);
  SignType newChargeSign = PLUS;

  CDCTrajectory3D trajectory3D(newPos3D, newMom3D, newChargeSign);
  UncertainHelix uncertainHelix = trajectory3D.getLocalHelix();
  Helix helix = uncertainHelix;

  PerigeeCircle perigeeCircle = helix.circleXY();
  GeneralizedCircle generalizedCircle = perigeeCircle;

  Line2D lineSZ = helix.lineSZ();

  trajectory3D.clear();
  EXPECT_TRUE(trajectory3D.isNull());
  EXPECT_FALSE(trajectory3D.isFitted());

  uncertainHelix.setNull();
  EXPECT_TRUE(uncertainHelix.isNull());

  helix.setNull();
  EXPECT_TRUE(helix.isNull());

  perigeeCircle.setNull();
  EXPECT_TRUE(perigeeCircle.isNull());

  generalizedCircle.setNull();
  EXPECT_TRUE(generalizedCircle.isNull());

  lineSZ.setNull();
  EXPECT_TRUE(lineSZ.isNull());
}
