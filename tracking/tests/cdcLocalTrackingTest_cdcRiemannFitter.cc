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

#include <tracking/cdcLocalTracking/fitting/CDCObservations2D.h>
#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>
#include <tracking/cdcLocalTracking/fitting/CDCKarimakiFitter.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;


namespace {
  const Vector2D generalCenter(6.0, 0);
  const FloatType generalRadius = 5;

  const Circle2D generalCircle(generalCenter, generalRadius);

  CDCObservations2D createGeneralCircleObservations()
  {
    vector<Vector2D> observationCenters;

    observationCenters.emplace_back(0.0, 0.0);

    observationCenters.emplace_back(1.0, 1.0);
    observationCenters.emplace_back(1.0, -1.0);

    observationCenters.emplace_back(1.5, 1.5);
    observationCenters.emplace_back(1.5, -1.5);

    observationCenters.emplace_back(2.0, 2.0);
    observationCenters.emplace_back(2.0, -2.0);

    observationCenters.emplace_back(2.5, 2.5);
    observationCenters.emplace_back(2.5, -2.5);

    observationCenters.emplace_back(3.0, 3.0);
    observationCenters.emplace_back(3.0, -3.0);

    observationCenters.emplace_back(3.5, 3.5);
    observationCenters.emplace_back(3.5, -3.5);

    observationCenters.emplace_back(4.0, 4.0);
    observationCenters.emplace_back(4.0, -4.0);

    observationCenters.emplace_back(4.5, 4.5);
    observationCenters.emplace_back(4.5, -4.5);

    observationCenters.emplace_back(5.0, 5.0);
    observationCenters.emplace_back(5.0, -5.0);

    observationCenters.emplace_back(5.5, 5.5);
    observationCenters.emplace_back(5.5, -5.5);

    CDCObservations2D observations2D;
    for (const Vector2D & observationCenter : observationCenters) {
      FloatType distance = generalCircle.distance(observationCenter);
      observations2D.append(observationCenter, distance);
    }
    return observations2D;
  }

}



// Tests if CDCGenHitVector can handle CDCWireHits
TEST_F(CDCLocalTrackingTest, CDCRiemannFitter_LineFit)
{

  const CDCRiemannFitter& lineFitter = CDCRiemannFitter::getLineFitter();

  CDCObservations2D observations2D;
  observations2D.append(Vector2D(0, 1), 0.5);
  observations2D.append(Vector2D(1, -1), -0.5);
  observations2D.append(Vector2D(2, 1), 0.5);

  CDCTrajectory2D trajectory2D;

  lineFitter.update(trajectory2D, observations2D);

  Vector2D perigee = trajectory2D.getPerigee();

  EXPECT_NEAR(0.0, perigee.x(), 10e-7);
  EXPECT_NEAR(0.0, perigee.y(), 10e-7);

  Vector2D closestToPoint = trajectory2D.getClosest(Vector2D(2, 2));

  ASSERT_FLOAT_EQ(2.0, closestToPoint.x());
  EXPECT_NEAR(0.0, closestToPoint.y(), 10e-7);

}



TEST_F(CDCLocalTrackingTest, CDCRiemannFitter_CircleFit)
{

  // Setup a test circle
  CDCObservations2D observations2D = createGeneralCircleObservations();

  //Now fit it
  const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();

  CDCTrajectory2D trajectory2D;
  fitter.update(trajectory2D, observations2D);

  const PerigeeCircle& fittedCircle = trajectory2D.getCircle();

  EXPECT_NEAR(generalCircle.perigee().x(), fittedCircle.perigee().x(), 10e-7);
  EXPECT_NEAR(generalCircle.perigee().y(), fittedCircle.perigee().y(), 10e-7);
  EXPECT_NEAR(generalCircle.radius(), fittedCircle.radius(), 10e-7);

}



TEST_F(CDCLocalTrackingTest, CDCKarimakiFitter_CircleFit)
{

  // Setup a test circle
  CDCObservations2D observations2D = createGeneralCircleObservations();

  //Now fit it
  const CDCKarimakiFitter& fitter = CDCKarimakiFitter::getFitter();

  CDCTrajectory2D trajectory2D;
  fitter.update(trajectory2D, observations2D);

  const PerigeeCircle& fittedCircle = trajectory2D.getCircle();

  EXPECT_NEAR(generalCircle.perigee().x(), fittedCircle.perigee().x(), 10e-7);
  EXPECT_NEAR(generalCircle.perigee().y(), fittedCircle.perigee().y(), 10e-7);
  EXPECT_NEAR(generalCircle.radius(), fittedCircle.radius(), 10e-7);

}


