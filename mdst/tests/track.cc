#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {



  /** Test class for the Track object.
   */
  class TrackTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters.
   */
  TEST_F(TrackTest, settersNGetters)
  {
    //Create some TrackFitResults in the the DataStore.
    //PDGCode of the TrackFitResult will be used in the test to identify the TFR.
    DataStore::Instance().setInitializeActive(true);
    StoreArray<TrackFitResult>::registerPersistent();
    StoreArray<TrackFitResult> myResults;

    const TVector3 dummyVector3;
    const TMatrixDSym dummyMatrix(6);
    const int charge = 1;
    const float pValue = 1.;
    const float bField = 1.5;

    const auto myPion = myResults.appendNew(dummyVector3, dummyVector3, dummyMatrix, charge, Const::pion,     pValue, bField, 0, 0);

    const auto myKaon = myResults.appendNew(dummyVector3, dummyVector3, dummyMatrix, charge, Const::kaon,     pValue, bField, 0, 0);

    const auto myElectron = myResults.appendNew(dummyVector3, dummyVector3, dummyMatrix, charge, Const::electron, pValue, bField, 0, 0);

    const auto myMuon = myResults.appendNew(dummyVector3, dummyVector3, dummyMatrix, charge, Const::muon,     pValue, bField, 0, 0);

    EXPECT_EQ(myPion->getArrayIndex(), 0);
    EXPECT_EQ(myKaon->getArrayIndex(), 1);
    EXPECT_EQ(myElectron->getArrayIndex(), 2);
    EXPECT_EQ(myMuon->getArrayIndex(), 3);

    Track mytrack1;
    mytrack1.setTrackFitResultIndex(Const::pion, myPion->getArrayIndex());
    mytrack1.setTrackFitResultIndex(Const::muon, myMuon->getArrayIndex());
    mytrack1.setTrackFitResultIndex(Const::kaon, myKaon->getArrayIndex());

    EXPECT_EQ(mytrack1.getNumberOfFittedHypotheses(), 3);
    // If the index of the corresponding particle is set, the correct particle should be returned.
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::pion)->getParticleType(), Const::pion);
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::kaon)->getParticleType(), Const::kaon);
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::muon)->getParticleType(), Const::muon);
    // If the index of the corresponding particle is *not* set, the pion hypothesis should be returned.
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::electron)->getParticleType(), Const::pion);

    // If the index of the corresponding particle is set, the correct particle should be returned.
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::pion)->getArrayIndex(), myPion->getArrayIndex());
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::kaon)->getArrayIndex(), myKaon->getArrayIndex());
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::muon)->getArrayIndex(), myMuon->getArrayIndex());
    // If the index of the corresponding particle is *not* set, the pion hypothesis should be returned.
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::electron)->getArrayIndex(), myPion->getArrayIndex());
  }
}  // namespace
