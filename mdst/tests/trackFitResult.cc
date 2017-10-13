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
      EXPECT_NEAR(momentum.Px(), myResult.getMomentum().Px(), absError);
      EXPECT_NEAR(momentum.Py(), myResult.getMomentum().Py(), absError);
      EXPECT_NEAR(momentum.Pz(), myResult.getMomentum().Pz(), absError);

      // Test getter for transverse momentum
      EXPECT_NEAR(momentum.Perp(), myResult.getTransverseMomentum(), absError);

      // Test other variables
      EXPECT_EQ(charge, myResult.getChargeSign());
      EXPECT_EQ(pValue, myResult.getPValue());
      EXPECT_EQ(pType, myResult.getParticleType());

    }
  } // Testcases for getters

  /** Test if the error propagation of the covariance matrix works properly. */
  TEST_F(TrackFitResultTest, ErrorPropagation)
  {
    TRandom3 generator;
    unsigned int nCases = 1;
    double absError = 1e-6;

    for (unsigned int iCase = 0; iCase < nCases; ++iCase) {

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

  /** Test get charge. */
  TEST_F(TrackFitResultTest, Charge)
  {
    auto bField = 1.5;
    auto pValue = 0.45;
    TVector3 position(0., 0., 0.);
    TVector3 momentum(1., 1., 1.);
    TMatrixDSym cov6(6);
    auto pType = Belle2::Const::electron;

    auto charge = -1.0;
    Belle2::TrackFitResult myResultMinus(position, momentum, cov6, charge, pType, pValue, bField, 0, 0);
    EXPECT_EQ(myResultMinus.getChargeSign(), charge);

    charge = 0;
    Belle2::TrackFitResult myResultNull(position, momentum, cov6, charge, pType, pValue, bField, 0, 0);
    EXPECT_EQ(myResultNull.getChargeSign(), charge);

    charge = +1.0;
    Belle2::TrackFitResult myResultPlus(position, momentum, cov6, charge, pType, pValue, bField, 0, 0);
    EXPECT_EQ(myResultPlus.getChargeSign(), charge);
  }


}  // namespace
