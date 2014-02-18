#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/TrackFitResult.h>
//#include <genfit/Track.h>

#include <TMatrixF.h>
#include <TMatrixDSym.h>
#include <TVector3.h>
#include <TRandom3.h>
#include <TMath.h>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

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
      TrackFitResult myResult(position, momentum, cov6, charge, pType, pValue, bField);

      // Test all vector elements
      EXPECT_NEAR(position.X(), myResult.getPosition().X(), absError);
      EXPECT_NEAR(position.Y(), myResult.getPosition().Y(), absError);
      EXPECT_NEAR(position.Z(), myResult.getPosition().Z(), absError);
      EXPECT_NEAR(momentum.Px(), myResult.getMomentum(bField).Px(), absError);
      EXPECT_NEAR(momentum.Py(), myResult.getMomentum(bField).Py(), absError);
      EXPECT_NEAR(momentum.Pz(), myResult.getMomentum(bField).Pz(), absError);

      // Test other variables
      EXPECT_EQ(charge, myResult.getCharge());
      EXPECT_EQ(pValue, myResult.getPValue());
      EXPECT_EQ(pType, myResult.getParticleType());

    }
  } // Testcases for getters

  TEST_F(TrackFitResultTest, ErrorPropagation)
  {
    // TODO:
  }


}  // namespace
