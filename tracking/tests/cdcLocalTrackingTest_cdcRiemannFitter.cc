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

#include <tracking/cdcLocalTracking/eventdata/entities/CDCGenHit.h>
#include <tracking/cdcLocalTracking/topology/CDCWire.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <tracking/cdcLocalTracking/geometry/Vector2D.h>
#include <tracking/cdcLocalTracking/geometry/GeneralizedCircle.h>

#include <tracking/cdcLocalTracking/fitting/CDCObservations2D.h>
#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;

// Tests if CDCGenHitVector can handle CDCWireHits
TEST_F(CDCLocalTrackingTest, CDCRiemannFitter_LineFit)
{

  CDCRiemannFitter lineFitter = CDCRiemannFitter::getLineFitter();

  CDCObservations2D observations2D;
  observations2D.append(Vector2D(0, 1), 0.5);
  observations2D.append(Vector2D(1, -1), -0.5);
  observations2D.append(Vector2D(2, 1), 0.5);

  CDCTrajectory2D trajectory2D;

  lineFitter.update(trajectory2D, observations2D);

  //CDCGenerailzedCircle& genCircle = trajectory2D.getGenCircle();

  Vector2D closestToOrigin = trajectory2D.getClosestToOrigin();

  EXPECT_NEAR(0.0, closestToOrigin.x(), 10e-7);
  EXPECT_NEAR(0.0, closestToOrigin.y(), 10e-7);

  Vector2D closestToPoint = trajectory2D.getClosest(Vector2D(2, 2));

  ASSERT_FLOAT_EQ(2.0, closestToPoint.x());
  EXPECT_NEAR(0.0, closestToPoint.y(), 10e-7);

}





