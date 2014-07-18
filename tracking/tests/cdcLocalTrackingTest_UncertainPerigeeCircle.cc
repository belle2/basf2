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

#include <tracking/cdcLocalTracking/geometry/Vector2D.h>
#include <tracking/cdcLocalTracking/geometry/UncertainPerigeeCircle.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;



TEST_F(CDCLocalTrackingTest, PerigeeCircle_passiveMoveByJacobian)
{
  UncertainPerigeeCircle circle(1.0, -PI / 2, 0);

  TMatrixD moveByOneJacobian = circle.passiveMoveByJacobian(Vector2D(-1.0, 0.0));
  EXPECT_NEAR(1.0, moveByOneJacobian(0, 0), 10e-7);
  EXPECT_NEAR(0.0, moveByOneJacobian(0, 1), 10e-7);
  EXPECT_NEAR(0.0, moveByOneJacobian(0, 2), 10e-7);

  EXPECT_NEAR(0.0, moveByOneJacobian(1, 0), 10e-7);
  EXPECT_NEAR(1.0 / 2.0, moveByOneJacobian(1, 1), 10e-7);
  EXPECT_NEAR(0.0, moveByOneJacobian(1, 2), 10e-7);

  EXPECT_NEAR(0.0, moveByOneJacobian(2, 0), 10e-7);
  EXPECT_NEAR(0.0, moveByOneJacobian(2, 1), 10e-7);
  EXPECT_NEAR(1.0, moveByOneJacobian(2, 2), 10e-7);

  TMatrixD moveByTwoYJacobian = circle.passiveMoveByJacobian(Vector2D(0.0, -2.0));

  // Hand caluclated intermediate quantities;
  FloatType deltaParallel = 2;
  FloatType A = 4;
  FloatType u = 1;

  FloatType nu = 1;
  //FloatType xi = 1.0 / 5.0;
  FloatType lambda = 1.0 / (5.0 + 3.0 * sqrt(5.0));
  FloatType mu = sqrt(5.0) / 10.0;
  FloatType zeta = 4;

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





TEST_F(CDCLocalTrackingTest, PerigeeCircle_passiveMovedCovarianceBy)
{
  TMatrixDSym perigeeVariance(3, 3);
  perigeeVariance(0, 0) = 1.0;
  perigeeVariance(0, 1) = 0.0;
  perigeeVariance(0, 2) = 0.0;

  perigeeVariance(1, 0) = 0.0;
  perigeeVariance(1, 1) = 0.0;
  perigeeVariance(1, 2) = 0.0;

  perigeeVariance(2, 0) = 0.0;
  perigeeVariance(2, 1) = 0.0;
  perigeeVariance(2, 2) = 0.0;

  UncertainPerigeeCircle circle(1.0, -PI / 2, 0.0, perigeeVariance);

  {
    TMatrixDSym noMoveVariance = circle.passiveMovedCovarianceBy(Vector2D(0.0, 0.0));
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
    TMatrixDSym noChangeMoveVariance = circle.passiveMovedCovarianceBy(Vector2D(-1.0, 0.0));
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
    TMatrixDSym transformedVariance = circle.passiveMovedCovarianceBy(Vector2D(2.0, 0.0));

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
    TMatrixDSym transformedVariance = circle.passiveMovedCovarianceBy(Vector2D(2.5, 0.0));

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



TEST_F(CDCLocalTrackingTest, PerigeeCircle_passiveMove)
{

  TMatrixDSym perigeeVariance(3, 3);
  perigeeVariance(0, 0) = 1.0;
  perigeeVariance(0, 1) = 0.3;
  perigeeVariance(0, 2) = 0.5;

  perigeeVariance(1, 0) = 0.3;
  perigeeVariance(1, 1) = 0.7;
  perigeeVariance(1, 2) = 0.6;

  perigeeVariance(2, 0) = 0.5;
  perigeeVariance(2, 1) = 0.6;
  perigeeVariance(2, 2) = 1;

  UncertainPerigeeCircle circle(1.0, -PI / 4, 1.0, perigeeVariance);

  //circle.perigeeCovariance().Print();

  //Test if the move commutes
  circle.passiveMoveBy(Vector2D(0.0, -1.0));

  //circle.perigeeCovariance().Print();

  circle.passiveMoveBy(Vector2D(0.0, 1.0));

  //circle.perigeeCovariance().Print();

  TMatrixDSym twiceMovedVariance  = circle.perigeeCovariance();

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

