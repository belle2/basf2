#include <framework/dataobjects/Helix.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <TRandom3.h>
#include <TMath.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {



  /** Set up a few arrays and objects in the datastore */
  class HelixTest : public ::testing::Test {
  protected:
    // Common level precision for all tests.
    double absError = 1e-6;
  };

  /** Test simple Setters and Getters. */
  TEST_F(HelixTest, Getters)
  {
    TRandom3 generator;
    unsigned int nCases = 1;
    double bField = 1.5;

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
      EXPECT_NEAR(position.X(), helix.getPosition().X(), absError);
      EXPECT_NEAR(position.Y(), helix.getPosition().Y(), absError);
      EXPECT_NEAR(position.Z(), helix.getPosition().Z(), absError);
      EXPECT_NEAR(momentum.Px(), helix.getMomentum(bField).Px(), absError);
      EXPECT_NEAR(momentum.Py(), helix.getMomentum(bField).Py(), absError);
      EXPECT_NEAR(momentum.Pz(), helix.getMomentum(bField).Pz(), absError);

      // Test getter for transverse momentum
      EXPECT_NEAR(momentum.Perp(), helix.getTransverseMomentum(bField), absError);

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
    const float bField = 1.5;

    Helix helix(position, momentum, charge, bField);
    EXPECT_NEAR(1, helix.getD0(), absError);

    Helix helix2(position, momentum, -charge, bField);
    EXPECT_NEAR(1, helix2.getD0(), absError);

    Helix oppositeMomentumHelix(position, oppositeMomentum, charge, bField);
    EXPECT_NEAR(-1, oppositeMomentumHelix.getD0(), absError);

    Helix oppositeMomentumHelix2(position, oppositeMomentum, -charge, bField);
    EXPECT_NEAR(-1, oppositeMomentumHelix2.getD0(), absError);

  }


  TEST_F(HelixTest, SimpleExtrapolation)
  {
    // Setup a clockwise helix starting at (0, -0.5, 0) which does not surround the origin

    float d0 = 0.5;
    float phi0 = 0;
    float omega = 1.0;
    float z0 = 0;
    float cotTheta = 2;

    Helix helix(d0, phi0, omega, z0, cotTheta);

    // Check setup
    TVector3 perigee(helix.getPosition());
    EXPECT_NEAR(0, perigee.X(), absError);
    EXPECT_NEAR(-d0,  perigee.Y(), absError);
    EXPECT_NEAR(z0, perigee.Z(), absError);

    TVector3 momentumAtPerigee(helix.getMomentum());
    EXPECT_NEAR(phi0,  momentumAtPerigee.Phi(), absError);
    EXPECT_EQ(1,  helix.getChargeSign());


    // Advance a quater of a full circle.
    {
      float s =  M_PI / 2;
      TVector3 extrapolatedPosition = helix.getPositionAtS(s);

      EXPECT_NEAR(1, extrapolatedPosition.X(), absError);
      EXPECT_NEAR(-1.5, extrapolatedPosition.Y(), absError);
      EXPECT_NEAR(M_PI, extrapolatedPosition.Z(), absError);
    }
    // Reverse the helix in place
    helix.reverse();

    // Advance a quater of a circle now in the opposite direction
    {
      float s =  M_PI / 2;
      TVector3 extrapolatedPosition = helix.getPositionAtS(s);
      EXPECT_NEAR(-1, extrapolatedPosition.X(), absError);
      EXPECT_NEAR(-1.5, extrapolatedPosition.Y(), absError);
      EXPECT_NEAR(-M_PI, extrapolatedPosition.Z(), absError);
    }

  }

  TEST_F(HelixTest, SimpleExtrapolation2)
  {
    // Setup a clockwise helix starting at (0.25, 0 , 0) which *does* surround the origin

    float d0 = 0.25;
    float phi0 = M_PI / 2;
    float omega = -1.0;
    float z0 = 0;
    float cotTheta = 2;

    Helix helix(d0, phi0, omega, z0, cotTheta);

    // Check setup
    TVector3 perigee(helix.getPosition());
    EXPECT_NEAR(d0, perigee.X(), absError);
    EXPECT_NEAR(0,  perigee.Y(), absError);
    EXPECT_NEAR(z0, perigee.Z(), absError);

    TVector3 momentumAtPerigee(helix.getMomentum());
    EXPECT_NEAR(phi0,  momentumAtPerigee.Phi(), absError);
    EXPECT_EQ(-1,  helix.getChargeSign());


    // Advance a quater of a full circle.
    {
      float s =  M_PI / 2;
      TVector3 extrapolatedPosition = helix.getPositionAtS(s);

      EXPECT_NEAR(-0.75, extrapolatedPosition.X(), absError);
      EXPECT_NEAR(1, extrapolatedPosition.Y(), absError);
      EXPECT_NEAR(M_PI, extrapolatedPosition.Z(), absError);
    }

    // Reverse the helix in place
    helix.reverse();

    // Advance a quater of a circle now in the opposite direction
    {
      float s =  M_PI / 2;
      TVector3 extrapolatedPosition = helix.getPositionAtS(s);
      EXPECT_NEAR(-0.75, extrapolatedPosition.X(), absError);
      EXPECT_NEAR(-1, extrapolatedPosition.Y(), absError);
      EXPECT_NEAR(-M_PI, extrapolatedPosition.Z(), absError);
    }

  }


}  // namespace
