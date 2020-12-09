#include <tracking/v0Finding/fitter/V0Fitter.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <mdst/dataobjects/TrackFitResult.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>

#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/ConstField.h>
#include <genfit/TGeoMaterialInterface.h>

#include <utility>

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class V0FitterTest : public ::testing::Test {
  protected:
    /// Setup a "working environment" with store arrays for the hits and the correct relations.
    void SetUp() override
    {
      /// Name of the RecoTrack store array.
      m_storeArrayNameOfRecoTracks = "ILoveRecoTracks";
      /// Name of the TrackFitResults store array.
      m_storeArrayNameOfTrackFitResults = "TrackFitResultsAreTheBest";

      //--- Setup -----------------------------------------------------------------------
      DataStore::Instance().setInitializeActive(true);
      StoreArray<RecoTrack> recoTracks(m_storeArrayNameOfRecoTracks);
      recoTracks.registerInDataStore();
      StoreArray<TrackFitResult> trackFitResults(m_storeArrayNameOfTrackFitResults);
      trackFitResults.registerInDataStore();

      genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());

      // This is copied from genfit2/code2/gtest/TestConstField.cpp
      const double bFieldZ = 20;  // kGauss. Is 1.5T
      m_constField = new genfit::ConstField(0., 0., bFieldZ);
      genfit::FieldManager::getInstance()->init(m_constField);
    }

    std::string m_storeArrayNameOfRecoTracks; /**< name of recoTracks storeArray */
    std::string m_storeArrayNameOfTrackFitResults; /**< name of the TrackFitResults StoreArray */

    genfit::ConstField* m_constField; /**< constant magnetic field for genfit::FieldManager */
  };

  /// Test getter for track hypotheses.
  TEST_F(V0FitterTest, GetTrackHypotheses)
  {
    V0Fitter v0Fitter(m_storeArrayNameOfTrackFitResults, "V0_one", "", m_storeArrayNameOfRecoTracks, "copies_one");
    const auto kShortTracks = v0Fitter.getTrackHypotheses(Const::Kshort);
    const auto photonTracks = v0Fitter.getTrackHypotheses(Const::photon);
    const auto lambdaTracks = v0Fitter.getTrackHypotheses(Const::Lambda);
    const auto antiLambdaTracks = v0Fitter.getTrackHypotheses(Const::antiLambda);

    EXPECT_EQ(Const::pion, kShortTracks.first);
    EXPECT_EQ(Const::pion, kShortTracks.second);

    EXPECT_EQ(Const::electron, photonTracks.first);
    EXPECT_EQ(Const::electron, photonTracks.second);

    EXPECT_EQ(Const::proton, lambdaTracks.first);
    EXPECT_EQ(Const::pion, lambdaTracks.second);

    EXPECT_EQ(Const::pion, antiLambdaTracks.first);
    EXPECT_EQ(Const::proton, antiLambdaTracks.second);
  }

  /// Test initialization of cuts.
  TEST_F(V0FitterTest, InitializeCuts)
  {
    V0Fitter v0Fitter(m_storeArrayNameOfTrackFitResults, "V0_two", "", m_storeArrayNameOfRecoTracks, "copies_two");
    v0Fitter.initializeCuts(1.0, 52.);
    EXPECT_EQ(1.0, v0Fitter.m_beamPipeRadius);
    EXPECT_EQ(52., v0Fitter.m_vertexChi2CutOutside);
  }

}  // namespace
