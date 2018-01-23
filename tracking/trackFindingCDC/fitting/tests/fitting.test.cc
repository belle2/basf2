/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCFitter2D.icc.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCSZObservations.h>

#include <tracking/trackFindingCDC/fitting/ExtendedRiemannsMethod.h>
#include <tracking/trackFindingCDC/fitting/RiemannsMethod.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/geometry/UncertainSZLine.h>
#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>
#include <tracking/trackFindingCDC/geometry/Circle2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithTopology.h>

#include <framework/logging/Logger.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  const Vector2D generalCenter(6.0, 0);
  const double generalRadius = 5.0;

  const Circle2D generalCircle(generalCenter, generalRadius);

  std::vector<Vector2D> createGeneralCircleObservationCenters()
  {
    std::vector<Vector2D> observationCenters;
    // Setting up a trajectory traveling clockwise
    for (int iObservation = 11; iObservation > -12; --iObservation) {
      double y = iObservation / 2.0;
      double x = fabs(y);
      observationCenters.emplace_back(x, y);
    }
    return observationCenters;
  }

  CDCObservations2D createGeneralCircleObservations(bool withDriftLength = true)
  {
    std::vector<Vector2D> observationCenters = createGeneralCircleObservationCenters();

    CDCObservations2D observations2D;
    if (withDriftLength) {
      for (const Vector2D& observationCenter : observationCenters) {
        double distance = generalCircle.distance(observationCenter);
        observations2D.fill(observationCenter, distance);
      }
    } else {
      for (const Vector2D& observationCenter : observationCenters) {
        Vector2D onCircle = generalCircle.closest(observationCenter);
        observations2D.fill(onCircle, 0.0);
      }
    }
    return observations2D;
  }

  template<class AFitter>
  CDCTrajectory2D testGeneralCircleFitter(const AFitter& fitter, const bool withDriftLenght)
  {
    // Setup a test circle
    CDCObservations2D observations2D = createGeneralCircleObservations(withDriftLenght);
    CDCTrajectory2D trajectory2D;
    fitter.update(trajectory2D, observations2D);

    trajectory2D.setLocalOrigin(Vector2D(0.0, 0.0));
    const UncertainPerigeeCircle& fittedCircle = trajectory2D.getLocalCircle();

    EXPECT_NEAR(generalCircle.perigee().x(), fittedCircle->perigee().x(), 10e-7) <<
        "AFitter " << typeid(fitter).name() << " failed.";
    EXPECT_NEAR(generalCircle.perigee().y(), fittedCircle->perigee().y(), 10e-7) <<
        "AFitter " << typeid(fitter).name() << " failed.";
    EXPECT_NEAR(generalCircle.radius(), fittedCircle->radius(), 10e-7) <<
        "AFitter " << typeid(fitter).name() << " failed.";
    EXPECT_NEAR(generalCircle.tangentialPhi(), fittedCircle->phi0(), 10e-7) <<
        "AFitter " << typeid(fitter).name() << " failed.";
    EXPECT_NEAR(0.0, fittedCircle.chi2(), 10e-7) << "AFitter " << typeid(fitter).name() << " failed.";

    const double curv = fittedCircle->curvature();
    const double I = fittedCircle->impact();
    const double phi = fittedCircle->phi0();

    const double chi2 = fittedCircle.chi2();

    B2INFO("Curvature " << curv);
    B2INFO("Tangential phi " << phi);
    B2INFO("Impact " << I);
    B2INFO("Chi2 " << chi2);

    B2INFO("Covariance matrix:");
    B2INFO(fittedCircle.perigeeCovariance());
    return trajectory2D;
  }



  Line2D line(-0.5, 0.0, -1.0);

  template<class AFitter>
  CDCTrajectory2D testLineFitter(const AFitter& fitter)
  {

    CDCObservations2D observations2D;
    observations2D.fill(Vector2D(0, 0), 0.5);
    observations2D.fill(Vector2D(1, -1), -0.5);
    observations2D.fill(Vector2D(2, 0), 0.5);

    CDCTrajectory2D trajectory2D;

    fitter.update(trajectory2D, observations2D);

    trajectory2D.setLocalOrigin(Vector2D(0.0, 0.0));
    const UncertainPerigeeCircle& fittedCircle = trajectory2D.getLocalCircle();

    EXPECT_EQ(0.0, fittedCircle->curvature()) << "AFitter " << typeid(fitter).name() << " failed.";

    Vector2D perigee = trajectory2D.getGlobalPerigee();
    EXPECT_NEAR(0.0, perigee.x(), 10e-7);
    EXPECT_NEAR(-0.5, perigee.y(), 10e-7);

    return trajectory2D;
  }

}



TEST(TrackFindingCDCTest, fitting_ExtendedRiemannsMethod_GeneralCircleFit_NoDriftLength)
{
  const CDCFitter2D<ExtendedRiemannsMethod> fitter;
  CDCTrajectory2D trajectory2D = testGeneralCircleFitter(fitter, false);

  trajectory2D.setLocalOrigin(Vector2D(0.0, 0.0));
  const UncertainPerigeeCircle& perigeeCircle = trajectory2D.getLocalCircle();
  PerigeeCovariance perigeeCovariance = perigeeCircle.perigeeCovariance();

  EXPECT_NEAR(0.0003644, perigeeCovariance(0, 0), 10e-7);
  EXPECT_NEAR(3.028e-20, perigeeCovariance(0, 1), 10e-7);
  EXPECT_NEAR(-0.002235, perigeeCovariance(0, 2), 10e-7);

  EXPECT_NEAR(3.028e-20, perigeeCovariance(1, 0), 10e-7);
  EXPECT_NEAR(0.002525, perigeeCovariance(1, 1), 10e-7);
  EXPECT_NEAR(-1.265e-19, perigeeCovariance(1, 2), 10e-7);

  EXPECT_NEAR(-0.002235, perigeeCovariance(2, 0), 10e-7);
  EXPECT_NEAR(-1.265e-19, perigeeCovariance(2, 1), 10e-7);
  EXPECT_NEAR(0.09703616, perigeeCovariance(2, 2), 10e-7);
}



TEST(TrackFindingCDCTest, fitting_ExtendedRiemannsMethod_GeneralCircleFit_WithDriftLengths)
{
  const CDCFitter2D<ExtendedRiemannsMethod> fitter;
  testGeneralCircleFitter(fitter, true);
}


TEST(TrackFindingCDCTest, fitting_RiemannsMethod_GeneralCircleFit_WithDriftLengths)
{
  const CDCFitter2D<RiemannsMethod> fitter;
  testGeneralCircleFitter(fitter, true);
}



TEST(TrackFindingCDCTest, fitting_CDCRiemannFitter_CircleFit_NoDriftLength)
{
  CDCRiemannFitter fitter = CDCRiemannFitter::getFitter();
  testGeneralCircleFitter(fitter, false);
}



TEST(TrackFindingCDCTest, fitting_CDCKarimakiFitter_CircleFit_NoDriftLength)
{
  CDCKarimakiFitter fitter = CDCKarimakiFitter::getFitter();
  fitter.useOnlyPosition();
  testGeneralCircleFitter(fitter, false);
}



TEST(TrackFindingCDCTest, fitting_CDCRiemannFitter_LineFit_WithDriftLength)
{
  CDCRiemannFitter fitter = CDCRiemannFitter::getLineFitter();
  testLineFitter(fitter);
}

TEST(TrackFindingCDCTest, fitting_CDCSZFitter)
{
  CDCSZFitter fitter;
  CDCSZObservations szObservations;

  szObservations.fill(0, 0);
  szObservations.fill(5, 3);
  szObservations.fill(10, 0);

  CDCTrajectorySZ trajectorySZ = fitter.fit(szObservations);
  const UncertainSZLine& fittedSZLine = trajectorySZ.getSZLine();

  EXPECT_NEAR(0.0, fittedSZLine->slope(), 10e-10);
  EXPECT_NEAR(1.0, fittedSZLine->intercept(), 10e-10);
}
