#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <gtest/gtest.h>

#include <algorithm>

using namespace std;

namespace Belle2 {



  /** Test class for the Track object.
   */
  class TrackTest : public ::testing::Test {
  protected:
  };

  /**
   * Utility function to create dummy TrackFitResults
   */
  TrackFitResult const* addDummyTrack(StoreArray<TrackFitResult>& trackFitResults, Const::ChargedStable particeType)
  {
    const TVector3 dummyVector3;
    const TMatrixDSym dummyMatrix(6);
    const int charge = 1;
    const float pValue = 1.;
    const float bField = 1.5;

    const auto newFitRes = trackFitResults.appendNew(dummyVector3, dummyVector3, dummyMatrix, charge, particeType, pValue,
                                                     bField, 0, 0);
    return newFitRes;
  }

  /** Test simple Setters and Getters.
   */
  TEST_F(TrackTest, settersNGetters)
  {
    //Create some TrackFitResults in the the DataStore.
    //PDGCode of the TrackFitResult will be used in the test to identify the TFR.
    DataStore::Instance().setInitializeActive(true);
    StoreArray<TrackFitResult> myResults;
    myResults.registerInDataStore();

    const auto myPion = addDummyTrack(myResults, Const::pion);
    const auto myKaon = addDummyTrack(myResults, Const::kaon);
    const auto myElectron = addDummyTrack(myResults, Const::electron);
    const auto myMuon = addDummyTrack(myResults, Const::muon);

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
    // If the index of the corresponding particle is *not* set, a nullptr should be returned.
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::electron), nullptr);

    // If the index of the corresponding particle is set, the correct particle should be returned.
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::pion)->getArrayIndex(), myPion->getArrayIndex());
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::kaon)->getArrayIndex(), myKaon->getArrayIndex());
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::muon)->getArrayIndex(), myMuon->getArrayIndex());
    // If the index of the corresponding particle is *not* set, a nullptr should be returned.
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::electron), nullptr);

    const auto allResults = mytrack1.getTrackFitResults();
    // should return all hypothesis which were added before
    EXPECT_EQ(allResults.size(), 3);

    // check that all correct hypothesis are returned and the electron is not
    auto countPion = std::count_if(allResults.begin(), allResults.end(),
                                   [myPion](std::pair<Const::ChargedStable,  const TrackFitResult*> fitPair)
    {return (fitPair.first == Const::pion) && (fitPair.second->getArrayIndex() == myPion->getArrayIndex());});
    auto countMuon = std::count_if(allResults.begin(), allResults.end(),
                                   [myMuon](std::pair<Const::ChargedStable,  const TrackFitResult*> fitPair)
    {return (fitPair.first == Const::muon) && (fitPair.second->getArrayIndex() == myMuon->getArrayIndex());});
    auto countElectron = std::count_if(allResults.begin(), allResults.end(),
                                       [](std::pair<Const::ChargedStable,  const TrackFitResult*> fitPair)
    {return fitPair.first == Const::electron;});
    auto countKaon = std::count_if(allResults.begin(), allResults.end(),
                                   [myKaon](std::pair<Const::ChargedStable,  const TrackFitResult*> fitPair)
    {return (fitPair.first == Const::kaon) && (fitPair.second->getArrayIndex() == myKaon->getArrayIndex());});

    EXPECT_EQ(countPion, 1);
    EXPECT_EQ(countMuon, 1);
    EXPECT_EQ(countElectron, 0);
    EXPECT_EQ(countKaon, 1);

    Track trackQITest;
    EXPECT_EQ(trackQITest.getQualityIndicator(), 0.);
    Track trackQITest1(0.5);
    EXPECT_EQ(trackQITest1.getQualityIndicator(), 0.5);
  }

  /** Test simple Setters and Getters.
   */
  TEST_F(TrackTest, getTrackFitResultWithClosestMass)
  {
    //Create some TrackFitResults in the the DataStore.
    //PDGCode of the TrackFitResult will be used in the test to identify the TFR.
    DataStore::Instance().setInitializeActive(true);
    StoreArray<TrackFitResult> myResults;
    myResults.registerInDataStore();

    // add two fit results
    const auto myKaon = addDummyTrack(myResults, Const::kaon);
    const auto myElectron = addDummyTrack(myResults, Const::electron);

    Track mytrack1;
    mytrack1.setTrackFitResultIndex(Const::electron, myElectron->getArrayIndex());
    mytrack1.setTrackFitResultIndex(Const::kaon, myKaon->getArrayIndex());

    EXPECT_EQ(mytrack1.getNumberOfFittedHypotheses(), 2);

    // check for correct hypothesis if we request a fitted particle
    const auto fitCloseToKaonMass = mytrack1.getTrackFitResultWithClosestMass(Const::kaon);
    EXPECT_EQ(Const::kaon.getPDGCode(), fitCloseToKaonMass->getParticleType().getPDGCode());
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::kaon)->getArrayIndex(), fitCloseToKaonMass->getArrayIndex());

    // check to get Pion fit
    const auto wantPionButHaveElectronFit = mytrack1.getTrackFitResultWithClosestMass(Const::pion);
    EXPECT_EQ(Const::electron.getPDGCode(), wantPionButHaveElectronFit->getParticleType().getPDGCode());
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::electron)->getArrayIndex(), wantPionButHaveElectronFit->getArrayIndex());

    // check to get Electron fit
    const auto wantMuonButHaveElectronFit = mytrack1.getTrackFitResultWithClosestMass(Const::muon);
    EXPECT_EQ(Const::electron.getPDGCode(), wantMuonButHaveElectronFit->getParticleType().getPDGCode());
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::electron)->getArrayIndex(), wantMuonButHaveElectronFit->getArrayIndex());

    // check to get Proton fit
    const auto wantProtonButHaveKaonFit = mytrack1.getTrackFitResultWithClosestMass(Const::proton);
    EXPECT_EQ(Const::kaon.getPDGCode(), wantProtonButHaveKaonFit->getParticleType().getPDGCode());
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::kaon)->getArrayIndex(), wantProtonButHaveKaonFit->getArrayIndex());

  }
}  // namespace
