/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <variant>

#include <gtest/gtest.h>

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/dataobjects/VxdID.h>
#include <genfit/TrackPoint.h>
#include <genfit/AbsFitterInfo.h>

#include <svd/variables/SVDClusterVariables.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace Belle2;

namespace {

  const Belle2::VxdID defaultVxdID = Belle2::VxdID("3.2.1");
  const bool defaultIsU = true;
  const double defaultPosition = 1.0;
  const double defaultPositionErr = 0.01;
  const double defaultClsTime = 20.0;
  const double defaultClsTimeErr = 5.0;
  const double defaultClsCharge = 23e3;
  const double defaultSeedCharge = 5e3;
  const int defaultClsSize = 2;
  const double defaultClsSNR = 25.0;
  const double defaultClsChi2 = 1.0;
  const int defaultFirstFrame = 0;

  auto dummySVDCluster = []() -> Belle2::SVDCluster
  {
    return Belle2::SVDCluster(
             defaultVxdID, defaultIsU, defaultPosition, defaultPositionErr,
             defaultClsTime, defaultClsTimeErr, defaultClsCharge, defaultSeedCharge,
             defaultClsSize, defaultClsSNR, defaultClsChi2, defaultFirstFrame
           );
  }();
}

namespace Belle2::SVD {

  /** Text fixture for SVD variables. */
  class SVDVariableTest : public ::testing::Test {
  public:

    /** Set up test environment */
    void SetUp() override
    {
      std::string storeArrayNameOfParticles = "Particles";
      std::string storeArrayNameOfTracks = "Tracks";
      std::string storeArrayNameOfRecoTracks = "RecoTracks";
      std::string storeArrayNameOfPXDHits = "PXDClusters";
      std::string storeArrayNameOfSVDHits = "SVDClusters";
      std::string storeArrayNameOfCDCHits = "CDCClusters";
      std::string storeArrayNameOfBKLMHits = "BKLMClusters";
      std::string storeArrayNameOfEKLMHits = "EKLMClusters";
      std::string storeArrayNameOfRecoHitInformations = "RecoHitInformations";
      std::string storeArrayNameOfTrackFitResults = "TrackFitResults";

      DataStore::Instance().setInitializeActive(true);
      m_particles = StoreArray<Particle>(storeArrayNameOfParticles);
      m_tracks = StoreArray<Track>(storeArrayNameOfTracks);
      m_recoTracks = StoreArray<RecoTrack>(storeArrayNameOfRecoTracks);
      m_svdClusters = StoreArray<SVDCluster>(storeArrayNameOfSVDHits);
      m_recoHitInformations = StoreArray<RecoHitInformation>(storeArrayNameOfRecoHitInformations);
      m_trackFitResults = StoreArray<TrackFitResult>(storeArrayNameOfTrackFitResults);

      m_particles.registerInDataStore();
      m_tracks.registerInDataStore();
      m_recoTracks.registerInDataStore();
      m_svdClusters.registerInDataStore();
      m_recoHitInformations.registerInDataStore();
      m_trackFitResults.registerInDataStore();

      m_tracks.registerRelationTo(m_recoTracks);
      m_recoTracks.registerRelationTo(m_recoHitInformations);
      m_recoHitInformations.registerRelationTo(m_svdClusters);
      m_svdClusters.registerRelationTo(m_recoTracks);
      m_eventT0.registerInDataStore();
      DataStore::Instance().setInitializeActive(false);

      m_trackFitResults.appendNew(ROOT::Math::XYZVector(0.1, 0.1, 0.1), ROOT::Math::XYZVector(0.1, 0.0, 0.0),
                                  TMatrixDSym(6), 1, Const::pion, 0.01, 1.5, 0, 0, 0);

      m_tracks.appendNew(Track());
      m_tracks[0]->setTrackFitResultIndex(Const::pion, 0);

      m_particles.appendNew(Particle(m_tracks[0], Const::pion));

      m_recoTracks.appendNew(ROOT::Math::XYZVector(0.1, 0.1, 0.1), ROOT::Math::XYZVector(0.1, 0.0, 0.0), 1,
                             storeArrayNameOfPXDHits, storeArrayNameOfSVDHits, storeArrayNameOfCDCHits,
                             storeArrayNameOfBKLMHits, storeArrayNameOfEKLMHits, storeArrayNameOfRecoHitInformations);

      m_tracks[0]->addRelationTo(m_recoTracks[0]);
      m_svdClusters.appendNew(dummySVDCluster);
      m_recoTracks[0]->addSVDHit(m_svdClusters[0], 0);
    }

    /** Clear test environment */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }

    StoreArray<Particle> m_particles; /**< StoreArray for Particle objects */
    StoreArray<Track> m_tracks; /**< StoreArray for Track objects */
    StoreArray<RecoTrack> m_recoTracks; /**< StoreArray for RecoTrack objects */
    StoreArray<SVDCluster> m_svdClusters; /**< StoreArray for SVDCluster objects */
    StoreArray<TrackFitResult> m_trackFitResults; /**< StoreArray for TrackFitResult objects */
    StoreArray<RecoHitInformation> m_recoHitInformations; /**< StoreArray for RecoHitInformation objects */
    StoreObjPtr<EventT0> m_eventT0; /**< StoreObjPtr for EventT0 */
  };

  /** Test SVDClusterCharge */
  TEST_F(SVDVariableTest, SVDClusterCharge)
  {
    EXPECT_DOUBLE_EQ(Variable::SVDClusterCharge(m_particles[0], {0}), defaultClsCharge);
    EXPECT_TRUE(std::isnan(Variable::SVDClusterCharge(nullptr,        {0})));
    EXPECT_TRUE(std::isnan(Variable::SVDClusterCharge(m_particles[0], {1})));
  }

  /** Test SVDClusterSNR */
  TEST_F(SVDVariableTest, SVDClusterSNR)
  {
    EXPECT_DOUBLE_EQ(Variable::SVDClusterSNR(m_particles[0], {0}), defaultClsSNR);
    EXPECT_TRUE(std::isnan(Variable::SVDClusterSNR(nullptr,        {0})));
    EXPECT_TRUE(std::isnan(Variable::SVDClusterSNR(m_particles[0], {1})));
  }

  /** Test SVDClusterTime */
  TEST_F(SVDVariableTest, SVDClusterTime)
  {
    EXPECT_DOUBLE_EQ(Variable::SVDClusterTime(m_particles[0], {0}), defaultClsTime);
    EXPECT_TRUE(std::isnan(Variable::SVDClusterTime(nullptr,        {0})));
    EXPECT_TRUE(std::isnan(Variable::SVDClusterTime(m_particles[0], {1})));
  }

  /** Test SVDClusterSize */
  TEST_F(SVDVariableTest, SVDClusterSize)
  {
    EXPECT_EQ(Variable::SVDClusterSize(m_particles[0], {0}), defaultClsSize);
    EXPECT_EQ(Variable::SVDClusterSize(nullptr,        {0}), -1);
    EXPECT_EQ(Variable::SVDClusterSize(m_particles[0], {1}), -1);
  }

  /** SVDLayer */
  TEST_F(SVDVariableTest, SVDLayer)
  {
    EXPECT_EQ(Variable::SVDLayer(m_particles[0], {0}), defaultVxdID.getLayerNumber());
    EXPECT_EQ(Variable::SVDLayer(nullptr,        {0}), -1);
    EXPECT_EQ(Variable::SVDLayer(m_particles[0], {1}), -1);
  }

  /** Test SVDLadder */
  TEST_F(SVDVariableTest, SVDLadder)
  {
    EXPECT_EQ(Variable::SVDLadder(m_particles[0], {0}), defaultVxdID.getLadderNumber());
    EXPECT_EQ(Variable::SVDLadder(nullptr,        {0}), -1);
    EXPECT_EQ(Variable::SVDLadder(m_particles[0], {1}), -1);
  }

  /** Test SVDSensor */
  TEST_F(SVDVariableTest, SVDSensor)
  {
    EXPECT_EQ(Variable::SVDSensor(m_particles[0], {0}), defaultVxdID.getSensorNumber());
    EXPECT_EQ(Variable::SVDSensor(nullptr,        {0}), -1);
    EXPECT_EQ(Variable::SVDSensor(m_particles[0], {1}), -1);
  }

  /** Test SVDClusterChargeNormTrkLength */
  TEST_F(SVDVariableTest, SVDClusterChargeNormTrkLength)
  {
    // Without proper genfit track fitting setup, this should return NaN
    // but the function should not crash
    EXPECT_TRUE(std::isnan(Variable::SVDClusterChargeNormTrkLength(m_particles[0], {0})));
    EXPECT_TRUE(std::isnan(Variable::SVDClusterChargeNormTrkLength(nullptr,        {0})));
    EXPECT_TRUE(std::isnan(Variable::SVDClusterChargeNormTrkLength(m_particles[0], {1})));
  }

  /** Test SVDClusterTimeMinusEventT0 */
  TEST_F(SVDVariableTest, SVDClusterTimeMinusEventT0)
  {
    // nullptr particle or out-of-range cluster -> NaN
    EXPECT_TRUE(std::isnan(Variable::SVDClusterTimeMinusEventT0(nullptr,        {0})));
    EXPECT_TRUE(std::isnan(Variable::SVDClusterTimeMinusEventT0(m_particles[0], {1})));

    // EventT0 not yet created -> NaN
    EXPECT_TRUE(std::isnan(Variable::SVDClusterTimeMinusEventT0(m_particles[0], {0})));

    // EventT0 created but no final T0 set -> NaN
    m_eventT0.create();
    EXPECT_TRUE(std::isnan(Variable::SVDClusterTimeMinusEventT0(m_particles[0], {0})));

    // EventT0 set -> clsTime - eventT0
    const double eventT0Value = 5.0;
    m_eventT0->setEventT0(eventT0Value, 1.0, Const::SVD);
    EXPECT_DOUBLE_EQ(Variable::SVDClusterTimeMinusEventT0(m_particles[0], {0}), defaultClsTime - eventT0Value);
  }

  /** Test SVDClusterTimeMinusCDCEventT0 */
  TEST_F(SVDVariableTest, SVDClusterTimeMinusCDCEventT0)
  {
    // nullptr particle or out-of-range cluster -> NaN
    EXPECT_TRUE(std::isnan(Variable::SVDClusterTimeMinusCDCEventT0(nullptr,        {0})));
    EXPECT_TRUE(std::isnan(Variable::SVDClusterTimeMinusCDCEventT0(m_particles[0], {1})));

    // EventT0 not yet created -> NaN
    EXPECT_TRUE(std::isnan(Variable::SVDClusterTimeMinusCDCEventT0(m_particles[0], {0})));

    // EventT0 created but no CDC temporary T0 -> NaN
    m_eventT0.create();
    EXPECT_TRUE(std::isnan(Variable::SVDClusterTimeMinusCDCEventT0(m_particles[0], {0})));

    // CDC temporary T0 added -> clsTime - cdcT0
    const double cdcT0Value = 8.0;
    m_eventT0->addTemporaryEventT0(EventT0::EventT0Component(cdcT0Value, 1.0, Const::CDC));
    EXPECT_DOUBLE_EQ(Variable::SVDClusterTimeMinusCDCEventT0(m_particles[0], {0}), defaultClsTime - cdcT0Value);
  }

  /** Test CDCEventT0 */
  TEST_F(SVDVariableTest, CDCEventT0)
  {
    // EventT0 not yet created -> NaN
    EXPECT_TRUE(std::isnan(Variable::CDCEventT0(m_particles[0], {})));

    // EventT0 created but no CDC temporary T0 -> NaN
    m_eventT0.create();
    EXPECT_TRUE(std::isnan(Variable::CDCEventT0(m_particles[0], {})));

    // CDC temporary T0 added -> correct value returned
    const double cdcT0Value = 12.0;
    m_eventT0->addTemporaryEventT0(EventT0::EventT0Component(cdcT0Value, 1.0, Const::CDC));
    EXPECT_DOUBLE_EQ(Variable::CDCEventT0(m_particles[0], {}), cdcT0Value);

    // Last entry wins when multiple are present
    const double cdcT0Value2 = 15.0;
    m_eventT0->addTemporaryEventT0(EventT0::EventT0Component(cdcT0Value2, 0.5, Const::CDC));
    EXPECT_DOUBLE_EQ(Variable::CDCEventT0(m_particles[0], {}), cdcT0Value2);
  }

  /** Test SVDEventT0 */
  TEST_F(SVDVariableTest, SVDEventT0)
  {
    // EventT0 not yet created -> NaN
    EXPECT_TRUE(std::isnan(Variable::SVDEventT0(m_particles[0], {})));

    // EventT0 created but no SVD temporary T0 -> NaN
    m_eventT0.create();
    EXPECT_TRUE(std::isnan(Variable::SVDEventT0(m_particles[0], {})));

    // SVD temporary T0 added -> correct value returned
    const double svdT0Value = 3.0;
    m_eventT0->addTemporaryEventT0(EventT0::EventT0Component(svdT0Value, 0.8, Const::SVD));
    EXPECT_DOUBLE_EQ(Variable::SVDEventT0(m_particles[0], {}), svdT0Value);

    // Last entry wins when multiple are present
    const double svdT0Value2 = 6.0;
    m_eventT0->addTemporaryEventT0(EventT0::EventT0Component(svdT0Value2, 0.4, Const::SVD));
    EXPECT_DOUBLE_EQ(Variable::SVDEventT0(m_particles[0], {}), svdT0Value2);
  }
} // namespace Belle2::SVD
