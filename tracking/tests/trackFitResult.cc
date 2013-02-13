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
    EXPECT_LT(pValue - 1e-5, myResult.getPValue());
    EXPECT_GT(pValue + 1e-5, myResult.getPValue());

    myResult.setCharge(1);
    EXPECT_EQ(1, myResult.getCharge());

    myResult.setCharge(-10);
    EXPECT_EQ(-1, myResult.getCharge());

    myResult.setPDGCode(Const::electron);
    EXPECT_EQ(11, myResult.getPDGCode());
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
    EXPECT_LT(pos.Mag() - 1e-5, myResult.getPosition().Mag());
    EXPECT_GT(pos.Mag() + 1e-5, myResult.getPosition().Mag());

    EXPECT_LT(pos.Phi() - 1e-5, myResult.getPosition().Phi());
    EXPECT_GT(pos.Phi() + 1e-5, myResult.getPosition().Phi());

    EXPECT_LT(pos.Eta() - 1e-5, myResult.getPosition().Eta());
    EXPECT_GT(pos.Eta() + 1e-5, myResult.getPosition().Eta());

    //Momentum
    EXPECT_LT(mom.Mag() - 1e-5, myResult.getMomentum().Mag());
    EXPECT_GT(mom.Mag() + 1e-5, myResult.getMomentum().Mag());

    EXPECT_LT(mom.Phi() - 1e-5, myResult.getMomentum().Phi());
    EXPECT_GT(mom.Phi() + 1e-5, myResult.getMomentum().Phi());

    EXPECT_LT(mom.Eta() - 1e-5, myResult.getMomentum().Eta());
    EXPECT_GT(mom.Eta() + 1e-5, myResult.getMomentum().Eta());

    //Covariance
    //Dangerous, because expectation of bite-wise equality is unreasonable if there are internal
    // conversions.
    EXPECT_EQ(errorMatrix, myResult.getCovariance6());

  }

}  // namespace
