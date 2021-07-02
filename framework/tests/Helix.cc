/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/dataobjects/Helix.h>

#include <TVector3.h>
#include <TRandom3.h>

#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace HelixParameterIndex;

/// String output operator for a TVector3 for gtest print support.
std::ostream& operator<<(::std::ostream& output, const TVector3& tVector3)
{
  return output
         << "TVector3("
         << tVector3.X() << ", "
         << tVector3.Y() << ", "
         << tVector3.Z() << ")";
}

namespace Belle2::TestHelpers {

  /** Predicate checking that all five components of the Helix are close by a
   * maximal error of absError. Extends the EXPECT_ALL_NEAR/ASSERT_ALL_NEAR
   * macro in the framework testhelpers to work for the helix class
   */
  template<>
  bool allNear<Belle2::Helix>(const Belle2::Helix& expected,
                              const Belle2::Helix& actual,
                              double tolerance)
  {
    bool d0Near = fabs(expected.getD0() - actual.getD0()) < tolerance;
    bool phi0Near = angleNear(expected.getPhi0(), actual.getPhi0(), tolerance);
    bool omegaNear = fabs(expected.getOmega() - actual.getOmega()) < tolerance;
    bool z0Near = fabs(expected.getZ0() - actual.getZ0()) < tolerance;
    bool tanLambdaNear = fabs(expected.getTanLambda() - actual.getTanLambda()) < tolerance;

    return d0Near and phi0Near and omegaNear and z0Near and tanLambdaNear;
  }
}

namespace {
  Belle2::Helix helixFromCenter(const TVector3& center, const float& radius, const float& tanLambda)
  {
    double omega = 1 / radius;
    double phi0 = center.Phi() + copysign(M_PI / 2.0, radius);
    double d0 = copysign(center.Perp(), radius) - radius;
    double z0 = center.Z();

    return Belle2::Helix(d0, phi0, omega, z0, tanLambda);
  }

  /** Returns n evenly spaced samples, calculated over the closed interval [start, stop ].*/
  vector<float> linspace(const float& start, const float& end, const int n)
  {
    std::vector<float> result(n);
    result[0] = start;
    result[n - 1] = end;

    for (int i = 1; i < n - 1; ++i) {
      float start_weight = (float)(n - 1 - i) / (n - 1);
      float end_weight = 1 - start_weight;
      result[i] = start * start_weight + end * end_weight;
    }

    return result;
  }

  /** Set up a few arrays and objects in the datastore */
  class HelixTest : public ::testing::Test {

  protected:
    // Common level precision for all tests.
    double absError = 1e-6;
    double nominalBz = 1.5;

    std::vector<float> omegas { -1, 0, 1};
    //std::vector<float> omegas {1};
    std::vector<float> phi0s = linspace(-M_PI, M_PI, 11);
    std::vector<float> d0s { -0.5, -0.2, 0, 0.2, 0.5};
    //std::vector<float> d0s {0.5};
    std::vector<float> chis = linspace(-5 * M_PI / 6, 5 * M_PI / 6, 11);

    std::vector<TVector3> bys = {
      TVector3(-2.0, 0.5, 0.0),
      TVector3(-1.0, 0.5, 0.0),
      TVector3(0.0, 0.5, 0.0),
      TVector3(1.0, 0.5, 0.0),
      TVector3(2.0, 0.5, 0.0),

      TVector3(-2.0, 0.0, 0.0),
      TVector3(-1.0, 0.0, 0.0),
      TVector3(0.0, 0.0, 0.0),
      TVector3(1.0, 0.0, 0.0),
      TVector3(2.0, 0.0, 0.0),

      TVector3(-2.0, -1.0, 0.0),
      TVector3(-1.0, -1.0, 0.0),
      TVector3(0.0, -1.0, 0.0),
      TVector3(1.0, -1.0, 0.0),
      TVector3(2.0, -1.0, 0.0),
    };

  };

  /** Test simple Setters and Getters. */
  TEST_F(HelixTest, Getters)
  {
    TRandom3 generator;
    unsigned int nCases = 1;
    double bField = nominalBz;

    for (unsigned int i = 0; i < nCases; ++i) {

      short int charge = generator.Uniform(-1, 1) > 0 ? 1 : -1;

      // Generate a random put orthogonal pair of vectors in the r-phi plane
      TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      d.Set(d.X(), -d.X() * pt.Px() / pt.Py());

      // Add a random z component
      TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
      TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));

      // Set up class for testing
      Helix helix(position, momentum, charge, bField);

      // Test all vector elements
      EXPECT_ALL_NEAR(position, helix.getPerigee(), absError);
      EXPECT_ALL_NEAR(momentum, helix.getMomentum(bField), absError);

      // Test getter for transverse momentum
      EXPECT_NEAR(momentum.Perp(), helix.getTransverseMomentum(bField), absError);

      // Test getter of kappa
      EXPECT_NEAR(charge / momentum.Perp(), helix.getKappa(bField), absError);

      // Test getter of d0
      // Check absolute value of d0
      EXPECT_NEAR(position.Perp(), fabs(helix.getD0()), absError);

      // Check sign of d0
      EXPECT_SAME_SIGN(position.Cross(momentum).Z(), helix.getD0());

      // Test getter of phi0
      EXPECT_NEAR(momentum.Phi(), helix.getPhi0(), absError);

      // Test getter of d0
      EXPECT_NEAR(position.Z(), helix.getZ0(), absError);

      // Test getter of tan lambda
      EXPECT_NEAR(1 / tan(momentum.Theta()), helix.getTanLambda(), absError);

      // Test getter of cot theta
      EXPECT_NEAR(1 / tan(momentum.Theta()), helix.getCotTheta(), absError);

      // Test other variables
      EXPECT_EQ(charge, helix.getChargeSign());

    }
  } // Testcases for getters

  TEST_F(HelixTest, SignOfD0)
  {
    // This tests the assumption that the sign of d0 is given by the sign of position x momentum as a two dimensional cross product.

    const TVector3 position(1, 0, 0);
    const TVector3 momentum(0, 1, 0);
    const TVector3 oppositeMomentum(0, -1, 0);
    const float charge = 1;
    const float bField = nominalBz;

    Helix helix(position, momentum, charge, bField);
    EXPECT_NEAR(1, helix.getD0(), absError);

    // D0 does not change with the charge
    Helix helix2(position, momentum, -charge, bField);
    EXPECT_NEAR(1, helix2.getD0(), absError);

    // But with reversal of momentum
    Helix oppositeMomentumHelix(position, oppositeMomentum, charge, bField);
    EXPECT_NEAR(-1, oppositeMomentumHelix.getD0(), absError);

    Helix oppositeMomentumHelix2(position, oppositeMomentum, -charge, bField);
    EXPECT_NEAR(-1, oppositeMomentumHelix2.getD0(), absError);

  }

  TEST_F(HelixTest, Center)
  {
    /** Setup a helix
     *  for counterclockwise travel
     *  starting at -1.0, 0.0, 0.0
     *  heading in the negative y direction initially
     */

    TVector3 center(0.0, -2.0, 0.0);
    float radius = -1;
    // Keep it flat
    float tanLambda = 0;

    Helix helix = helixFromCenter(center, radius, tanLambda);

    TVector3 actualCenter = helix.getPerigee() * ((1 / helix.getOmega() + helix.getD0()) / helix.getD0());
    EXPECT_NEAR(center.X(), actualCenter.X(), absError);
    EXPECT_NEAR(center.Y(), actualCenter.Y(), absError);
  }

  TEST_F(HelixTest, Explicit)
  {
    /** Setup a helix
     *  for counterclockwise travel
     *  starting at -1.0, 0.0, 0.0
     *  heading in the negative y direction initially
     */

    TVector3 center(0.0, -2.0, 0.0);
    float radius = -1;
    // Keep it flat
    float tanLambda = 0;

    Helix helix = helixFromCenter(center, radius, tanLambda);
    EXPECT_NEAR(-1, helix.getD0(), absError);
    EXPECT_ANGLE_NEAR(-M_PI, helix.getPhi0(), absError);
    EXPECT_NEAR(-1, helix.getOmega(), absError);

    // Positions on the helix
    {
      // Start point
      float arcLength2D = 0;
      TVector3 position = helix.getPositionAtArcLength2D(arcLength2D);
      TVector3 tangential = helix.getUnitTangentialAtArcLength2D(arcLength2D);

      EXPECT_ALL_NEAR(TVector3(0.0, -1.0, 0.0), position, absError);
      EXPECT_ANGLE_NEAR(-M_PI, tangential.Phi(), absError);
    }

    {
      float arcLength2D = M_PI / 2;
      TVector3 position = helix.getPositionAtArcLength2D(arcLength2D);
      TVector3 tangential = helix.getUnitTangentialAtArcLength2D(arcLength2D);
      EXPECT_ALL_NEAR(TVector3(-1.0, -2.0, 0.0), position, absError);
      EXPECT_ANGLE_NEAR(-M_PI / 2, tangential.Phi(), absError);
    }

    {
      float arcLength2D = M_PI;
      TVector3 position = helix.getPositionAtArcLength2D(arcLength2D);
      TVector3 tangential = helix.getUnitTangentialAtArcLength2D(arcLength2D);
      EXPECT_ALL_NEAR(TVector3(0.0, -3.0, 0.0), position, absError);
      EXPECT_ANGLE_NEAR(0, tangential.Phi(), absError);
    }

    {
      float arcLength2D = 3 * M_PI / 2 ;
      TVector3 position = helix.getPositionAtArcLength2D(arcLength2D);
      TVector3 tangential = helix.getUnitTangentialAtArcLength2D(arcLength2D);
      EXPECT_ALL_NEAR(TVector3(1.0, -2.0, 0.0), position, absError);
      EXPECT_ANGLE_NEAR(M_PI / 2, tangential.Phi(), absError);
    }
  }

  TEST_F(HelixTest, Tangential)
  {
    float z0 = 0;
    float tanLambda = 2;

    for (const float d0 : d0s) {
      for (const float phi0 : phi0s) {
        for (const float omega : omegas) {

          Helix helix(d0, phi0, omega, z0, tanLambda);
          TEST_CONTEXT("Failed for " << helix);

          TVector3 tangentialAtPerigee = helix.getUnitTangentialAtArcLength2D(0.0);

          EXPECT_ANGLE_NEAR(phi0, tangentialAtPerigee.Phi(), absError);
          EXPECT_FLOAT_EQ(1.0, tangentialAtPerigee.Mag());
          EXPECT_FLOAT_EQ(tanLambda, 1 / tan(tangentialAtPerigee.Theta()));

          for (const float chi : chis) {

            if (omega == 0) {
              // Use chi as the arc length in the straight line case
              float arcLength2D = chi;

              // Tangential vector shall not change along the line
              TVector3 tangential = helix.getUnitTangentialAtArcLength2D(arcLength2D);
              EXPECT_ALL_NEAR(tangentialAtPerigee, tangential, absError);

            } else {
              float arcLength2D = -chi / omega;
              TVector3 tangential = helix.getUnitTangentialAtArcLength2D(arcLength2D);

              float actualChi = tangential.DeltaPhi(tangentialAtPerigee);
              EXPECT_ANGLE_NEAR(chi, actualChi, absError);
              EXPECT_FLOAT_EQ(tangentialAtPerigee.Theta(), tangential.Theta());
              EXPECT_FLOAT_EQ(1, tangential.Mag());

            }

          }
        }
      }
    }
  }


  TEST_F(HelixTest, MomentumExtrapolation)
  {
    float z0 = 0;
    float tanLambda = -2;

    for (const float d0 : d0s) {
      for (const float phi0 : phi0s) {
        for (const float omega : omegas) {
          if (omega != 0) {

            Helix helix(d0, phi0, omega, z0, tanLambda);
            TVector3 momentumAtPerigee = helix.getMomentum(nominalBz);
            for (const float chi : chis) {

              float arcLength2D = -chi / omega;
              TVector3 extrapolatedMomentum = helix.getMomentumAtArcLength2D(arcLength2D, nominalBz);

              float actualChi = extrapolatedMomentum.DeltaPhi(momentumAtPerigee);
              EXPECT_ANGLE_NEAR(chi, actualChi, absError);
              EXPECT_FLOAT_EQ(momentumAtPerigee.Theta(), extrapolatedMomentum.Theta());
              EXPECT_FLOAT_EQ(momentumAtPerigee.Mag(), extrapolatedMomentum.Mag());
            }
          }
        }
      }
    }
  }



  TEST_F(HelixTest, Extrapolation)
  {

    // z coordinates do not matter for this test.
    float z0 = 0;
    float tanLambda = 2;

    for (const float d0 : d0s) {
      for (const float phi0 : phi0s) {
        for (const float omega : omegas) {

          Helix helix(d0, phi0, omega, z0, tanLambda);
          TVector3 perigee = helix.getPerigee();

          TVector3 tangentialAtPerigee = helix.getUnitTangentialAtArcLength2D(0.0);
          TEST_CONTEXT("Failed for " << helix);

          //continue;

          for (const float chi : chis) {
            TEST_CONTEXT("Failed for chi = " << chi);

            // In the cases where omega is 0 (straight line case) chi become undefined.
            // Use chi sample as transverse travel distance instead.
            float expectedArcLength2D = omega != 0 ? -chi / omega : chi;
            TVector3 pointOnHelix = helix.getPositionAtArcLength2D(expectedArcLength2D);

            float cylindricalR = pointOnHelix.Perp();
            float arcLength2D = helix.getArcLength2DAtCylindricalR(cylindricalR);

            // Only the absolute value is returned.
            EXPECT_NEAR(fabs(expectedArcLength2D), arcLength2D, absError);

            // Also check it the extrapolation lies in the forward direction.
            TVector3 secantVector = pointOnHelix - perigee;

            if (expectedArcLength2D == 0) {
              EXPECT_NEAR(0, secantVector.Mag(), absError);
            } else {
              TVector2 secantVectorXY = secantVector.XYvector();

              TVector2 tangentialXY = tangentialAtPerigee.XYvector();
              float coalignment = secantVectorXY * tangentialXY ;

              bool extrapolationIsForward = coalignment > 0;
              bool expectedIsForward = expectedArcLength2D > 0;
              EXPECT_EQ(expectedIsForward, extrapolationIsForward);
            }
          }
        }
      }
    }
  } // end TEST_F


  TEST_F(HelixTest, ExtrapolationToNormalPlane)
  {
    {
      TVector3 center(0.0, 2.0, 0.0);
      float radius = -1;
      // Keep it flat
      float tanLambda = 0;
      Helix helix = helixFromCenter(center, radius, tanLambda);

      // Plane coordinate
      double x = 0.0;
      double y = 3.0;
      double nx = -1.0;
      double ny = 1.0;
      double arcLength2D = helix.getArcLength2DAtNormalPlane(x, y, nx, ny);
      EXPECT_NEAR(-M_PI / 2, arcLength2D, absError);
    }

    {
      TVector3 center(0.0, 2.0, 0.0);
      float radius = -1;
      // Keep it flat
      float tanLambda = 0;
      Helix helix = helixFromCenter(center, radius, tanLambda);

      // Plane coordinate
      double x = 0.0;
      double y = 3.0;
      double nx = 1.0;
      double ny = 1.0;
      double arcLength2D = helix.getArcLength2DAtNormalPlane(x, y, nx, ny);
      EXPECT_NEAR(M_PI / 2, arcLength2D, absError);
    }

    {
      TVector3 center(0.0, 2.0, 0.0);
      float radius = 1;
      // Keep it flat
      float tanLambda = 0;
      Helix helix = helixFromCenter(center, radius, tanLambda);

      // Plane coordinate
      double x = 0.0;
      double y = 3.0;
      double nx = -1.0;
      double ny = 1.0;
      double arcLength2D = helix.getArcLength2DAtNormalPlane(x, y, nx, ny);
      EXPECT_NEAR(M_PI / 2, arcLength2D, absError);
    }

    {
      TVector3 center(0.0, 2.0, 0.0);
      float radius = 1;
      // Keep it flat
      float tanLambda = 0;
      Helix helix = helixFromCenter(center, radius, tanLambda);

      // Plane coordinate
      double x = 0.0;
      double y = 3.0;
      double nx = 1.0;
      double ny = 1.0;
      double arcLength2D = helix.getArcLength2DAtNormalPlane(x, y, nx, ny);
      EXPECT_NEAR(-M_PI / 2, arcLength2D, absError);
    }



  }

  TEST_F(HelixTest, PerigeeExtrapolateRoundTrip)
  {
    float z0 = 0;
    float tanLambda = -2;

    for (const float d0 : d0s) {
      for (const float phi0 : phi0s) {
        for (const float omega : omegas) {

          // Extrapolations involving the momentum only makes sense with finit momenta
          if (omega != 0) {
            Helix expectedHelix(d0, phi0, omega, z0, tanLambda);

            for (const float chi : chis) {
              float arcLength2D = -chi / omega;
              TVector3 position = expectedHelix.getPositionAtArcLength2D(arcLength2D);
              TVector3 momentum = expectedHelix.getMomentumAtArcLength2D(arcLength2D, nominalBz);
              int chargeSign = expectedHelix.getChargeSign();

              EXPECT_NEAR(tanLambda, 1 / tan(momentum.Theta()), absError);
              EXPECT_ANGLE_NEAR(phi0 + chi, momentum.Phi(), absError);
              EXPECT_NEAR(z0 + tanLambda * arcLength2D, position.Z(), absError);

              //B2INFO("chi out " << chi);
              Helix helix(position, momentum, chargeSign, nominalBz);

              EXPECT_NEAR(expectedHelix.getOmega(), helix.getOmega(), absError);
              EXPECT_ANGLE_NEAR(expectedHelix.getPhi0(), helix.getPhi0(), absError);
              EXPECT_NEAR(expectedHelix.getD0(), helix.getD0(), absError);
              EXPECT_NEAR(expectedHelix.getTanLambda(), helix.getTanLambda(), absError);
              EXPECT_NEAR(expectedHelix.getZ0(), helix.getZ0(), absError);
            }
          }

        }
      }
    }
  }




  /*
  TEST_F(HelixTest, CalculateDrExplicit)
  {
    float tanLambda = 3;

    TVector3 center(0.0, -2.0, 0.0);
    float radius = -1;

    Helix helix = helixFromCenter(center, radius, tanLambda);
    EXPECT_NEAR(-1, helix.getD0(), absError);
    EXPECT_ANGLE_NEAR(-M_PI, helix.getPhi0(), absError);
    EXPECT_NEAR(-1, helix.getOmega(), absError);
    {
      TVector3 position(0.0, 0.0, 0.0);
      float newD0 = helix.getDr(position);
      EXPECT_NEAR(-1, newD0, absError);
    }

    {
      TVector3 position(2.0, -2.0, 0.0);
      float newD0 = helix.getDr(position);
      EXPECT_NEAR(-1, newD0, absError);
    }
    {
      TVector3 position(-2.0, -2.0, 0.0);
      float newD0 = helix.getDr(position);
      EXPECT_NEAR(-1, newD0, absError);
    }

    {
      TVector3 position(1.0, -1.0, 0.0);
      float newD0 = helix.getDr(position);
      EXPECT_NEAR(-(sqrt(2) - 1) , newD0, absError);
    }
  }

  TEST_F(HelixTest, CalculateDr)
  {
    float z0 = 2;
    float tanLambda = 3;

    for (const float phi0 : phi0s) {
      for (const float omega : omegas) {
        for (const float d0 : d0s) {
          Helix helix(d0, phi0, omega, z0, tanLambda);
          TEST_CONTEXT("Failed for " << helix);

          EXPECT_NEAR(d0, helix.getDr(TVector3(0.0, 0.0, 0.0)), absError);

          for (const float chi : chis) {
            for (const float newD0 : d0s) {
              // In the line case use the chi value directly as the arc length

              float arcLength2D = omega == 0 ? chi : -chi / omega;
              TVector3 positionOnHelix = helix.getPositionAtArcLength2D(arcLength2D);

              TVector3 tangentialToHelix = helix.getUnitTangentialAtArcLength2D(arcLength2D);

              TVector3 perpendicularToHelix = tangentialToHelix;
              perpendicularToHelix.RotateZ(M_PI / 2.0);
              // Normalize the xy part
              perpendicularToHelix *= 1 / perpendicularToHelix.Perp();

              TVector3 displacementFromHelix = perpendicularToHelix * newD0;
              TVector3 testPosition = positionOnHelix + displacementFromHelix;

              TEST_CONTEXT("Failed for chi " << chi);
              TEST_CONTEXT("Failed for position on helix " << positionOnHelix);
              TEST_CONTEXT("Failed for tangential to helix " << tangentialToHelix);
              TEST_CONTEXT("Failed for perpendicular to helix " << perpendicularToHelix);
              TEST_CONTEXT("Failed for test position " << testPosition);

              float testDr = helix.getDr(testPosition);
              EXPECT_NEAR(newD0, testDr, absError);
            }
          }
        }
      }
    }
  }
  */

  TEST_F(HelixTest, PassiveMoveExplicit)
  {
    TVector3 center(0.0, 1.0, 0.0);
    float radius = -1;
    float tanLambda = 3;

    Helix helix = helixFromCenter(center, radius, tanLambda);

    ASSERT_NEAR(0, helix.getD0(), absError);
    ASSERT_ANGLE_NEAR(0, helix.getPhi0(), absError);
    ASSERT_NEAR(-1, helix.getOmega(), absError);

    // Save the untransformed Helix
    Helix expectedHelix(helix);

    // Vector by which the coordinate system should move.
    // (To the right of the circle)
    TVector3 by(1.0, 1.0, 0.0);

    float arcLength2D = helix.passiveMoveBy(by);

    // The right of the circle lies in the counterclockwise direction
    // The forward direction is counterclockwise, so we expect to move forward.
    ASSERT_NEAR(M_PI / 2, arcLength2D, absError);

    ASSERT_NEAR(0, helix.getD0(), absError);
    ASSERT_ANGLE_NEAR(M_PI / 2, helix.getPhi0(), absError);
    ASSERT_NEAR(-1, helix.getOmega(), absError);

    ASSERT_NEAR(3 * M_PI / 2, helix.getZ0(), absError);
    ASSERT_NEAR(3, helix.getTanLambda(), absError);

    // Now transform back to the original point
    float arcLength2DBackward = helix.passiveMoveBy(-by);

    ASSERT_NEAR(arcLength2D, -arcLength2DBackward, absError);
    ASSERT_ALL_NEAR(expectedHelix, helix, absError);
  }


  TEST_F(HelixTest, PassiveMove)
  {
    float z0 = 2;
    float tanLambda = 2;

    for (const float phi0 : phi0s) {
      for (const float omega : omegas) {
        for (const float d0 : d0s) {
          for (const float chi : chis) {
            for (const float newD0 : d0s) {
              Helix helix(d0, phi0, omega, z0, tanLambda);
              TEST_CONTEXT("Failed for " << helix);

              // In the line case use the chi value directly as the arc length

              float expectedArcLength2D = omega == 0 ? chi : -chi / omega;
              TVector3 positionOnHelix = helix.getPositionAtArcLength2D(expectedArcLength2D);
              TVector3 tangentialToHelix = helix.getUnitTangentialAtArcLength2D(expectedArcLength2D);

              TVector3 perpendicularToHelix = tangentialToHelix;
              perpendicularToHelix.RotateZ(M_PI / 2.0);
              // Normalize the xy part
              perpendicularToHelix *= 1 / perpendicularToHelix.Perp();

              TVector3 displacementFromHelix = -perpendicularToHelix * newD0;
              TVector3 testPosition = positionOnHelix + displacementFromHelix;

              TVector3 expectedPerigee = -displacementFromHelix;

              TEST_CONTEXT("Failed for chi " << chi);
              TEST_CONTEXT("Failed for position on helix " << positionOnHelix);
              TEST_CONTEXT("Failed for tangential to helix " << tangentialToHelix);
              TEST_CONTEXT("Failed for perpendicular to helix " << perpendicularToHelix);
              TEST_CONTEXT("Failed for test position " << testPosition);

              float arcLength2D = helix.passiveMoveBy(testPosition);

              ASSERT_NEAR(expectedArcLength2D, arcLength2D, absError);

              ASSERT_ALL_NEAR(expectedPerigee, helix.getPerigee(), absError);

            }
          }
        }
      }
    }
  }

  TEST_F(HelixTest, calcPassiveMoveByJacobian_identity)
  {
    TVector3 center(0.0, 1.0, 0.0);
    float radius = -1;
    float tanLambda = 3;

    Helix helix = helixFromCenter(center, radius, tanLambda);

    TMatrixD noMoveJacobian = helix.calcPassiveMoveByJacobian(TVector3(0.0, 0.0, 0.0));

    EXPECT_NEAR(1.0, noMoveJacobian(0, 0), 1e-7);
    EXPECT_NEAR(0.0, noMoveJacobian(0, 1), 1e-7);
    EXPECT_NEAR(0.0, noMoveJacobian(0, 2), 1e-7);

    EXPECT_NEAR(0.0, noMoveJacobian(1, 0), 1e-7);
    EXPECT_NEAR(1.0, noMoveJacobian(1, 1), 1e-7);
    EXPECT_NEAR(0.0, noMoveJacobian(1, 2), 1e-7);

    EXPECT_NEAR(0.0, noMoveJacobian(2, 0), 1e-7);
    EXPECT_NEAR(0.0, noMoveJacobian(2, 1), 1e-7);
    EXPECT_NEAR(1.0, noMoveJacobian(2, 2), 1e-7);
  }

  TEST_F(HelixTest, calcPassiveMoveByJacobian_orthogonalToPhi0)
  {
    TVector3 center(0.0, 1.0, 0.0);
    float radius = -1;
    float tanLambda = 3;

    Helix helix = helixFromCenter(center, radius, tanLambda);

    TMatrixD moveByOneJacobian = helix.calcPassiveMoveByJacobian(TVector3(0.0, -1.0, 0.0));

    EXPECT_NEAR(1.0, moveByOneJacobian(iD0, iD0), 1e-7);
    EXPECT_NEAR(0.0, moveByOneJacobian(iD0, iPhi0), 1e-7);
    EXPECT_NEAR(0.0, moveByOneJacobian(iD0, iOmega), 1e-7);

    EXPECT_NEAR(0.0, moveByOneJacobian(iPhi0, iD0), 1e-7);
    EXPECT_NEAR(1.0 / 2.0, moveByOneJacobian(iPhi0, iPhi0), 1e-7);
    EXPECT_NEAR(0.0, moveByOneJacobian(iPhi0, iOmega), 1e-7);

    EXPECT_NEAR(0.0, moveByOneJacobian(iOmega, iD0), 1e-7);
    EXPECT_NEAR(0.0, moveByOneJacobian(iOmega, iPhi0), 1e-7);
    EXPECT_NEAR(1.0, moveByOneJacobian(iOmega, iOmega), 1e-7);
  }


  TEST_F(HelixTest, calcPassiveMoveByJacobian_parallelToPhi0_compare_opposite_transformation)
  {
    TVector3 center(0.0, 1.0, 0.0);
    float radius = -1;
    float tanLambda = 3;

    Helix helix = helixFromCenter(center, radius, tanLambda);

    TMatrixD moveByPlusTwoXJacobian = helix.calcPassiveMoveByJacobian(TVector3(2.0, 0.0, 0.0));
    TMatrixD moveByMinusTwoXJacobian = helix.calcPassiveMoveByJacobian(TVector3(-2.0, 0.0, 0.0));

    EXPECT_NEAR(1.0, moveByPlusTwoXJacobian(iOmega, iOmega), 1e-7);
    EXPECT_NEAR(0.0, moveByPlusTwoXJacobian(iOmega, iPhi0), 1e-7);
    EXPECT_NEAR(0.0, moveByPlusTwoXJacobian(iOmega, iD0), 1e-7);

    EXPECT_NEAR(1.0, moveByMinusTwoXJacobian(iOmega, iOmega), 1e-7);
    EXPECT_NEAR(0.0, moveByMinusTwoXJacobian(iOmega, iPhi0), 1e-7);
    EXPECT_NEAR(0.0, moveByMinusTwoXJacobian(iOmega, iD0), 1e-7);

    // Symmetric effects
    EXPECT_NEAR(moveByMinusTwoXJacobian(iD0, iOmega), moveByPlusTwoXJacobian(iD0, iOmega), 1e-7);
    EXPECT_NEAR(moveByMinusTwoXJacobian(iPhi0, iPhi0), moveByPlusTwoXJacobian(iPhi0, iPhi0), 1e-7);
    EXPECT_NEAR(moveByMinusTwoXJacobian(iD0, iD0), moveByPlusTwoXJacobian(iD0, iD0), 1e-7);

    // Asymmetric effects
    EXPECT_NEAR(moveByMinusTwoXJacobian(iD0, iPhi0), -moveByPlusTwoXJacobian(iD0, iPhi0), 1e-7);
    EXPECT_NEAR(moveByMinusTwoXJacobian(iPhi0, iD0), -moveByPlusTwoXJacobian(iPhi0, iD0), 1e-7);
    EXPECT_NEAR(moveByMinusTwoXJacobian(iPhi0, iOmega), -moveByPlusTwoXJacobian(iPhi0, iOmega), 1e-7);

    // Signs
    EXPECT_NEGATIVE(moveByPlusTwoXJacobian(iD0, iPhi0));
    EXPECT_POSITIVE(moveByPlusTwoXJacobian(iD0, iOmega));
    EXPECT_POSITIVE(moveByPlusTwoXJacobian(iPhi0, iD0));
    EXPECT_NEGATIVE(moveByPlusTwoXJacobian(iPhi0, iOmega));

    EXPECT_NEGATIVE(moveByPlusTwoXJacobian(iD0, iD0) - 1);
    EXPECT_NEGATIVE(moveByPlusTwoXJacobian(iPhi0, iPhi0) - 1);
  }

  TEST_F(HelixTest, calcPassiveMoveByJacobian_roundtrip)
  {
    for (const TVector3& by : bys) {
      TVector3 center(0.0, 1.0, 0.0);
      float radius = -1;
      float tanLambda = 3;

      Helix helix = helixFromCenter(center, radius, tanLambda);

      TMatrixD jacobian = helix.calcPassiveMoveByJacobian(by);
      helix.passiveMoveBy(by);

      TMatrixD reverseJacobian = helix.calcPassiveMoveByJacobian(-by);

      TMatrixD unitMatrix = reverseJacobian * jacobian;

      for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
          if (i == j) {
            // Diagonal is one.
            EXPECT_NEAR(1, unitMatrix(i, j), 1e-7);
          } else {
            // Off diagonal is zero.
            EXPECT_NEAR(0, unitMatrix(i, j), 1e-7);
          }
        }
      }
    }

  }


  TEST_F(HelixTest, calcPassiveMoveByJacobian_parallelToPhi0)
  {
    TVector3 center(1.0, 0.0, 0.0);
    float radius = -1;
    float tanLambda = 3;

    Helix helix = helixFromCenter(center, radius, tanLambda);

    TMatrixD moveByTwoYJacobian = helix.calcPassiveMoveByJacobian(TVector3(0.0, 2.0, 0.0));

    // Hand caluclated intermediate quantities;
    double deltaParallel = 2;
    double A = 4;
    double u = 1;

    double nu = 1;
    //double xi = 1.0 / 5.0;
    double lambda = 1.0 / (5.0 + 3.0 * sqrt(5.0));
    double mu = sqrt(5.0) / 10.0;
    double zeta = 4;

    EXPECT_NEAR(1.0, moveByTwoYJacobian(iOmega, iOmega), 1e-7);
    EXPECT_NEAR(0.0, moveByTwoYJacobian(iOmega, iPhi0), 1e-7);
    EXPECT_NEAR(0.0, moveByTwoYJacobian(iOmega, iD0), 1e-7);

    EXPECT_NEAR(2.0 / 5.0, moveByTwoYJacobian(iPhi0, iOmega), 1e-7);
    EXPECT_NEAR(1.0 / 5.0, moveByTwoYJacobian(iPhi0, iPhi0), 1e-7);
    EXPECT_NEAR(-2.0 / 5.0, moveByTwoYJacobian(iPhi0, iD0), 1e-7);

    EXPECT_NEAR(mu * zeta - A * lambda, moveByTwoYJacobian(iD0, iOmega), 1e-7);
    EXPECT_NEAR(2.0 * mu * u * deltaParallel, moveByTwoYJacobian(iD0, iPhi0), 1e-7);
    EXPECT_NEAR(2.0 * mu * nu, moveByTwoYJacobian(iD0, iD0), 1e-7);

  }


  TEST_F(HelixTest, calcPassiveMoveByJacobian_consistency_of_expansion)
  {
    TVector3 center(0.0, 1.0, 0.0);
    float radius = -1;
    float tanLambda = 3;

    Helix helix = helixFromCenter(center, radius, tanLambda);

    for (const TVector3& by : bys) {
      TMatrixD jacobian = helix.calcPassiveMoveByJacobian(by);
      TMatrixD jacobianWithExpansion = helix.calcPassiveMoveByJacobian(by, 10000);

      EXPECT_NEAR(1.0, jacobian(iOmega, iOmega), 1e-7);
      EXPECT_NEAR(0.0, jacobian(iOmega, iPhi0), 1e-7);
      EXPECT_NEAR(0.0, jacobian(iOmega, iD0), 1e-7);

      EXPECT_NEAR(1.0, jacobianWithExpansion(iOmega, iOmega), 1e-7);
      EXPECT_NEAR(0.0, jacobianWithExpansion(iOmega, iPhi0), 1e-7);
      EXPECT_NEAR(0.0, jacobianWithExpansion(iOmega, iD0), 1e-7);

      EXPECT_NEAR(jacobianWithExpansion(iD0, iD0), jacobian(iD0, iD0), 1e-7);
      EXPECT_NEAR(jacobianWithExpansion(iD0, iPhi0), jacobian(iD0, iPhi0), 1e-7);
      EXPECT_NEAR(jacobianWithExpansion(iD0, iOmega), jacobian(iD0, iOmega), 1e-7);

      EXPECT_NEAR(jacobianWithExpansion(iPhi0, iD0), jacobian(iPhi0, iD0), 1e-7);
      EXPECT_NEAR(jacobianWithExpansion(iPhi0, iPhi0), jacobian(iPhi0, iPhi0), 1e-7);
      EXPECT_NEAR(jacobianWithExpansion(iPhi0, iOmega), jacobian(iPhi0, iOmega), 1e-7);
    }
  }

  TEST_F(HelixTest, realExample)
  {
    // Example from Belle I data.
    std::vector<double> helixParams(5);
    helixParams[0] = 3.82384;
    helixParams[1] = std::remainder(3.575595, 2 * M_PI);
    helixParams[2] = 0.00530726;
    helixParams[3] = -0.000317335;
    helixParams[4] = 1.34536;

    TVector3 momentum(-0.768755, -0.356297, 1.13994);
    TVector3 position(-1.60794, 3.46933, -0.000317335);

    // Note: The helix parameters already have small mismatches that can be fixed as follows
    // helixParams[1] = std::atan2(static_cast<double>(momentum.Y()), static_cast<double>(momentum.X()));
    // helixParams[0] = static_cast<double>(position.Perp());

    const double bZ = 1.5;

    // Test if the cartesian coordinates are at the perigee
    EXPECT_NEAR(0.0, momentum.XYvector() * position.XYvector(), 1e-6);

    Helix helix(helixParams[0], helixParams[1], helixParams[2], helixParams[3], helixParams[4]);

    EXPECT_ALL_NEAR(momentum, helix.getMomentum(bZ), 1e-5);
    EXPECT_ALL_NEAR(position, helix.getPerigee(), 1e-5);
  }


  TEST_F(HelixTest, omegaForUnitMomentum)
  {
    TVector3 expectedMomentum(1.0, 0.0, 0.0);
    TVector3 expectedPosition(0.0, 1.0, 0.0);
    const double expectedCharge = 1;
    const double bZ = 1.5;

    Helix helix(expectedPosition, expectedMomentum, expectedCharge, bZ);

    double expectedOmega = 0.0044968868700000003; // Omega for one GeV
    EXPECT_ALL_NEAR(expectedOmega, helix.getOmega(), 1e-7);
  }
}
