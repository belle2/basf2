/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/StoreArray.h>
#include <framework/geometry/VectorUtil.h>

#include <framework/utilities/TestHelpers.h>
#include <framework/gearbox/Const.h>
#include <vector>
#include <genfit/WireTrackCandHit.h>

using namespace std;

namespace Belle2 {
  /** Test class for the RecoTrack object. */
  class RecoTrackTest : public ::testing::Test {
  protected:
    /// Setup a "working environment" with store arrays for the hits and the correct relations.
    void SetUp() override
    {
      /// Name of the RecoTrack store array.
      m_storeArrayNameOfRecoTracks = "ILoveRecoTracks";
      /// Name of the PXD hits store array.
      m_storeArrayNameOfPXDHits = "PXDsILike";
      /// Name of the SVD hits store array.
      m_storeArrayNameOfSVDHits = "WhatAboutSVD";
      /// Name of the CDC hits store array.
      m_storeArrayNameOfCDCHits = "CDCHitsAreCool";
      /// Name of the BKLM hits store array.
      m_storeArrayNameOfBKLMHits = "KeepBKLMsAlive";
      /// Name of the EKLM hits store array.
      m_storeArrayNameOfEKLMHits = "EKLMsAreImportant";
      /// Name of the reco hit information store array.
      m_storeArrayNameOfHitInformation = "ConnectingAll";

      //--- Setup -----------------------------------------------------------------------
      // We do not use the KLM store arrays to test, if the RecoTrack can be used without them.
      DataStore::Instance().setInitializeActive(true);
      StoreArray<PXDCluster> pxdHits(m_storeArrayNameOfPXDHits);
      pxdHits.registerInDataStore();
      StoreArray<SVDCluster> svdHits(m_storeArrayNameOfSVDHits);
      svdHits.registerInDataStore();
      StoreArray<CDCHit> cdcHits(m_storeArrayNameOfCDCHits);
      cdcHits.registerInDataStore();
      StoreArray<RecoTrack> recoTracks(m_storeArrayNameOfRecoTracks);
      recoTracks.registerInDataStore();
      StoreArray<RecoHitInformation> recoHitInformations(m_storeArrayNameOfHitInformation);
      recoHitInformations.registerInDataStore();

      pxdHits.registerRelationTo(recoTracks);
      svdHits.registerRelationTo(recoTracks);
      cdcHits.registerRelationTo(recoTracks);
      recoHitInformations.registerRelationTo(pxdHits);
      recoHitInformations.registerRelationTo(svdHits);
      recoHitInformations.registerRelationTo(cdcHits);

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
      ROOT::Math::XYZVector position(0, 1, 2);
      ROOT::Math::XYZVector momentum(-1, -0.5, 1.123);
      short int charge = 1;
      m_recoTrack = recoTracks.appendNew(position, momentum, charge,
                                         m_storeArrayNameOfPXDHits, m_storeArrayNameOfSVDHits, m_storeArrayNameOfCDCHits,
                                         m_storeArrayNameOfBKLMHits, m_storeArrayNameOfEKLMHits, m_storeArrayNameOfHitInformation);
      m_recoTrack2 = recoTracks.appendNew(position, momentum, charge,
                                          m_storeArrayNameOfPXDHits, m_storeArrayNameOfSVDHits, m_storeArrayNameOfCDCHits,
                                          m_storeArrayNameOfBKLMHits, m_storeArrayNameOfEKLMHits, m_storeArrayNameOfHitInformation);
    }

    RecoTrack* m_recoTrack; /**< pointer to recoTrack */
    RecoTrack* m_recoTrack2; /**< pointer to other recoTrack */
    std::string m_storeArrayNameOfRecoTracks; /**< name of recoTracks storeArray */
    std::string m_storeArrayNameOfPXDHits; /**< name of storeArray with PXD hits */
    std::string m_storeArrayNameOfSVDHits; /**< name of storeArray with SVD hits */
    std::string m_storeArrayNameOfCDCHits; /**< name of storeArray with CDC hits */
    std::string m_storeArrayNameOfBKLMHits; /**< name of storeArray with BKLM hits */
    std::string m_storeArrayNameOfEKLMHits; /**< name of storeArray with EKLM hits */
    std::string m_storeArrayNameOfHitInformation; /**< name of storeArray with hit information */
  };

  /** Test simple Setters and Getters. */
  // cppcheck-suppress syntaxError
  TEST_F(RecoTrackTest, cdcHit)
  {
    StoreArray<CDCHit> cdcHits(m_storeArrayNameOfCDCHits);

    EXPECT_FALSE(m_recoTrack->hasCDCHits());

    // Add three cdc hits to the track
    m_recoTrack->addCDCHit(cdcHits[0], 1);
    m_recoTrack->addCDCHit(cdcHits[1], 0, RecoHitInformation::RightLeftInformation::c_right);
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
    EXPECT_EQ(recoHitInformation->getTrackingDetector(), RecoHitInformation::RecoHitDetector::c_CDC);
    EXPECT_EQ(recoHitInformation->getRightLeftInformation(), RecoHitInformation::RightLeftInformation::c_undefinedRightLeftInformation);
    EXPECT_EQ(recoHitInformation->getFoundByTrackFinder(), RecoHitInformation::OriginTrackFinder::c_undefinedTrackFinder);
    EXPECT_EQ(recoHitInformation->getSortingParameter(), 1);

    cdcHit = cdcHits[1];
    recoHitInformation = m_recoTrack->getRecoHitInformation(cdcHit);
    EXPECT_NE(recoHitInformation, nullptr);
    EXPECT_EQ(recoHitInformation->getRightLeftInformation(), RecoHitInformation::RightLeftInformation::c_right);



    // Setter and getter for the reco hit information
    // with added hits
    cdcHit = cdcHits[0];

    EXPECT_EQ(m_recoTrack->getTrackingDetector(cdcHit), RecoHitInformation::RecoHitDetector::c_CDC);
    EXPECT_EQ(m_recoTrack->getRightLeftInformation(cdcHit), RecoHitInformation::RightLeftInformation::c_undefinedRightLeftInformation);
    EXPECT_EQ(m_recoTrack->getFoundByTrackFinder(cdcHit), RecoHitInformation::OriginTrackFinder::c_undefinedTrackFinder);
    EXPECT_EQ(m_recoTrack->getSortingParameter(cdcHit), 1);

    EXPECT_NO_THROW(m_recoTrack->setFoundByTrackFinder(cdcHit, RecoHitInformation::OriginTrackFinder::c_SegmentTrackCombiner));
    EXPECT_NO_THROW(m_recoTrack->setRightLeftInformation(cdcHit, RecoHitInformation::RightLeftInformation::c_left));
    EXPECT_NO_THROW(m_recoTrack->setSortingParameter(cdcHit, 3));

    EXPECT_EQ(m_recoTrack->getFoundByTrackFinder(cdcHit), RecoHitInformation::OriginTrackFinder::c_SegmentTrackCombiner);
    EXPECT_EQ(m_recoTrack->getRightLeftInformation(cdcHit), RecoHitInformation::RightLeftInformation::c_left);
    EXPECT_EQ(m_recoTrack->getSortingParameter(cdcHit), 3);

    // with not added hits
    cdcHit = cdcHits[4];

    EXPECT_B2FATAL(m_recoTrack->getTrackingDetector(cdcHit));
    EXPECT_B2FATAL(m_recoTrack->getRightLeftInformation(cdcHit));
    EXPECT_B2FATAL(m_recoTrack->getFoundByTrackFinder(cdcHit));
    EXPECT_B2FATAL(m_recoTrack->getSortingParameter(cdcHit));

    EXPECT_B2FATAL(m_recoTrack->setFoundByTrackFinder(cdcHit, RecoHitInformation::OriginTrackFinder::c_SegmentTrackCombiner));
    EXPECT_B2FATAL(m_recoTrack->setRightLeftInformation(cdcHit, RecoHitInformation::RightLeftInformation::c_left));
  }

  /** Test simple Correct handling fo the MCFinder hit classification */
  TEST_F(RecoTrackTest, cdcHitMCFinderCategory)
  {
    StoreArray<CDCHit> cdcHits(m_storeArrayNameOfCDCHits);

    EXPECT_FALSE(m_recoTrack->hasCDCHits());

    // Add three cdc hits to the track
    m_recoTrack->addCDCHit(cdcHits[0], 1, RecoHitInformation::RightLeftInformation::c_right,
                           RecoHitInformation::OriginTrackFinder::c_MCTrackFinderPriorityHit);
    m_recoTrack->addCDCHit(cdcHits[1], 0, RecoHitInformation::RightLeftInformation::c_right,
                           RecoHitInformation::OriginTrackFinder::c_MCTrackFinderAuxiliaryHit);
    // the mcfinder prorperty of this hit is not provided explicitly and therefore should be set to undefined
    m_recoTrack->addCDCHit(cdcHits[2], 2);

    // get the RecoHitInfo and check their category
    EXPECT_EQ(m_recoTrack->getRecoHitInformation(cdcHits[0])->getFoundByTrackFinder(),
              RecoHitInformation::OriginTrackFinder::c_MCTrackFinderPriorityHit);
    EXPECT_EQ(m_recoTrack->getRecoHitInformation(cdcHits[1])->getFoundByTrackFinder(),
              RecoHitInformation::OriginTrackFinder::c_MCTrackFinderAuxiliaryHit);
    EXPECT_EQ(m_recoTrack->getRecoHitInformation(cdcHits[2])->getFoundByTrackFinder(),
              RecoHitInformation::OriginTrackFinder::c_undefinedTrackFinder);
  }

  /** Test conversion to genfit track cands. */
  TEST_F(RecoTrackTest, testGenfitConversionOne)
  {
    // Create a genfit track cand
    genfit::TrackCand newCreatedTrackCand;
    ROOT::Math::XYZVector position(4, 23, 5.6);
    ROOT::Math::XYZVector momentum(4, 23, 5.6);
    short int charge = 1;
    // We can not add these parameters immediately - we hve to convert them to the perigee parameters
    newCreatedTrackCand.setPosMomSeed(XYZToTVector(position), XYZToTVector(momentum), charge);
    newCreatedTrackCand.addHit(new genfit::WireTrackCandHit(Const::CDC, 0, -1, 0, 0));
    newCreatedTrackCand.addHit(new genfit::WireTrackCandHit(Const::CDC, 1, -1, 1, 0));
    newCreatedTrackCand.addHit(new genfit::WireTrackCandHit(Const::CDC, 2, -1, 2, 0));

    // convert it to a RecoTrack
    RecoTrack* recoTrackFromGenfit = RecoTrack::createFromTrackCand(newCreatedTrackCand, m_storeArrayNameOfRecoTracks,
                                     m_storeArrayNameOfPXDHits, m_storeArrayNameOfSVDHits, m_storeArrayNameOfCDCHits,
                                     m_storeArrayNameOfBKLMHits, m_storeArrayNameOfEKLMHits,
                                     m_storeArrayNameOfHitInformation);

    // convert it back

    const genfit::TrackCand& exportedTrackCand = recoTrackFromGenfit->createGenfitTrackCand();

    // Expect equal
    ASSERT_EQ(exportedTrackCand.getNHits(), newCreatedTrackCand.getNHits());
    EXPECT_NEAR(exportedTrackCand.getPosSeed().X(), newCreatedTrackCand.getPosSeed().X(), 1E-10);
    EXPECT_NEAR(exportedTrackCand.getPosSeed().Y(), newCreatedTrackCand.getPosSeed().Y(), 1E-10);
    EXPECT_NEAR(exportedTrackCand.getPosSeed().Z(), newCreatedTrackCand.getPosSeed().Z(), 1E-10);
    EXPECT_NEAR(exportedTrackCand.getMomSeed().X(), newCreatedTrackCand.getMomSeed().X(), 1E-10);
    EXPECT_NEAR(exportedTrackCand.getMomSeed().Y(), newCreatedTrackCand.getMomSeed().Y(), 1E-10);
    EXPECT_NEAR(exportedTrackCand.getMomSeed().Z(), newCreatedTrackCand.getMomSeed().Z(), 1E-10);
    EXPECT_EQ(exportedTrackCand.getChargeSeed(), newCreatedTrackCand.getChargeSeed());
    EXPECT_EQ(exportedTrackCand.getHit(0)->getHitId(), newCreatedTrackCand.getHit(0)->getHitId());
    EXPECT_EQ(exportedTrackCand.getHit(1)->getSortingParameter(), newCreatedTrackCand.getHit(1)->getSortingParameter());
    EXPECT_EQ(exportedTrackCand.getHit(2)->getHitId(), newCreatedTrackCand.getHit(2)->getHitId());
  }

  /** Test conversion from genfit track cands. */
  TEST_F(RecoTrackTest, testGenfitConversionTwo)
  {
    EXPECT_FALSE(m_recoTrack->hasCDCHits());
    StoreArray<CDCHit> cdcHits(m_storeArrayNameOfCDCHits);

    // Add three cdc hits to the track
    m_recoTrack->addCDCHit(cdcHits[0], 1);
    m_recoTrack->addCDCHit(cdcHits[1], 0, RecoHitInformation::RightLeftInformation::c_right);
    m_recoTrack->addCDCHit(cdcHits[2], 2);

    EXPECT_TRUE(m_recoTrack->hasCDCHits());

    const genfit::TrackCand& exportedTrackCand = m_recoTrack->createGenfitTrackCand();

    ASSERT_EQ(exportedTrackCand.getNHits(), m_recoTrack->getNumberOfTotalHits());
    ASSERT_EQ(m_recoTrack->getNumberOfTotalHits(), 3);

    RecoTrack* recoTrackFromGenfit = RecoTrack::createFromTrackCand(exportedTrackCand, m_storeArrayNameOfRecoTracks,
                                     m_storeArrayNameOfPXDHits, m_storeArrayNameOfSVDHits, m_storeArrayNameOfCDCHits,
                                     m_storeArrayNameOfBKLMHits, m_storeArrayNameOfEKLMHits,
                                     m_storeArrayNameOfHitInformation);

    B2INFO("kjh");

    ASSERT_EQ(recoTrackFromGenfit->getNumberOfCDCHits(), m_recoTrack->getNumberOfCDCHits());
    const auto& cdcHitListOne = recoTrackFromGenfit->getCDCHitList();
    const auto& cdcHitListTwo = m_recoTrack->getCDCHitList();
    ASSERT_EQ(cdcHitListOne.size(), 3);
    ASSERT_EQ(cdcHitListTwo.size(), 3);
  }

  /** Test copying a RecoTrack. */
  TEST_F(RecoTrackTest, copyRecoTrack)
  {
    StoreArray<CDCHit> cdcHits(m_storeArrayNameOfCDCHits);

    // Add three cdc hits to the track
    m_recoTrack->addCDCHit(cdcHits[0], 1);
    m_recoTrack->addCDCHit(cdcHits[1], 0, RecoHitInformation::RightLeftInformation::c_right);
    m_recoTrack->addCDCHit(cdcHits[2], 2);

    // create a second RecoTrack
    StoreArray<RecoTrack> recoTracks(m_storeArrayNameOfRecoTracks);

    auto recoTrack = recoTracks.appendNew(m_recoTrack->getPositionSeed(), m_recoTrack->getMomentumSeed(), m_recoTrack->getChargeSeed(),
                                          m_storeArrayNameOfPXDHits, m_storeArrayNameOfSVDHits, m_storeArrayNameOfCDCHits,
                                          m_storeArrayNameOfBKLMHits, m_storeArrayNameOfEKLMHits,
                                          m_storeArrayNameOfHitInformation);
    EXPECT_FALSE(recoTrack->hasCDCHits());

    // check if the offset computation of the hit order works as expected
    size_t offset = 3;
    recoTrack->addHitsFromRecoTrack(m_recoTrack, offset);
    ASSERT_EQ(recoTrack->getNumberOfCDCHits(), 3);

    size_t this_i = offset;
    for (auto pHit : recoTrack->getSortedCDCHitList()) {
      auto sortParam = recoTrack->getSortingParameter(pHit);
      ASSERT_EQ(this_i, sortParam);
      this_i++;
    }
  }

  /** Test the getRecoHitInformations() function */
  TEST_F(RecoTrackTest, recoHitInformations)
  {
    StoreArray<CDCHit> cdcHits(m_storeArrayNameOfCDCHits);

    m_recoTrack->addCDCHit(cdcHits[0], 1);

    // create a second RecoTrack
    StoreArray<RecoTrack> recoTracks(m_storeArrayNameOfRecoTracks);

    RecoTrack* recoTrack2 = recoTracks.appendNew(m_recoTrack->getPositionSeed(), m_recoTrack->getMomentumSeed(),
                                                 m_recoTrack->getChargeSeed(),
                                                 m_storeArrayNameOfPXDHits, m_storeArrayNameOfSVDHits, m_storeArrayNameOfCDCHits,
                                                 m_storeArrayNameOfBKLMHits, m_storeArrayNameOfEKLMHits,
                                                 m_storeArrayNameOfHitInformation);
    recoTrack2->addCDCHit(cdcHits[1], 2);

    ASSERT_EQ(m_recoTrack->getRecoHitInformations().size(), 1);
    ASSERT_EQ(m_recoTrack->getRecoHitInformations()[0]->getSortingParameter(), 1);
    ASSERT_EQ(recoTrack2->getRecoHitInformations().size(), 1);
    ASSERT_EQ(recoTrack2->getRecoHitInformations()[0]->getSortingParameter(), 2);
  }

  /** Test getOutgoingArmTime() and getIngoingArmTime() functions */
  TEST_F(RecoTrackTest, trackTime)
  {
    StoreArray<SVDCluster> svdHits(m_storeArrayNameOfSVDHits);
    StoreArray<CDCHit> cdcHits(m_storeArrayNameOfCDCHits);

    //"SVDCluster(VxdID, isU, position, positionErr, clsTime, clsTimeErr, clsCharge, seedCharge, clsSize, clsSNR, clsChi2, FF)"
    svdHits.appendNew(Belle2::VxdID("3.1.2"), true, 1.0, 0.01, 0.1, 0.1, 5.0e4, 5.0e4, 1, 50.0, 0.1, 0);
    svdHits.appendNew(Belle2::VxdID("4.1.2"), true, 1.0, 0.01, 0.2, 0.1, 5.0e4, 5.0e4, 1, 50.0, 0.1, 0);
    svdHits.appendNew(Belle2::VxdID("5.1.3"), true, 1.0, 0.01, 0.3, 0.1, 5.0e4, 5.0e4, 1, 50.0, 0.1, 0);
    svdHits.appendNew(Belle2::VxdID("6.1.3"), true, 1.0, 0.01, 0.4, 0.1, 5.0e4, 5.0e4, 1, 50.0, 0.1, 0);
    svdHits.appendNew(Belle2::VxdID("6.7.7"), true, 1.0, 0.01, 1.0, 0.1, 5.0e4, 5.0e4, 1, 50.0, 0.1, 0);
    svdHits.appendNew(Belle2::VxdID("5.7.4"), true, 1.0, 0.01, 1.1, 0.1, 5.0e4, 5.0e4, 1, 50.0, 0.1, 0);

    ROOT::Math::XYZVector momentum(0.25, 0.25, 0.05);

    StoreArray<RecoTrack> recoTracks(m_storeArrayNameOfRecoTracks);
    RecoTrack* recoTrack = recoTracks.appendNew(m_recoTrack->getPositionSeed(), momentum,
                                                m_recoTrack->getChargeSeed(),
                                                m_storeArrayNameOfPXDHits, m_storeArrayNameOfSVDHits, m_storeArrayNameOfCDCHits,
                                                m_storeArrayNameOfBKLMHits, m_storeArrayNameOfEKLMHits,
                                                m_storeArrayNameOfHitInformation);

    // SVD Hits
    recoTrack->addSVDHit(svdHits[0], 0);
    recoTrack->addSVDHit(svdHits[1], 1);
    recoTrack->addSVDHit(svdHits[2], 2);
    recoTrack->addSVDHit(svdHits[3], 3);

    // CDC Hits
    recoTrack->addCDCHit(cdcHits[0], 4);
    recoTrack->addCDCHit(cdcHits[1], 5);

    // SVD Hits
    recoTrack->addSVDHit(svdHits[4], 6);
    recoTrack->addSVDHit(svdHits[5], 7);

    EXPECT_TRUE(recoTrack->hasSVDHits());

    EXPECT_FALSE(recoTrack->hasOutgoingArmTime());
    EXPECT_FALSE(recoTrack->hasIngoingArmTime());

    B2INFO("outgoing arm time: " << recoTrack->getOutgoingArmTime());
    B2INFO("ingoing arm time:  " << recoTrack->getIngoingArmTime());
    B2INFO("difference: " << recoTrack->getInOutArmTimeDifference());
    EXPECT_NEAR(recoTrack->getIngoingArmTime(), recoTrack->getOutgoingArmTime(), 0.8);
    EXPECT_NEAR(recoTrack->getIngoingArmTime(), 1.05, 1E-5);

    EXPECT_TRUE(recoTrack->hasOutgoingArmTime());
    EXPECT_TRUE(recoTrack->hasIngoingArmTime());
  }
}
