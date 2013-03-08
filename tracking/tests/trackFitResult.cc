#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/TrackFitResult.h>
#include <GFTrack.h>
#include <TMatrixF.h>
#include <RKTrackRep.h>


#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class TrackFitResultTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(TrackFitResultTest, settersNGetters)
  {
    TrackFitResult myResult;
    float pValue = 0.45;
    myResult.setPValue(pValue);
    EXPECT_FLOAT_EQ(pValue, myResult.getPValue());

    myResult.setCharge(1);
    EXPECT_EQ(1, myResult.getCharge());

    myResult.setCharge(-10);
    EXPECT_EQ(-1, myResult.getCharge());

    myResult.setParticleType(Const::electron);
    EXPECT_EQ(Const::electron, myResult.getParticleType());
  }

  /** Test Position/Momentum Vectors and respective error matrix input and retrieval.*/
  TEST_F(TrackFitResultTest, posMomErrorIO)
  {
    TVector3 pos(0.1, 0.1, 0.1);
    TVector3 mom(1.0, 1.0, -1.0);
    TMatrixF errorMatrix(6, 6);
    for (int ii = 0; ii < 6; ii++) {
      for (int jj = ii; jj < 6; jj++) {
        errorMatrix(ii, jj) = 0.01;
      }
    }
    TrackFitResult myResult;
    myResult.setPosition(pos);
    myResult.setMomentum(mom);
    myResult.setCovariance6(errorMatrix);

    //The following should be replaced with proper inspection,
    // if the TVector3 are compatible within the tolerance.
    //Position
    EXPECT_FLOAT_EQ(pos.Mag(), myResult.getPosition().Mag());

    EXPECT_FLOAT_EQ(pos.Phi(), myResult.getPosition().Phi());

    EXPECT_FLOAT_EQ(pos.Eta(), myResult.getPosition().Eta());

    //Momentum
    EXPECT_FLOAT_EQ(mom.Mag(), myResult.getMomentum().Mag());

    EXPECT_FLOAT_EQ(mom.Phi(), myResult.getMomentum().Phi());

    EXPECT_FLOAT_EQ(mom.Eta(), myResult.getMomentum().Eta());

    //Covariance
    //Dangerous, because expectation of bite-wise equality is unreasonable if there are internal
    // conversions.
    EXPECT_EQ(errorMatrix, myResult.getCovariance6());

  }

}  // namespace
