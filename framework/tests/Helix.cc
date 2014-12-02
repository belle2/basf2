#include <framework/dataobjects/Helix.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <TRandom3.h>
#include <TMath.h>

#include <gtest/gtest.h>

using namespace std;

// Additional tests
/// Expectation macro for combound structures like vectors to be close to each other
#define EXPECT_ALL_NEAR(expected, actual, delta) EXPECT_PRED3(near, expected, actual, delta)

/// Assertation macro for combound structures like vectors to be close to each other
#define ASSERT_ALL_NEAR(expected, actual, delta) ASSERT_PRED3(near, expected, actual, delta)

/// Expectation macro for angle values that should not care for a multiple of 2 * PI difference between the values
#define EXPECT_ANGLE_NEAR(expected, actual, delta) EXPECT_PRED3(angleNear, expected, actual, delta)

/// Assertation macro for angle values that should not care for a multiple of 2 * PI difference between the values
#define ASSERT_ANGLE_NEAR(expected, actual, delta) EXPECT_PRED3(angleNear, expected, actual, delta)

/// Sting output operator for a TVector3 for gtest print support.
std::ostream& operator<<(std::ostream& output, const TVector3& tVector3)
{
  return output
         << "TVector3("
         << tVector3.X() << ", "
         << tVector3.Y() << ", "
         << tVector3.Z() << ")";
}


namespace {
  /** Predicate checking that all three components of TVector are close by a maximal error of absError*/
  bool near(const TVector3& expected, const TVector3& actual, const float& absError)
  {

    bool xNear = fabs(expected.X() - actual.X()) < absError;
    bool yNear = fabs(expected.Y() - actual.Y()) < absError;
    bool zNear = fabs(expected.Z() - actual.Z()) < absError;
    return xNear and yNear and zNear;
  }

  /** Predicate checking that two angular values are close to each other modulus a 2 * PI difference. */
  bool angleNear(const float& expected, const float& actual, const float& absError)
  {
    return fabs(remainder(expected - actual, 2 * M_PI)) < absError;
  }

}


namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class HelixTest : public ::testing::Test {

  protected:
    // Common level precision for all tests.
    double absError = 1e-6;
    double nominalBz = 1.5;

    std::vector<float> omegas { -1, 0, 1};
    std::vector<float> phi0s;
    std::vector<float> d0s { -0.5, -0.2, 0, 0.2, 0.5};
    std::vector<float> chis;

    virtual void SetUp() {
      // Make a sample from the full angle range
      phi0s.clear();
      for (int iAngle = -4; iAngle <= 5; ++iAngle) {
        float angle = 2 * M_PI * iAngle / 10.0;
        phi0s.push_back(angle);
      }

      // Make a sample from the full angle range
      // Avoid the far end of the helix for now.
      chis.clear();
      for (int iAngle = -4; iAngle < 5; ++iAngle) {
        float angle = 2 * M_PI * iAngle / 10.0;
        chis.push_back(angle);
      }

    }

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
      d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());
      // Add a random z component
      TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
      TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));

      // Set up class for testing
      Helix helix(position, momentum, charge, bField);

      // Test all vector elements
      EXPECT_ALL_NEAR(position, helix.getPosition(), absError);
      EXPECT_ALL_NEAR(momentum, helix.getMomentum(bField), absError);

      // Test getter for transverse momentum
      EXPECT_NEAR(momentum.Perp(), helix.getTransverseMomentum(bField), absError);

      // Test getter of kappa
      EXPECT_NEAR(charge / momentum.Perp(), helix.getKappa(bField), absError);

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


  TEST_F(HelixTest, Tangential)
  {
    float z0 = 0;
    float tanLambda = 2;

    for (const float d0 : d0s) {
      for (const float phi0 : phi0s) {
        for (const float omega : omegas) {

          Helix helix(d0, phi0, omega, z0, tanLambda);

          TVector3 tangentialAtPerigee = helix.getUnitTangentialAtArcLength(0.0);

          EXPECT_ANGLE_NEAR(phi0, tangentialAtPerigee.Phi(), absError);
          EXPECT_FLOAT_EQ(1.0, tangentialAtPerigee.Mag());
          EXPECT_FLOAT_EQ(tanLambda, 1 / tan(tangentialAtPerigee.Theta()));

          for (const float chi : chis) {

            if (omega == 0) {
              // Use chi as the arc length in the straight line case
              float arcLength = chi;

              // Tangential vector shall not change along the line
              TVector3 tangential = helix.getUnitTangentialAtArcLength(arcLength);
              EXPECT_ALL_NEAR(tangentialAtPerigee, tangential, absError);

            } else {
              float arcLength  =  chi / omega;
              TVector3 tangential = helix.getUnitTangentialAtArcLength(arcLength);

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
            TVector3 momentumAtPerigee = helix.getMomentum();
            for (const float chi : chis) {

              float arcLength = chi / omega;
              TVector3 extrapolatedMomentum = helix.getMomentumAtArcLength(arcLength, nominalBz);

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
          TVector3 perigee = helix.getPosition();

          TVector3 tangentialAtPerigee = helix.getUnitTangentialAtArcLength(0.0);

          std::ostringstream message;
          message << "Failed for " << helix;
          SCOPED_TRACE(message.str());

          for (const float chi : chis) {

            std::ostringstream message;
            message << "Failed for chi=" << chi;
            SCOPED_TRACE(message.str());

            // In the cases where omega is 0 (straight line case) chi become undefined.
            // Use chi sample as transverse travel distance instead.
            float expectedArcLength = omega != 0 ? chi / omega : chi;
            TVector3 pointOnHelix = helix.getPositionAtArcLength(expectedArcLength);

            float polarR = pointOnHelix.Perp();
            float arcLength = helix.getArcLengthAtPolarR(polarR);

            // Only the absolute value is returned.
            EXPECT_NEAR(fabs(expectedArcLength), arcLength, absError);

            // Also check it the extrapolation lies in the forward direction.
            TVector3 secantVector = pointOnHelix - perigee;

            if (expectedArcLength == 0) {
              EXPECT_NEAR(0, secantVector.Mag(), absError);
            } else {
              TVector2 secantVectorXY = secantVector.XYvector();

              TVector2 tangentialXY = tangentialAtPerigee.XYvector();
              float coalignment = secantVectorXY * tangentialXY ;

              bool extrapolationIsForward = coalignment > 0;
              bool expectedIsForward = expectedArcLength > 0;
              EXPECT_EQ(expectedIsForward, extrapolationIsForward);
            }
          }
        }
      }
    }
  } // end TEST_F


  TEST_F(HelixTest, PerigeeExtrapolateRoundTrip)
  {

    float z0 = 0;
    float tanLambda = -2;

    for (const float d0 : d0s) {
      for (const float phi0 : phi0s) {
        for (const float omega : omegas) {

          if (omega != 0) {
            Helix expectedHelix(d0, phi0, omega, z0, tanLambda);
            TVector3 perigee = expectedHelix.getPosition();
            TVector3 momentum = expectedHelix.getMomentum(nominalBz);
            int chargeSign = expectedHelix.getChargeSign();

            Helix helix(perigee, momentum, chargeSign, nominalBz);

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

}  // namespace
