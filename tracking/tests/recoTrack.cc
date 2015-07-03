/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/StoreArray.h>

#include <framework/utilities/TestHelpers.h>

#include <vector>


using namespace std;

namespace Belle2 {
  /** Test class for the RecoTrack object. */
  class RecoTrackTest : public Belle2::TestHelpers::TestWithGearbox {
  protected:
    void SetUp() override
    {
      Belle2::TestHelpers::TestWithGearbox::SetUp();

      //--- Setup -----------------------------------------------------------------------
      DataStore::Instance().setInitializeActive(true);
      StoreArray<CDCHit>::registerPersistent("CDCHitsAreCoolVector");
      StoreArray<SVDCluster>::registerPersistent("SVDHitsAreCoolVector");
      StoreArray<PXDCluster>::registerPersistent("PXDHitsAreCoolVector");
      StoreArray<RecoTrack>::registerPersistent("ILoveRecoTracks");
      StoreArray<RecoHitInformation>::registerPersistent("ConnectingTwoWorlds");

      StoreArray<CDCHit> cdcHits("CDCHitsAreCoolVector");
      StoreArray<SVDCluster> svdHits("SVDHitsAreCoolVector");
      StoreArray<PXDCluster> pxdHits("PXDHitsAreCoolVector");
      StoreArray<RecoTrack> recoTracks("ILoveRecoTracks");
      StoreArray<RecoHitInformation> recoHitInformations("ConnectingTwoWorlds");

      cdcHits.registerRelationTo(recoTracks);
      svdHits.registerRelationTo(recoTracks);
      pxdHits.registerRelationTo(recoTracks);
      recoHitInformations.registerRelationTo(cdcHits);
      recoHitInformations.registerRelationTo(svdHits);
      recoHitInformations.registerRelationTo(pxdHits);

      recoTracks.registerRelationTo(recoHitInformations);

      //"CDCHit(tdcCount, adcCount, superLayer, layer, wire)"
      //Indices range from 0-7
      cdcHits.appendNew(100, 100, 0, 0, 0);
      cdcHits.appendNew(100, 100, 2, 0, 0);
      cdcHits.appendNew(100, 100, 4, 0, 0);
      cdcHits.appendNew(100, 100, 6, 0, 0);
      cdcHits.appendNew(100, 100, 8, 0, 0);
      cdcHits.appendNew(100, 100, 1, 1, 0);
      cdcHits.appendNew(100, 100, 3, 0, 0);
      cdcHits.appendNew(100, 100, 5, 0, 0);

      // We add some hits to the track. Then we assure they were added properly and the hit information objects are correct.
      TVector3 position(0, 1, 2);
      TVector3 momentum(-1, -0.5, 1.123);
      short int charge = 1;
      double bZ = 1.5;
      m_recoTrack = recoTracks.appendNew(position, momentum, charge, bZ,
                                         "CDCHitsAreCoolVector", "SVDHitsAreCoolVector", "PXDHitsAreCoolVector", "ConnectingTwoWorlds");
    }

    RecoTrack* m_recoTrack;
  };

  /** Test simple Setters and Getters. */
  TEST_F(RecoTrackTest, cdcHit)
  {
    StoreArray<CDCHit> cdcHits("CDCHitsAreCoolVector");

    EXPECT_FALSE(m_recoTrack->hasCDCHits());

    // Add three cdc hits to the track
    m_recoTrack->addCDCHit(cdcHits[0], 0.5);
    m_recoTrack->addCDCHit(cdcHits[1], 0.2, RecoTrack::RightLeftInformation::right);
    m_recoTrack->addCDCHit(cdcHits[2], 2);

    EXPECT_TRUE(m_recoTrack->hasCDCHits());

    EXPECT_TRUE(m_recoTrack->hasHit(cdcHits[0]));
    EXPECT_TRUE(m_recoTrack->hasHit(cdcHits[1]));
    EXPECT_TRUE(m_recoTrack->hasHit(cdcHits[2]));

    EXPECT_FALSE(m_recoTrack->hasHit(cdcHits[4]));

    ASSERT_EQ(m_recoTrack->getNumberOfCDCHits(), 3);
    const std::vector<CDCHit*> addedCDCHits = m_recoTrack->getCDCHitList();
    ASSERT_EQ(addedCDCHits.size(), 3);
    EXPECT_EQ(addedCDCHits[0], cdcHits[0]);
    EXPECT_EQ(addedCDCHits[1], cdcHits[1]);
    EXPECT_EQ(addedCDCHits[2], cdcHits[2]);

    const std::vector<CDCHit*> sortedCDCHits = m_recoTrack->getSortedCDCHitList();
    ASSERT_EQ(sortedCDCHits.size(), 3);
    EXPECT_EQ(sortedCDCHits[0], cdcHits[1]);
    EXPECT_EQ(sortedCDCHits[1], cdcHits[0]);
    EXPECT_EQ(sortedCDCHits[2], cdcHits[2]);

    // Individual hit information
    CDCHit* cdcHit = cdcHits[0];

    RecoHitInformation* recoHitInformation = m_recoTrack->getRecoHitInformation(cdcHit);
    EXPECT_NE(recoHitInformation, nullptr);
    EXPECT_EQ(recoHitInformation->getTrackingDetector(), RecoTrack::TrackingDetector::CDC);
    EXPECT_EQ(recoHitInformation->getRightLeftInformation(), RecoTrack::RightLeftInformation::undefinedRightLeftInformation);
    EXPECT_EQ(recoHitInformation->getFoundByTrackFinder(), RecoTrack::OriginTrackFinder::undefinedTrackFinder);
    EXPECT_EQ(recoHitInformation->getReconstructedArcLength(), 0.5);

    cdcHit = cdcHits[1];
    recoHitInformation = m_recoTrack->getRecoHitInformation(cdcHit);
    EXPECT_NE(recoHitInformation, nullptr);
    EXPECT_EQ(recoHitInformation->getRightLeftInformation(), RecoTrack::RightLeftInformation::right);



    // Setter and getter for the reco hit information
    // with added hits
    cdcHit = cdcHits[0];

    EXPECT_EQ(m_recoTrack->getTrackingDetector(cdcHit), RecoTrack::TrackingDetector::CDC);
    EXPECT_EQ(m_recoTrack->getRightLeftInformation(cdcHit), RecoTrack::RightLeftInformation::undefinedRightLeftInformation);
    EXPECT_EQ(m_recoTrack->getFoundByTrackFinder(cdcHit), RecoTrack::OriginTrackFinder::undefinedTrackFinder);
    EXPECT_EQ(m_recoTrack->getReconstructedArcLength(cdcHit), 0.5);

    EXPECT_TRUE(m_recoTrack->setFoundByTrackFinder(cdcHit, RecoTrack::OriginTrackFinder::SegmentTrackCombiner));
    EXPECT_TRUE(m_recoTrack->setRightLeftInformation(cdcHit, RecoTrack::RightLeftInformation::left));

    EXPECT_EQ(m_recoTrack->getFoundByTrackFinder(cdcHit), RecoTrack::OriginTrackFinder::SegmentTrackCombiner);
    EXPECT_EQ(m_recoTrack->getRightLeftInformation(cdcHit), RecoTrack::RightLeftInformation::left);

    // with not added hits
    cdcHit = cdcHits[4];

    EXPECT_EQ(m_recoTrack->getTrackingDetector(cdcHit), RecoTrack::TrackingDetector::invalidTrackingDetector);
    EXPECT_EQ(m_recoTrack->getRightLeftInformation(cdcHit), RecoTrack::RightLeftInformation::invalidRightLeftInformation);
    EXPECT_EQ(m_recoTrack->getFoundByTrackFinder(cdcHit), RecoTrack::OriginTrackFinder::invalidTrackFinder);
    EXPECT_TRUE(std::isnan(m_recoTrack->getReconstructedArcLength(cdcHit)));
    // TODO FIXME
    //EXPECT_EQ(recoTrack.getReconstructedPosition(cdcHit).Mag(), 0);

    EXPECT_FALSE(m_recoTrack->setFoundByTrackFinder(cdcHit, RecoTrack::OriginTrackFinder::SegmentTrackCombiner));
    EXPECT_FALSE(m_recoTrack->setRightLeftInformation(cdcHit, RecoTrack::RightLeftInformation::left));


    // Test the hit patterns

    //Fill the arm into the hit pattern, that has actually hits.
    const HitPatternCDC& hitPatternFull = m_recoTrack->getHitPatternCDC(1);
    EXPECT_EQ(hitPatternFull.hasAxialLayer(),  true);
    EXPECT_EQ(hitPatternFull.hasStereoLayer(), false);

    //Now I fill the arm into the pattern, that doesn't have any hits.
    const HitPatternCDC& hitPatternEmpty = m_recoTrack->getHitPatternCDC(-1);
    EXPECT_EQ(hitPatternEmpty.hasAxialLayer(),  false);
    EXPECT_EQ(hitPatternEmpty.hasStereoLayer(), false);
  }
}
