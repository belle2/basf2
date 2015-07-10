#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TMatrixD.h>
#include <TMatrixDSym.h>
#include <TVector3.h>
#include <TRandom3.h>
#include <TMath.h>

#include <vector>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {



  /** Set up a few arrays and objects in the datastore */
  class TrackFitResultTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(TrackFitResultTest, Getters)
  {
    TRandom3 generator;
    unsigned int nCases = 1;
    double absError = 1e-6;
    double bField = 1.5;

    for (unsigned int i = 0; i < nCases; ++i) {

      short int charge = generator.Uniform(-1, 1) > 0 ? 1 : -1;
      Const::ParticleType pType = Const::electron;
      float pValue = 0.45;

      // Generate a random put orthogonal pair of vectors in the r-phi plane
      TVector2 d(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      TVector2 pt(generator.Uniform(-1, 1), generator.Uniform(-1, 1));
      d.Set(d.X(), -(d.X()*pt.Px()) / pt.Py());
      // Add a random z component
      TVector3 position(d.X(), d.Y(), generator.Uniform(-1, 1));
      TVector3 momentum(pt.Px(), pt.Py(), generator.Uniform(-1, 1));

      TMatrixDSym cov6(6);

      // Set up class for testing
      TrackFitResult myResult(position, momentum, cov6, charge, pType, pValue, bField, 0, 0);

      // Test all vector elements
      EXPECT_NEAR(position.X(), myResult.getPosition().X(), absError);
      EXPECT_NEAR(position.Y(), myResult.getPosition().Y(), absError);
      EXPECT_NEAR(position.Z(), myResult.getPosition().Z(), absError);
      EXPECT_NEAR(momentum.Px(), myResult.getMomentum(bField).Px(), absError);
      EXPECT_NEAR(momentum.Py(), myResult.getMomentum(bField).Py(), absError);
      EXPECT_NEAR(momentum.Pz(), myResult.getMomentum(bField).Pz(), absError);

      // Test getter for transverse momentum
      EXPECT_NEAR(momentum.Perp(), myResult.getTransverseMomentum(bField), absError);

      // Test other variables
      EXPECT_EQ(charge, myResult.getChargeSign());
      EXPECT_EQ(pValue, myResult.getPValue());
      EXPECT_EQ(pType, myResult.getParticleType());
      EXPECT_EQ(pType.getPDGCode(), myResult.getPDG());

    }
  } // Testcases for getters

  /** Test if the error propagation of the covariance matrix works properly. */
  TEST_F(TrackFitResultTest, ErrorPropagation)
  {
    TRandom3 generator;
    unsigned int nCases = 1;
    double absError = 1e-6;

    for (unsigned int i = 0; i < nCases; ++i) {

      auto bField = 1.5;
      auto pType = Belle2::Const::electron;
      auto pValue = 0.45;
      std::vector<float> tau;
      for (int i = 0; i < 5; ++i) {
        // does not matter what is appended here, we only test the cov matrix
        tau.push_back(1);
      }
      std::vector<float> cov(15);
      for (auto& element : cov) {
        element = generator.Gaus(1e-4);
      }
      Belle2::TrackFitResult myResult(tau, cov, pType, pValue, 0, 0);
      TMatrixDSym covariance(myResult.getCovariance6());

      for (int i = 0; i < 5; ++i)
        for (int j = i; j < 5; ++j)
          EXPECT_EQ(covariance(i, j), covariance(j, i));

      TMatrixDSym cov6(6);
      for (unsigned int row = 0; row < 6; ++row) {
        for (unsigned int col = 0; col < 6; ++col) {
          cov6(row, col) = covariance(row, col);
        }
      }
      Belle2::TrackFitResult myResult2(myResult.getPosition(), myResult.getMomentum(), cov6,
                                       myResult.getChargeSign(), pType, pValue, bField, 0, 0);

      TMatrixDSym myResultCov5 = myResult.getCovariance5();
      TMatrixDSym myResult2Cov5 = myResult2.getCovariance5();

      EXPECT_NEAR(myResultCov5(0, 0), myResult2Cov5(0, 0), absError);
      EXPECT_NEAR(myResultCov5(0, 1), myResult2Cov5(0, 1), absError);
      EXPECT_NEAR(myResultCov5(0, 2), myResult2Cov5(0, 2), absError);
      EXPECT_NEAR(myResultCov5(0, 3), myResult2Cov5(0, 3), absError);
      EXPECT_NEAR(myResultCov5(0, 4), myResult2Cov5(0, 4), absError);
      EXPECT_NEAR(myResultCov5(1, 0), myResult2Cov5(1, 0), absError);
      EXPECT_NEAR(myResultCov5(1, 1), myResult2Cov5(1, 1), absError);
      EXPECT_NEAR(myResultCov5(1, 2), myResult2Cov5(1, 2), absError);
      EXPECT_NEAR(myResultCov5(1, 3), myResult2Cov5(1, 3), absError);
      EXPECT_NEAR(myResultCov5(1, 4), myResult2Cov5(1, 4), absError);
      EXPECT_NEAR(myResultCov5(2, 0), myResult2Cov5(2, 0), absError);
      EXPECT_NEAR(myResultCov5(2, 1), myResult2Cov5(2, 1), absError);
      EXPECT_NEAR(myResultCov5(2, 2), myResult2Cov5(2, 2), absError);
      EXPECT_NEAR(myResultCov5(2, 3), myResult2Cov5(2, 3), absError);
      EXPECT_NEAR(myResultCov5(2, 4), myResult2Cov5(2, 4), absError);
      EXPECT_NEAR(myResultCov5(3, 0), myResult2Cov5(3, 0), absError);
      EXPECT_NEAR(myResultCov5(3, 1), myResult2Cov5(3, 1), absError);
      EXPECT_NEAR(myResultCov5(3, 2), myResult2Cov5(3, 2), absError);
      EXPECT_NEAR(myResultCov5(3, 3), myResult2Cov5(3, 3), absError);
      EXPECT_NEAR(myResultCov5(3, 4), myResult2Cov5(3, 4), absError);
      EXPECT_NEAR(myResultCov5(4, 0), myResult2Cov5(4, 0), absError);
      EXPECT_NEAR(myResultCov5(4, 1), myResult2Cov5(4, 1), absError);
      EXPECT_NEAR(myResultCov5(4, 2), myResult2Cov5(4, 2), absError);
      EXPECT_NEAR(myResultCov5(4, 3), myResult2Cov5(4, 3), absError);
      EXPECT_NEAR(myResultCov5(4, 4), myResult2Cov5(4, 4), absError);

    }
  } // Testcases error propagation

  /** Helix -> Cartesian -> Helix -> Cartesian */
  TEST_F(TrackFitResultTest, H2C2H2C)
  {
    double absError = 1e-4;
    double bField = 1.5;

    // perigee helix parameters
    std::vector<float> hp(5);
    hp[0] = 51.8142;
    hp[1] = 6.99831e-01;
    hp[2] = -2.40505e-02;
    hp[3] = 320.652;
    hp[4] = 3.50734;
    std::vector<float> hpErr(15);

    // corresponding cartesian coordinates
    TVector3 momentum(0.143028, 0.12043, 0.655792);
    TVector3 position(33.373, -39.6353, 320.652);

    auto pType = Belle2::Const::pion;

    TrackFitResult fromHelixP(hp, hpErr, pType, 0.5, 0, 0);
    EXPECT_NEAR(fromHelixP.getMomentum(bField).Px(), momentum.Px(), absError);
    EXPECT_NEAR(fromHelixP.getMomentum(bField).Py(), momentum.Py(), absError);
    EXPECT_NEAR(fromHelixP.getMomentum(bField).Pz(), momentum.Pz(), absError);
    EXPECT_NEAR(fromHelixP.getPosition().X(), position.X(), absError);
    EXPECT_NEAR(fromHelixP.getPosition().Y(), position.Y(), absError);
    EXPECT_NEAR(fromHelixP.getPosition().Z(), position.Z(), absError);

    TMatrixDSym cov6(6);
    TrackFitResult fromHelixP2Cartesian(fromHelixP.getPosition(), fromHelixP.getMomentum(bField), cov6, fromHelixP.getChargeSign(),
                                        pType, 0.5, bField, 0, 0);
    EXPECT_NEAR(fromHelixP2Cartesian.getMomentum(bField).Px(), momentum.Px(), absError);
    EXPECT_NEAR(fromHelixP2Cartesian.getMomentum(bField).Py(), momentum.Py(), absError);
    EXPECT_NEAR(fromHelixP2Cartesian.getMomentum(bField).Pz(), momentum.Pz(), absError);
    EXPECT_NEAR(fromHelixP2Cartesian.getPosition().X(), position.X(), absError);
    EXPECT_NEAR(fromHelixP2Cartesian.getPosition().Y(), position.Y(), absError);
    EXPECT_NEAR(fromHelixP2Cartesian.getPosition().Z(), position.Z(), absError);
  }

}  // namespace
