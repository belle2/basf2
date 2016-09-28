/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <gtest/gtest.h>


using namespace Belle2;
using namespace TrackFindingCDC;

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_passiveMoveByJacobian_identity)
{
  for (double curvature : { -1.0, 0.0, 1.0}) {
    PerigeeCircle circle(curvature, -M_PI / 2, 0.5);

    PerigeeJacobian moveByZeroJacobian = circle.passiveMoveByJacobian(Vector2D(0.0, 0.0));
    EXPECT_NEAR(1.0, moveByZeroJacobian(0, 0), 10e-7);
    EXPECT_NEAR(0.0, moveByZeroJacobian(0, 1), 10e-7);
    EXPECT_NEAR(0.0, moveByZeroJacobian(0, 2), 10e-7);

    EXPECT_NEAR(0.0, moveByZeroJacobian(1, 0), 10e-7);
    EXPECT_NEAR(1.0, moveByZeroJacobian(1, 1), 10e-7);
    EXPECT_NEAR(0.0, moveByZeroJacobian(1, 2), 10e-7);

    EXPECT_NEAR(0.0, moveByZeroJacobian(2, 0), 10e-7);
    EXPECT_NEAR(0.0, moveByZeroJacobian(2, 1), 10e-7);
    EXPECT_NEAR(1.0, moveByZeroJacobian(2, 2), 10e-7);
  }
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_passiveMoveByJacobian_roundtrip)
{
  for (double curvature : { -1.0, 0.0, 1.0}) {
    PerigeeCircle circle(curvature, -M_PI / 2, 0.5);
    Vector2D by(0.1, 1.0);

    PerigeeJacobian moveByOneJacobian = circle.passiveMoveByJacobian(by);

    circle.passiveMoveBy(by);

    PerigeeJacobian moveOneBackJacobian = circle.passiveMoveByJacobian(-by);
    {
      PerigeeJacobian moveByZeroJacobian = moveByOneJacobian * moveOneBackJacobian;

      EXPECT_NEAR(1.0, moveByZeroJacobian(0, 0), 10e-7);
      EXPECT_NEAR(0.0, moveByZeroJacobian(0, 1), 10e-7);
      EXPECT_NEAR(0.0, moveByZeroJacobian(0, 2), 10e-7);

      EXPECT_NEAR(0.0, moveByZeroJacobian(1, 0), 10e-7);
      EXPECT_NEAR(1.0, moveByZeroJacobian(1, 1), 10e-7);
      EXPECT_NEAR(0.0, moveByZeroJacobian(1, 2), 10e-7);

      EXPECT_NEAR(0.0, moveByZeroJacobian(2, 0), 10e-7);
      EXPECT_NEAR(0.0, moveByZeroJacobian(2, 1), 10e-7);
      EXPECT_NEAR(1.0, moveByZeroJacobian(2, 2), 10e-7);
    }
    {
      PerigeeJacobian moveByZeroJacobian = moveOneBackJacobian * moveByOneJacobian;

      EXPECT_NEAR(1.0, moveByZeroJacobian(0, 0), 10e-7);
      EXPECT_NEAR(0.0, moveByZeroJacobian(0, 1), 10e-7);
      EXPECT_NEAR(0.0, moveByZeroJacobian(0, 2), 10e-7);

      EXPECT_NEAR(0.0, moveByZeroJacobian(1, 0), 10e-7);
      EXPECT_NEAR(1.0, moveByZeroJacobian(1, 1), 10e-7);
      EXPECT_NEAR(0.0, moveByZeroJacobian(1, 2), 10e-7);

      EXPECT_NEAR(0.0, moveByZeroJacobian(2, 0), 10e-7);
      EXPECT_NEAR(0.0, moveByZeroJacobian(2, 1), 10e-7);
      EXPECT_NEAR(1.0, moveByZeroJacobian(2, 2), 10e-7);
    }
  }
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_passiveMoveByJacobian)
{
  PerigeeCircle circle(1.0, -M_PI / 2, 0);

  PerigeeJacobian moveByOneJacobian = circle.passiveMoveByJacobian(Vector2D(-1.0, 0.0));
  EXPECT_NEAR(1.0, moveByOneJacobian(0, 0), 10e-7);
  EXPECT_NEAR(0.0, moveByOneJacobian(0, 1), 10e-7);
  EXPECT_NEAR(0.0, moveByOneJacobian(0, 2), 10e-7);

  EXPECT_NEAR(0.0, moveByOneJacobian(1, 0), 10e-7);
  EXPECT_NEAR(1.0 / 2.0, moveByOneJacobian(1, 1), 10e-7);
  EXPECT_NEAR(0.0, moveByOneJacobian(1, 2), 10e-7);

  EXPECT_NEAR(0.0, moveByOneJacobian(2, 0), 10e-7);
  EXPECT_NEAR(0.0, moveByOneJacobian(2, 1), 10e-7);
  EXPECT_NEAR(1.0, moveByOneJacobian(2, 2), 10e-7);

  PerigeeJacobian moveByTwoYJacobian = circle.passiveMoveByJacobian(Vector2D(0.0, -2.0));

  // Hand caluclated intermediate quantities;
  double deltaParallel = 2;
  double A = 4;
  double u = 1;

  double nu = 1;
  // double xi = 1.0 / 5.0;
  double lambda = 1.0 / (5.0 + 3.0 * sqrt(5.0));
  double mu = sqrt(5.0) / 10.0;
  double zeta = 4;

  EXPECT_NEAR(1.0, moveByTwoYJacobian(0, 0), 10e-7);
  EXPECT_NEAR(0.0, moveByTwoYJacobian(0, 1), 10e-7);
  EXPECT_NEAR(0.0, moveByTwoYJacobian(0, 2), 10e-7);

  EXPECT_NEAR(2.0 / 5.0, moveByTwoYJacobian(1, 0), 10e-7);
  EXPECT_NEAR(1.0 / 5.0, moveByTwoYJacobian(1, 1), 10e-7);
  EXPECT_NEAR(-2.0 / 5.0, moveByTwoYJacobian(1, 2), 10e-7);

  EXPECT_NEAR(mu * zeta - A * lambda, moveByTwoYJacobian(2, 0), 10e-7);
  EXPECT_NEAR(2.0 * mu * u * deltaParallel, moveByTwoYJacobian(2, 1), 10e-7);
  EXPECT_NEAR(2.0 * mu * nu, moveByTwoYJacobian(2, 2), 10e-7);
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_passiveMovedCovarianceBy)
{
  PerigeeCovariance perigeeVariance;
  perigeeVariance(0, 0) = 1.0;
  perigeeVariance(0, 1) = 0.0;
  perigeeVariance(0, 2) = 0.0;

  perigeeVariance(1, 0) = 0.0;
  perigeeVariance(1, 1) = 0.0;
  perigeeVariance(1, 2) = 0.0;

  perigeeVariance(2, 0) = 0.0;
  perigeeVariance(2, 1) = 0.0;
  perigeeVariance(2, 2) = 0.0;

  UncertainPerigeeCircle circle(1.0, -M_PI / 2, 0.0, PerigeeCovariance(perigeeVariance));

  {
    PerigeeCovariance noMoveVariance = circle.passiveMovedCovarianceBy(Vector2D(0.0, 0.0));

    EXPECT_NEAR(1.0, noMoveVariance(0, 0), 10e-7);
    EXPECT_NEAR(0.0, noMoveVariance(0, 1), 10e-7);
    EXPECT_NEAR(0.0, noMoveVariance(0, 2), 10e-7);

    EXPECT_NEAR(0.0, noMoveVariance(1, 0), 10e-7);
    EXPECT_NEAR(0.0, noMoveVariance(1, 1), 10e-7);
    EXPECT_NEAR(0.0, noMoveVariance(1, 2), 10e-7);

    EXPECT_NEAR(0.0, noMoveVariance(2, 0), 10e-7);
    EXPECT_NEAR(0.0, noMoveVariance(2, 1), 10e-7);
    EXPECT_NEAR(0.0, noMoveVariance(2, 2), 10e-7);
  }

  {
    PerigeeCovariance noChangeMoveVariance = circle.passiveMovedCovarianceBy(Vector2D(-1.0, 0.0));

    EXPECT_NEAR(1.0, noChangeMoveVariance(0, 0), 10e-7);
    EXPECT_NEAR(0.0, noChangeMoveVariance(0, 1), 10e-7);
    EXPECT_NEAR(0.0, noChangeMoveVariance(0, 2), 10e-7);

    EXPECT_NEAR(0.0, noChangeMoveVariance(1, 0), 10e-7);
    EXPECT_NEAR(0.0, noChangeMoveVariance(1, 1), 10e-7);
    EXPECT_NEAR(0.0, noChangeMoveVariance(1, 2), 10e-7);

    EXPECT_NEAR(0.0, noChangeMoveVariance(2, 0), 10e-7);
    EXPECT_NEAR(0.0, noChangeMoveVariance(2, 1), 10e-7);
    EXPECT_NEAR(0.0, noChangeMoveVariance(2, 2), 10e-7);
  }

  {
    PerigeeCovariance transformedVariance = circle.passiveMovedCovarianceBy(Vector2D(2.0, 0.0));

    EXPECT_NEAR(1.0, transformedVariance(0, 0), 10e-7);

    EXPECT_NEAR(2.0, transformedVariance(2, 0), 10e-7);
    EXPECT_NEAR(2.0, transformedVariance(0, 2), 10e-7);
    EXPECT_NEAR(4.0, transformedVariance(2, 2), 10e-7);

    // Variances in phi should vanish
    EXPECT_NEAR(0.0, transformedVariance(1, 1), 10e-7);

    EXPECT_NEAR(0.0, transformedVariance(0, 1), 10e-7);
    EXPECT_NEAR(0.0, transformedVariance(1, 0), 10e-7);

    EXPECT_NEAR(0.0, transformedVariance(1, 2), 10e-7);
    EXPECT_NEAR(0.0, transformedVariance(2, 1), 10e-7);
  }

  {
    // Should be same as before
    PerigeeCovariance transformedVariance = circle.passiveMovedCovarianceBy(Vector2D(2.5, 0.0));

    EXPECT_NEAR(1, transformedVariance(0, 0), 10e-7);

    EXPECT_NEAR(2, transformedVariance(2, 0), 10e-7);
    EXPECT_NEAR(2, transformedVariance(0, 2), 10e-7);
    EXPECT_NEAR(4, transformedVariance(2, 2), 10e-7);

    // Variances in phi should vanish
    EXPECT_NEAR(0, transformedVariance(1, 1), 10e-7);

    EXPECT_NEAR(0, transformedVariance(0, 1), 10e-7);
    EXPECT_NEAR(0, transformedVariance(1, 0), 10e-7);

    EXPECT_NEAR(0, transformedVariance(1, 2), 10e-7);
    EXPECT_NEAR(0, transformedVariance(2, 1), 10e-7);
  }
}

TEST(TrackFindingCDCTest, geometry_PerigeeCircle_passiveMove)
{

  PerigeeCovariance perigeeVariance;
  perigeeVariance(0, 0) = 1.0;
  perigeeVariance(0, 1) = 0.3;
  perigeeVariance(0, 2) = 0.5;

  perigeeVariance(1, 0) = 0.3;
  perigeeVariance(1, 1) = 0.7;
  perigeeVariance(1, 2) = 0.6;

  perigeeVariance(2, 0) = 0.5;
  perigeeVariance(2, 1) = 0.6;
  perigeeVariance(2, 2) = 1;

  UncertainPerigeeCircle circle(1.0, -M_PI / 4, 1.0, PerigeeCovariance(perigeeVariance));

  // circle.perigeeCovariance().Print();

  // Test if the move commutes
  circle.passiveMoveBy(Vector2D(0.0, -1.0));

  // circle.perigeeCovariance().Print();

  circle.passiveMoveBy(Vector2D(0.0, 1.0));

  // circle.perigeeCovariance().Print();

  PerigeeCovariance twiceMovedVariance = circle.perigeeCovariance();

  EXPECT_NEAR(perigeeVariance(0, 0), twiceMovedVariance(0, 0), 10e-7);
  EXPECT_NEAR(perigeeVariance(0, 1), twiceMovedVariance(0, 1), 10e-7);
  EXPECT_NEAR(perigeeVariance(0, 2), twiceMovedVariance(0, 2), 10e-7);

  EXPECT_NEAR(perigeeVariance(1, 0), twiceMovedVariance(1, 0), 10e-7);
  EXPECT_NEAR(perigeeVariance(1, 1), twiceMovedVariance(1, 1), 10e-7);
  EXPECT_NEAR(perigeeVariance(1, 2), twiceMovedVariance(1, 2), 10e-7);

  EXPECT_NEAR(perigeeVariance(2, 0), twiceMovedVariance(2, 0), 10e-7);
  EXPECT_NEAR(perigeeVariance(2, 1), twiceMovedVariance(2, 1), 10e-7);
  EXPECT_NEAR(perigeeVariance(2, 2), twiceMovedVariance(2, 2), 10e-7);
}
