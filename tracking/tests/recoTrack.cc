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
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <framework/gearbox/Const.h>
#include <vector>


using namespace std;

namespace Belle2 {
  /** Test class for the RecoTrack object. */
  class RecoTrackTest : public ::testing::Test {
  protected:
    void SetUp() override
    {
      m_storeArrayNameOfRecoTracks = "ILoveRecoTracks";
      m_storeArrayNameOfCDCHits = "CDCHitsAreCool";
      m_storeArrayNameOfSVDHits = "WhatAboutSVD";
      m_storeArrayNameOfPXDHits = "PXDsILike";
      m_storeArrayNameOfHitInformation = "ConnectingAll";

      //--- Setup -----------------------------------------------------------------------
      DataStore::Instance().setInitializeActive(true);
      StoreArray<CDCHit>::registerPersistent(m_storeArrayNameOfCDCHits);
      StoreArray<SVDCluster>::registerPersistent(m_storeArrayNameOfSVDHits);
      StoreArray<PXDCluster>::registerPersistent(m_storeArrayNameOfPXDHits);
      StoreArray<RecoTrack>::registerPersistent(m_storeArrayNameOfRecoTracks);
      StoreArray<RecoHitInformation>::registerPersistent(m_storeArrayNameOfHitInformation);

      StoreArray<CDCHit> cdcHits(m_storeArrayNameOfCDCHits);
      StoreArray<SVDCluster> svdHits(m_storeArrayNameOfSVDHits);
      StoreArray<PXDCluster> pxdHits(m_storeArrayNameOfPXDHits);
      StoreArray<RecoTrack> recoTracks(m_storeArrayNameOfRecoTracks);
      StoreArray<RecoHitInformation> recoHitInformations(m_storeArrayNameOfHitInformation);

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
      m_recoTrack = recoTracks.appendNew(position, momentum, charge,
                                         m_storeArrayNameOfCDCHits, m_storeArrayNameOfSVDHits, m_storeArrayNameOfPXDHits, m_storeArrayNameOfHitInformation);
    }

    RecoTrack* m_recoTrack;
    std::string m_storeArrayNameOfRecoTracks;
    std::string m_storeArrayNameOfCDCHits;
    std::string m_storeArrayNameOfSVDHits;
    std::string m_storeArrayNameOfPXDHits;
    std::string m_storeArrayNameOfHitInformation;
  };

  /** Test simple Setters and Getters. */
  TEST_F(RecoTrackTest, cdcHit)
  {
    StoreArray<CDCHit> cdcHits(m_storeArrayNameOfCDCHits);

    EXPECT_FALSE(m_recoTrack->hasCDCHits());

    // Add three cdc hits to the track
    m_recoTrack->addCDCHit(cdcHits[0], 1);
    m_recoTrack->addCDCHit(cdcHits[1], 0, RecoTrack::RightLeftInformation::right);
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
    EXPECT_EQ(recoHitInformation->getSortingParameter(), 1);

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
    EXPECT_EQ(m_recoTrack->getSortingParameter(cdcHit), 1);

    EXPECT_NO_THROW(m_recoTrack->setFoundByTrackFinder(cdcHit, RecoTrack::OriginTrackFinder::SegmentTrackCombiner));
    EXPECT_NO_THROW(m_recoTrack->setRightLeftInformation(cdcHit, RecoTrack::RightLeftInformation::left));
    EXPECT_NO_THROW(m_recoTrack->setSortingParameter(cdcHit, 3));

    EXPECT_EQ(m_recoTrack->getFoundByTrackFinder(cdcHit), RecoTrack::OriginTrackFinder::SegmentTrackCombiner);
    EXPECT_EQ(m_recoTrack->getRightLeftInformation(cdcHit), RecoTrack::RightLeftInformation::left);
    EXPECT_EQ(m_recoTrack->getSortingParameter(cdcHit), 3);

    // with not added hits
    cdcHit = cdcHits[4];

    EXPECT_THROW(m_recoTrack->getTrackingDetector(cdcHit), std::invalid_argument);
    EXPECT_THROW(m_recoTrack->getRightLeftInformation(cdcHit), std::invalid_argument);
    EXPECT_THROW(m_recoTrack->getFoundByTrackFinder(cdcHit), std::invalid_argument);
    EXPECT_THROW(m_recoTrack->getSortingParameter(cdcHit), std::invalid_argument);

    EXPECT_THROW(m_recoTrack->setFoundByTrackFinder(cdcHit, RecoTrack::OriginTrackFinder::SegmentTrackCombiner), std::invalid_argument);
    EXPECT_THROW(m_recoTrack->setRightLeftInformation(cdcHit, RecoTrack::RightLeftInformation::left), std::invalid_argument);
  }

  TEST_F(RecoTrackTest, testGenfitConversionOne)
  {
    // Create a genfit track cand
    genfit::TrackCand* newCreatedTrackCand = new genfit::TrackCand();
    TVector3 position(4, 23, 5.6);
    TVector3 momentum(4, 23, 5.6);
    short int charge = 1;
    // We can not add these parameters immediately - we hve to convert them to the perigee parameters
    newCreatedTrackCand->setPosMomSeed(position, momentum, charge);
    newCreatedTrackCand->addHit(Const::CDC, 0, -1, 0);
    newCreatedTrackCand->addHit(Const::CDC, 1, -1, 1);
    newCreatedTrackCand->addHit(Const::CDC, 2, -1, 2);

    // convert it to a RecoTrack
    RecoTrack* recoTrackFromGenfit = RecoTrack::createFromTrackCand(newCreatedTrackCand, m_storeArrayNameOfRecoTracks,
                                     m_storeArrayNameOfCDCHits, m_storeArrayNameOfSVDHits,
                                     m_storeArrayNameOfPXDHits, m_storeArrayNameOfHitInformation);

    // convert it back

    genfit::TrackCand* exportedTrackCand = recoTrackFromGenfit->createGenfitTrackCand();

    // Expect equal
    ASSERT_EQ(exportedTrackCand->getNHits(), newCreatedTrackCand->getNHits());
    EXPECT_NEAR(exportedTrackCand->getPosSeed().X(), newCreatedTrackCand->getPosSeed().X(), 1E-10);
    EXPECT_NEAR(exportedTrackCand->getPosSeed().Y(), newCreatedTrackCand->getPosSeed().Y(), 1E-10);
    EXPECT_NEAR(exportedTrackCand->getPosSeed().Z(), newCreatedTrackCand->getPosSeed().Z(), 1E-10);
    EXPECT_NEAR(exportedTrackCand->getMomSeed().X(), newCreatedTrackCand->getMomSeed().X(), 1E-10);
    EXPECT_NEAR(exportedTrackCand->getMomSeed().Y(), newCreatedTrackCand->getMomSeed().Y(), 1E-10);
    EXPECT_NEAR(exportedTrackCand->getMomSeed().Z(), newCreatedTrackCand->getMomSeed().Z(), 1E-10);
    EXPECT_EQ(exportedTrackCand->getChargeSeed(), newCreatedTrackCand->getChargeSeed());
    EXPECT_EQ(exportedTrackCand->getHit(0)->getHitId(), newCreatedTrackCand->getHit(0)->getHitId());
    EXPECT_EQ(exportedTrackCand->getHit(1)->getSortingParameter(), newCreatedTrackCand->getHit(1)->getSortingParameter());
    EXPECT_EQ(exportedTrackCand->getHit(2)->getHitId(), newCreatedTrackCand->getHit(2)->getHitId());
  }

  TEST_F(RecoTrackTest, testGenfitConversionTwo)
  {
    EXPECT_FALSE(m_recoTrack->hasCDCHits());
    StoreArray<CDCHit> cdcHits(m_storeArrayNameOfCDCHits);

    // Add three cdc hits to the track
    m_recoTrack->addCDCHit(cdcHits[0], 1);
    m_recoTrack->addCDCHit(cdcHits[1], 0, RecoTrack::RightLeftInformation::right);
    m_recoTrack->addCDCHit(cdcHits[2], 2);

    EXPECT_TRUE(m_recoTrack->hasCDCHits());

    genfit::TrackCand* exportedTrackCand = m_recoTrack->createGenfitTrackCand();

    ASSERT_EQ(exportedTrackCand->getNHits(), m_recoTrack->getNumberOfTotalHits());
    ASSERT_EQ(m_recoTrack->getNumberOfTotalHits(), 3);

    RecoTrack* recoTrackFromGenfit = RecoTrack::createFromTrackCand(exportedTrackCand, m_storeArrayNameOfRecoTracks,
                                     m_storeArrayNameOfCDCHits, m_storeArrayNameOfSVDHits,
                                     m_storeArrayNameOfPXDHits, m_storeArrayNameOfHitInformation);

    ASSERT_EQ(recoTrackFromGenfit->getNumberOfCDCHits(), m_recoTrack->getNumberOfCDCHits());
    const auto& cdcHitListOne = recoTrackFromGenfit->getCDCHitList();
    const auto& cdcHitListTwo = m_recoTrack->getCDCHitList();
    ASSERT_EQ(cdcHitListOne.size(), 3);
    ASSERT_EQ(cdcHitListTwo.size(), 3);
  }
}
