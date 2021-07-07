/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dataobjects/UncertainHelix.h>
#include <TVectorD.h>
#include <boost/range/irange.hpp>
#include <framework/utilities/TestHelpers.h>
#include <gtest/gtest.h>

using boost::irange;

using namespace std;
using namespace Belle2;

namespace {

  TEST(UncertainHelixTest, CartesianCovarianceRoundtripWithPerigeeOnOrigin)
  {
    TVector3 expectedMomentum(1.0, 0.0, 0.0);
    TVector3 expectedPosition(0.0, 0.0, 0.0);
    const double expectedCharge = -1;
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

    const TVector3 position = uncertainHelix.getPerigee();
    const TVector3 momentum = uncertainHelix.getMomentum(bZ);
    const double charge = uncertainHelix.getChargeSign();
    const TMatrixDSym cov6 = uncertainHelix.getCartesianCovariance(bZ);

    EXPECT_NEAR(expectedPosition.X(), position.X(), 1e-7);
    EXPECT_NEAR(expectedPosition.Y(), position.Y(), 1e-7);
    EXPECT_NEAR(expectedPosition.Z(), position.Z(), 1e-7);

    EXPECT_NEAR(expectedMomentum.X(), momentum.X(), 1e-7);
    EXPECT_NEAR(expectedMomentum.Y(), momentum.Y(), 1e-7);
    EXPECT_NEAR(expectedMomentum.Z(), momentum.Z(), 1e-7);

    EXPECT_EQ(expectedCharge, charge);

    for (int i : irange(0, 6)) {
      for (int j : irange(0, 6)) {
        EXPECT_NEAR(expectedCov6(i, j), cov6(i, j), 1e-7);
      }
    }
  }


  TEST(UncertainHelixTest, CartesianCovarianceRoundtripAtPerigeeBesidesOrigin)
  {
    TVector3 expectedMomentum(1.0, 0.0, 0.0);
    TVector3 expectedPosition(0.0, 1.0, 0.0);
    const double expectedCharge = -1;
    const double bZ = 2;
    const double pValue = 0.5;

    TMatrixDSym expectedCov6(6);
    expectedCov6.Zero();
    expectedCov6(0, 0) = 0; // There cannot be a covariance in the x direction since the direction along the track has no constraint.
    for (int i : irange(1, 6)) {
      for (int j : irange(1, 6)) {
        // Assign some values to the covariances to check the correct propagation of signs.
        expectedCov6(i, j) = 1;
      }
    }

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

    const TVector3 position = uncertainHelix.getPerigee();
    const TVector3 momentum = uncertainHelix.getMomentum(bZ);
    const double charge = uncertainHelix.getChargeSign();
    const TMatrixDSym cov6 = uncertainHelix.getCartesianCovariance(bZ);

    EXPECT_NEAR(expectedPosition.X(), position.X(), 1e-7);
    EXPECT_NEAR(expectedPosition.Y(), position.Y(), 1e-7);
    EXPECT_NEAR(expectedPosition.Z(), position.Z(), 1e-7);

    EXPECT_NEAR(expectedMomentum.X(), momentum.X(), 1e-7);
    EXPECT_NEAR(expectedMomentum.Y(), momentum.Y(), 1e-7);
    EXPECT_NEAR(expectedMomentum.Z(), momentum.Z(), 1e-7);

    EXPECT_EQ(expectedCharge, charge);

    // Test the covariance matrix for the right null space
    TVectorD nullSpace(6);
    const double factor =
      (expectedPosition.X() * expectedMomentum.Y() - expectedPosition.Y() * expectedMomentum.X()) / expectedMomentum.Perp2();
    nullSpace[0] = expectedMomentum.X();
    nullSpace[1] = expectedMomentum.Y();
    nullSpace[2] = 0;
    nullSpace[3] = factor * expectedMomentum.Y();
    nullSpace[4] = - factor * expectedMomentum.X();
    nullSpace[5] = 0;

    TVectorD nullVector = cov6 * nullSpace;
    for (int j : irange(0, 6)) {
      EXPECT_NEAR(0, nullVector(j), 1e-7);
    }

    // Make a second round trip to assert the covariance
    // is stable after one adjustment of the null space
    UncertainHelix uncertainHelix2(position,
                                   momentum,
                                   charge,
                                   bZ,
                                   cov6,
                                   pValue);

    const TMatrixDSym cov6_2 = uncertainHelix2.getCartesianCovariance(bZ);
    for (int i : irange(0, 6)) {
      for (int j : irange(0, 6)) {
        EXPECT_NEAR(cov6(i, j), cov6_2(i, j), 1e-7);
      }
    }
  }

  TEST(UncertainHelixTest, CartesianCovarianceRoundtripAtPerigeeBesidesOriginIndividualCoordinates)
  {
    TVector3 expectedMomentum(1.0, 0.0, 0.0);
    TVector3 expectedPosition(0.0, 1.0, 0.0);
    const double expectedCharge = -1;
    const double bZ = 2;
    const double pValue = 0.5;

    for (int i : irange(1, 6)) {

      TMatrixDSym expectedCov6(6);
      expectedCov6.Zero();
      expectedCov6(0, 0) = 0; // There cannot be a covariance in the x direction since the direction along the track has no constraint.

      expectedCov6(i, i) = 1;

      UncertainHelix uncertainHelix(expectedPosition,
                                    expectedMomentum,
                                    expectedCharge,
                                    bZ,
                                    expectedCov6,
                                    pValue);

      const TVector3 position = uncertainHelix.getPerigee();
      const TVector3 momentum = uncertainHelix.getMomentum(bZ);
      const double charge = uncertainHelix.getChargeSign();
      const TMatrixDSym cov6 = uncertainHelix.getCartesianCovariance(bZ);

      EXPECT_NEAR(expectedPosition.X(), position.X(), 1e-7);
      EXPECT_NEAR(expectedPosition.Y(), position.Y(), 1e-7);
      EXPECT_NEAR(expectedPosition.Z(), position.Z(), 1e-7);

      EXPECT_NEAR(expectedMomentum.X(), momentum.X(), 1e-7);
      EXPECT_NEAR(expectedMomentum.Y(), momentum.Y(), 1e-7);
      EXPECT_NEAR(expectedMomentum.Z(), momentum.Z(), 1e-7);

      EXPECT_EQ(expectedCharge, charge);

      // Test the covariance matrix for the right null space
      TVectorD nullSpace(6);
      const double factor =
        (expectedPosition.X() * expectedMomentum.Y() - expectedPosition.Y() * expectedMomentum.X()) / expectedMomentum.Perp2();
      nullSpace[0] = expectedMomentum.X();
      nullSpace[1] = expectedMomentum.Y();
      nullSpace[2] = 0;
      nullSpace[3] = factor * expectedMomentum.Y();
      nullSpace[4] = - factor * expectedMomentum.X();
      nullSpace[5] = 0;

      TVectorD nullVector = cov6 * nullSpace;
      for (int j : irange(0, 6)) {
        EXPECT_NEAR(0, nullVector(j), 1e-7);
      }

      // Make a second round trip to assert the covariance
      // is stable after one adjustment of the null space
      UncertainHelix uncertainHelix2(position,
                                     momentum,
                                     charge,
                                     bZ,
                                     cov6,
                                     pValue);

      const TMatrixDSym cov6_2 = uncertainHelix2.getCartesianCovariance(bZ);
      for (int j : irange(0, 6)) {
        for (int k : irange(0, 6)) {
          EXPECT_NEAR(cov6(j, k), cov6_2(j, k), 1e-7);
        }
      }
    }
  }

  TEST(UncertainHelixTest, PerigeeCovarianceRoundtripAtPerigeeBesidesOriginIndividualCoordinates)
  {
    double d0 = 1;
    double phi0 = 0;
    double omega = -0.005;
    double tanLambda = 1;
    double z0 = 0;

    const double bZ = 2;
    const double pValue = 0.5;

    for (int i : irange(0, 5)) {

      TMatrixDSym expectedCov5(5);
      expectedCov5.Zero();
      expectedCov5(i, i) = 1;

      UncertainHelix uncertainHelix(d0, phi0, omega, z0, tanLambda, expectedCov5, pValue);

      const TVector3 position = uncertainHelix.getPerigee();
      const TVector3 momentum = uncertainHelix.getMomentum(bZ);
      const double charge = uncertainHelix.getChargeSign();
      const TMatrixDSym cov6 = uncertainHelix.getCartesianCovariance(bZ);

      UncertainHelix uncertainHelix2(position,
                                     momentum,
                                     charge,
                                     bZ,
                                     cov6,
                                     pValue);

      EXPECT_NEAR(d0, uncertainHelix2.getD0(), 1e-7);
      EXPECT_NEAR(phi0, uncertainHelix2.getPhi0(), 1e-7);
      EXPECT_NEAR(omega, uncertainHelix2.getOmega(), 1e-7);

      EXPECT_NEAR(z0, uncertainHelix2.getZ0(), 1e-7);
      EXPECT_NEAR(tanLambda, uncertainHelix2.getTanLambda(), 1e-7);

      const TMatrixDSym& cov5 = uncertainHelix2.getCovariance();
      EXPECT_EQ(-1, charge);

      for (int j : irange(0, 5)) {
        for (int k : irange(0, 5)) {
          EXPECT_NEAR(expectedCov5(j, k), cov5(j, k), 1e-7);
        }
      }
    }
  }

  TEST(UncertainHelixTest, RealPerigeeCovarianceRoundTrip)
  {
    const double bZ = 1.5;
    const double pValue = 0.5;

    std::vector<float> helixParams(5);
    std::vector<float> helixCovariance(15);

    helixParams[0] = 3.82384;
    helixParams[1] = std::remainder(3.57559, 2 * M_PI);
    helixParams[2] = 0.00530726;
    helixParams[3] = -0.000317335;
    helixParams[4] = 1.34536;

    helixCovariance[0] = 0.000648818;
    helixCovariance[1] = 6.357e-05;
    helixCovariance[2] = 2.04344e-07;
    helixCovariance[3] = -0.00101879;
    helixCovariance[4] = 2.29407e-05;

    helixCovariance[5] = 1.02934e-05;
    helixCovariance[6] = 1.39609e-08;
    helixCovariance[7] = 9.57685e-07;
    helixCovariance[8] = 1.19863e-06;

    helixCovariance[9] = 4.96093e-10;
    helixCovariance[10] = -8.41612e-07;
    helixCovariance[11] = 6.50684e-08;

    helixCovariance[12] = 0.0317385;
    helixCovariance[13] = -0.00065476;

    helixCovariance[14] = 3.6521e-05;

    TMatrixDSym expectedCov5(5);
    unsigned int counter = 0;
    for (unsigned int i = 0; i < 5; ++i) {
      for (unsigned int j = i; j < 5; ++j) {
        expectedCov5(i, j) = expectedCov5(j, i) = helixCovariance[counter];
        ++counter;
      }
    }

    UncertainHelix uncertainHelix(helixParams[0],
                                  helixParams[1],
                                  helixParams[2],
                                  helixParams[3],
                                  helixParams[4],
                                  expectedCov5,
                                  pValue);

    const TVector3 position = uncertainHelix.getPerigee();
    const TVector3 momentum = uncertainHelix.getMomentum(bZ);
    const double charge = uncertainHelix.getChargeSign();
    const TMatrixDSym cov6 = uncertainHelix.getCartesianCovariance(bZ);

    UncertainHelix uncertainHelix2(position,
                                   momentum,
                                   charge,
                                   bZ,
                                   cov6,
                                   pValue);

    EXPECT_NEAR(helixParams[0], uncertainHelix2.getD0(), 1e-7);
    EXPECT_NEAR(helixParams[1], uncertainHelix2.getPhi0(), 1e-7);
    EXPECT_NEAR(helixParams[2], uncertainHelix2.getOmega(), 1e-7);

    EXPECT_NEAR(helixParams[3], uncertainHelix2.getZ0(), 1e-7);
    EXPECT_NEAR(helixParams[4], uncertainHelix2.getTanLambda(), 1e-7);

    const TMatrixDSym& cov5 = uncertainHelix2.getCovariance();
    EXPECT_EQ(1, charge);

    for (int i : boost::irange(0, 5)) {
      for (int j : boost::irange(0, 5)) {
        EXPECT_NEAR(expectedCov5(i, j), cov5(i, j), 1e-7);
      }
    }
  }


  TEST(UncertainHelixTest, Explicit_D0)
  {
    double d0 = 1;
    double phi0 = 0;
    double omega = -0.005;
    double tanLambda = 1;
    double z0 = 0;

    const double bZ = 2;
    const double pValue = 0.5;

    TMatrixDSym expectedCov5(5);
    expectedCov5.Zero();
    expectedCov5(0, 0) = 1;

    TMatrixDSym expectedCov6(6);
    expectedCov6.Zero();
    expectedCov6(1, 1) = 1;

    UncertainHelix uncertainHelix(d0, phi0, omega, z0, tanLambda, expectedCov5, pValue);

    const TMatrixDSym cov6 = uncertainHelix.getCartesianCovariance(bZ);

    for (int i : irange(0, 6)) {
      for (int j : irange(0, 6)) {
        TEST_CONTEXT("Failed for index (" << i << ", " << j << ")");
        EXPECT_NEAR(expectedCov6(i, j), cov6(i, j), 1e-7);
      }
    }
  }

  TEST(UncertainHelixTest, Explicit_Phi0)
  {
    double d0 = 0;
    double phi0 = 0;
    double omega = -0.006;
    double tanLambda = 0;
    double z0 = 0;

    const double bZ = 1.5;
    const double pValue = 0.5;

    TMatrixDSym expectedCov5(5);
    expectedCov5.Zero();
    expectedCov5(1, 1) = 1;

    UncertainHelix uncertainHelix(d0, phi0, omega, z0, tanLambda, expectedCov5, pValue);

    const TVector3 position = uncertainHelix.getPerigee();
    const TVector3 momentum = uncertainHelix.getMomentum(bZ);
    const double charge = uncertainHelix.getChargeSign();
    const TMatrixDSym cov6 = uncertainHelix.getCartesianCovariance(bZ);

    double absMom2D = uncertainHelix.getTransverseMomentum(1.5);

    TMatrixDSym expectedCov6(6);
    expectedCov6.Zero();
    expectedCov6(4, 4) = absMom2D * absMom2D;

    for (int i : irange(0, 6)) {
      for (int j : irange(0, 6)) {
        TEST_CONTEXT("Failed for index (" << i << ", " << j << ")");
        EXPECT_NEAR(expectedCov6(i, j), cov6(i, j), 1e-7);
      }
    }

    UncertainHelix uncertainHelix2(position,
                                   momentum,
                                   charge,
                                   bZ,
                                   cov6,
                                   pValue);

    EXPECT_NEAR(d0, uncertainHelix2.getD0(), 1e-7);
    EXPECT_NEAR(phi0, uncertainHelix2.getPhi0(), 1e-7);
    EXPECT_NEAR(omega, uncertainHelix2.getOmega(), 1e-7);

    EXPECT_NEAR(z0, uncertainHelix2.getZ0(), 1e-7);
    EXPECT_NEAR(tanLambda, uncertainHelix2.getTanLambda(), 1e-7);

    const TMatrixDSym& cov5 = uncertainHelix2.getCovariance();
    EXPECT_EQ(-1, charge);

    for (int i : irange(0, 5)) {
      for (int j : irange(0, 5)) {
        TEST_CONTEXT("Failed for index (" << i << ", " << j << ")");
        EXPECT_NEAR(expectedCov5(i, j), cov5(i, j), 1e-7);
      }
    }
  }

  TEST(UncertainHelixTest, MoveInCartesianVersusMoveInPerigeeCoordinates)
  {
    // Move of the coordinate system
    TVector3 moveBy(0.5, -1, 0.5);

    double d0 = 1;
    double phi0 = 0;
    double omega = -0.005;
    double tanLambda = 1;
    double z0 = 0;

    const double bZ = 2;
    const double pValue = 0.5;

    TMatrixDSym initialCov5(5);
    initialCov5.Zero();
    for (int i : irange(0, 5)) {
      for (int j : irange(0, 5)) {
        initialCov5(i, j) = i + j;
      }
    }

    UncertainHelix uncertainHelix(d0, phi0, omega, z0, tanLambda, initialCov5, pValue);

    const TVector3 position = uncertainHelix.getPerigee();
    const TVector3 momentum = uncertainHelix.getMomentum(bZ);
    const double charge = uncertainHelix.getChargeSign();
    const TMatrixDSym cov6 = uncertainHelix.getCartesianCovariance(bZ);

    // Execute the move in cartesian coordinates
    const TVector3 movedPosition = position - moveBy;
    UncertainHelix movedUncertainHelix(movedPosition,
                                       momentum,
                                       charge,
                                       bZ,
                                       cov6,
                                       pValue);

    const TMatrixDSym& cov5 = movedUncertainHelix.getCovariance();

    // Execute the move in perigee coordinates
    UncertainHelix expectedMovedUncertainHelix = uncertainHelix;
    expectedMovedUncertainHelix.passiveMoveBy(moveBy);
    const TMatrixDSym expectedCov5 = expectedMovedUncertainHelix.getCovariance();

    // Test equivalence of both transformation
    EXPECT_NEAR(expectedMovedUncertainHelix.getD0(), movedUncertainHelix.getD0(), 1e-7);
    EXPECT_NEAR(expectedMovedUncertainHelix.getPhi0(), movedUncertainHelix.getPhi0(), 1e-7);
    EXPECT_NEAR(expectedMovedUncertainHelix.getOmega(), movedUncertainHelix.getOmega(), 1e-7);
    EXPECT_NEAR(expectedMovedUncertainHelix.getZ0(), movedUncertainHelix.getZ0(), 1e-7);
    EXPECT_NEAR(expectedMovedUncertainHelix.getTanLambda(), movedUncertainHelix.getTanLambda(), 1e-7);

    for (int i : irange(0, 5)) {
      for (int j : irange(0, 5)) {
        TEST_CONTEXT("Failed for index (" << i << ", " << j << ")");
        EXPECT_NEAR(expectedCov5(i, j), cov5(i, j), 1e-7);
      }
    }
  }
}
