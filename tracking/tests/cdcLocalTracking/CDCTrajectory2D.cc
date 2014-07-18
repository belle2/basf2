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
#include "CDCLocalTrackingTest.h"

#include <tracking/cdcLocalTracking/eventdata/trajectories/CDCTrajectory2D.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


TEST_F(CDCLocalTrackingTest, CDCTrajectory2D_constructorPosMomCharge)
{
  Vector2D newMom2D(1.0, 2.0);
  Vector2D newPos2D(1.0, 2.0);
  SignType newChargeSign = PLUS;

  CDCTrajectory2D trajectory(newPos2D, newMom2D, newChargeSign);

  Vector2D mom2D = trajectory.getMom2DAtSupport();
  Vector2D pos2D = trajectory.getSupport();
  SignType chargeSign = trajectory.getChargeSign();

  EXPECT_NEAR(newMom2D.x(), mom2D.x(), 10e-7);
  EXPECT_NEAR(newMom2D.y(), mom2D.y(), 10e-7);

  EXPECT_NEAR(newPos2D.x(), pos2D.x(), 10e-7);
  EXPECT_NEAR(newPos2D.y(), pos2D.y(), 10e-7);

  EXPECT_NEAR(newChargeSign, chargeSign, 10e-7);
}
