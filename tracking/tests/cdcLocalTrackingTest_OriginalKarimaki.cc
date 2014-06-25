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
#include <tracking/cdcLocalTracking/fitting/CDCFitter2D.h>
#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>
#include <tracking/cdcLocalTracking/fitting/CDCKarimakiFitter.h>
#include <tracking/cdcLocalTracking/fitting/OriginalKarimakisMethod.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;

namespace {
  const Vector2D generalCenter(6.0, 0);
  const FloatType generalRadius = 5.0;

  const Circle2D generalCircle(generalCenter, generalRadius);

  vector<Vector2D> createObservationCenters()
  {
    vector<Vector2D> observationCenters;
    // Setting up a trajectory traveling clockwise
    for (int iObservation = 11; iObservation > -12; --iObservation) {
      FloatType y = iObservation / 2.0;
      FloatType x = fabs(y);
      observationCenters.emplace_back(x, y);
    }
    return observationCenters;
  }

  CDCObservations2D createGeneralCircleObservations(bool withDriftLength = true)
  {
    vector<Vector2D>&&  observationCenters = createObservationCenters();

    CDCObservations2D observations2D;
    if (withDriftLength) {
      for (const Vector2D & observationCenter : observationCenters) {
        FloatType distance = generalCircle.distance(observationCenter);
        observations2D.append(observationCenter, distance);
      }
    } else {
      for (const Vector2D & observationCenter : observationCenters) {
        Vector2D onCircle = generalCircle.closest(observationCenter);
        observations2D.append(onCircle, 0.0);
      }
    }
    return observations2D;
  }

  template<class Fitter>
  CDCTrajectory2D testCircleFitter(const Fitter& fitter, const bool withDriftLenght)
  {
    // Setup a test circle
    CDCObservations2D observations2D = createGeneralCircleObservations(withDriftLenght);
    CDCTrajectory2D trajectory2D;
    fitter.update(trajectory2D, observations2D);

    const PerigeeCircle& fittedCircle = trajectory2D.getCircle();

    EXPECT_NEAR(generalCircle.perigee().x(), fittedCircle.perigee().x(), 10e-7) <<
        "Fitter " << typeid(fitter).name() << " failed.";
    EXPECT_NEAR(generalCircle.perigee().y(), fittedCircle.perigee().y(), 10e-7) <<
        "Fitter " << typeid(fitter).name() << " failed.";
    EXPECT_NEAR(generalCircle.radius(), fittedCircle.radius(), 10e-7) <<
        "Fitter " << typeid(fitter).name() << " failed.";

    B2INFO("Covariance matrix:");
    trajectory2D.getCircle().perigeeCovariance().Print();
    return trajectory2D;
  }
}



TEST_F(CDCLocalTrackingTest, OriginalKarimakisMethod_CircleFit)
{
  const CDCFitter2D<OriginalKarimakisMethod> fitter;
  testCircleFitter(fitter, false);
}

TEST_F(CDCLocalTrackingTest, PortedKarimakisMethod_CircleFit)
{
  const CDCFitter2D<PortedKarimakisMethod> fitter;
  testCircleFitter(fitter, false);
}


// TEST_F(CDCLocalTrackingTest, CDCRiemannFitter_CircleFit_WithoutDriftLength)
// {
//   CDCRiemannFitter fitter = CDCRiemannFitter::getFitter();
//   testCircleFitter(fitter, false);
// }


TEST_F(CDCLocalTrackingTest, CDCKarimakiFitter_CircleFit_WithoutDriftLength)
{
  CDCKarimakiFitter fitter = CDCKarimakiFitter::getFitter();
  fitter.useOnlyPosition();
  testCircleFitter(fitter, false);
}



