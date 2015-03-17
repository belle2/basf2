/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/dataobjects/UncertainHelix.h>
#include <boost/range/irange.hpp>

#include <gtest/gtest.h>

using boost::irange;

using namespace std;
using namespace Belle2;

TEST(UncertainHelixTest, CartesianCovarianceRoundtrip)
{
  TVector3 expectedMomentum(1.0, 0.0, 0.0);
  TVector3 expectedPosition(0.0, 0.0, 0.0);
  const double expectedCharge = 1;
  const double bZ = 2;
  const double pValue = 0.5;

  TMatrixDSym expectedCov6(6);
  expectedCov6.Zero();
  expectedCov6(0, 0) = 0; // There cannot be a covariance in the x direction since the direction along the track has no constrained.
  expectedCov6(1, 1) = 2;
  expectedCov6(2, 2) = 3;
  expectedCov6(3, 3) = 4;
  expectedCov6(4, 4) = 5;
  expectedCov6(5, 5) = 6;

  UncertainHelix uncertainHelix(expectedPosition,
                                expectedMomentum,
                                expectedCharge,
                                bZ,
                                expectedCov6,
                                pValue);

  const TMatrixDSym cov5 = uncertainHelix.getCovariance();

  const TVector3 position = uncertainHelix.getPerigee();
  const TVector3 momentum = uncertainHelix.getMomentum(bZ);
  const double charge = uncertainHelix.getChargeSign();
  const TMatrixDSym cov6 = uncertainHelix.getCartesianCovariance(bZ);

  EXPECT_NEAR(expectedPosition.X(), position.X(), 10e-7);
  EXPECT_NEAR(expectedPosition.Y(), position.Y(), 10e-7);
  EXPECT_NEAR(expectedPosition.Z(), position.Z(), 10e-7);

  EXPECT_NEAR(expectedMomentum.X(), momentum.X(), 10e-7);
  EXPECT_NEAR(expectedMomentum.Y(), momentum.Y(), 10e-7);
  EXPECT_NEAR(expectedMomentum.Z(), momentum.Z(), 10e-7);

  EXPECT_EQ(expectedCharge, charge);

  for (int i : irange(0, 6)) {
    for (int j : irange(0, 6)) {
      EXPECT_NEAR(expectedCov6(i, j), cov6(i, j), 10e-7);
    }
  }
}
