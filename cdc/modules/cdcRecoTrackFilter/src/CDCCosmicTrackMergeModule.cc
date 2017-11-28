/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:    Dong Van Thanh                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcRecoTrackFilter/CDCCosmicTrackMergeModule.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/StoreArray.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <eklm/dataobjects/EKLMHit2d.h>

using namespace Belle2;
using namespace CDC;

REG_MODULE(CDCCosmicTrackMerger);

CDCCosmicTrackMergerModule::CDCCosmicTrackMergerModule() : Module()
{
  setDescription("Select cosmic events containing two tracks (up/down) and merge two tracks"
                 "Old reco tracks store array will be deleted afterwards, if the parameter is set to do so.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName,
           "StoreArray containing the RecoTracks to read from and delete afterwards.",
           m_param_recoTracksStoreArrayName);
  addParam("MergedRecoTracksStoreArrayName", m_param_MergedRecoTracksStoreArrayName,
           "StoreArray to where to copy the merged RecoTrack.",
           m_param_MergedRecoTracksStoreArrayName);
  addParam("deleteOtherRecoTracks", m_param_deleteOtherRecoTracks,
           "Flag to delete the not Merged RecoTracks from the input StoreArray.",
           m_param_deleteOtherRecoTracks);
  addParam("MinimumNumHitCut", m_MinimumNumHitCut, "Number of hit per track required for each track", m_MinimumNumHitCut);
}

void CDCCosmicTrackMergerModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  StoreArray<RecoTrack> MergedRecoTracks(m_param_MergedRecoTracksStoreArrayName);
  MergedRecoTracks.registerInDataStore();

  RecoTrack::registerRequiredRelations(MergedRecoTracks);
}

void CDCCosmicTrackMergerModule::event()
{
  StoreArray<RecoTrack> recoTrackStoreArray(m_param_recoTracksStoreArrayName);
  StoreArray<RecoTrack> MergedRecoTracks(m_param_MergedRecoTracksStoreArrayName);
  // static CDCGeometryPar& cdcgeo = CDC::CDCGeometryPar::Instance();

  if (recoTrackStoreArray.getEntries() == 2) {
    if (recoTrackStoreArray[0]->getNumberOfCDCHits() > m_MinimumNumHitCut
        && recoTrackStoreArray[1]->getNumberOfCDCHits() > m_MinimumNumHitCut) {
      // if(recoTrackStoreArray[0].getPositionSeed().Y() * recoTrackStoreArray[1].getPositionSeed().Y() >0) continue;

      std::vector<RecoTrack*> recoTracks;
      recoTracks.reserve(static_cast<unsigned int>(recoTrackStoreArray.getEntries()));

      for (RecoTrack& recoTrack : recoTrackStoreArray) {
        recoTracks.push_back(&recoTrack);
      }

      std::function < bool (RecoTrack*, RecoTrack*)> lmdSort = [](RecoTrack * lhs, RecoTrack * rhs) {
        float minimumTimeFirstTrack = 100000;
        float minimumTimeSecondTrack = 100000;
        if (lhs->hasBKLMHits()) {
          minimumTimeFirstTrack = lhs->getSortedBKLMHitList()[lhs->getNumberOfBKLMHits() - 1]->getTime();
        }
        if (lhs->hasEKLMHits()) {
          RelationVector<EKLMHit2d> eklmHit2ds = lhs->getSortedEKLMHitList()[lhs->getNumberOfEKLMHits() - 1]->getRelationsTo<EKLMHit2d>();
          //std::cout << eklmHit2ds[0]->getTime() << "\n";
          if (minimumTimeFirstTrack > eklmHit2ds[0]->getTime()) {
            minimumTimeFirstTrack = eklmHit2ds[0]->getTime();
          }
        }
        if (rhs->hasBKLMHits()) {
          minimumTimeSecondTrack = rhs->getSortedBKLMHitList()[rhs->getNumberOfBKLMHits() - 1]->getTime();
        }
        if (rhs->hasEKLMHits()) {
          RelationVector<EKLMHit2d> eklmHit2ds = rhs->getSortedEKLMHitList()[rhs->getNumberOfEKLMHits() - 1]->getRelationsTo<EKLMHit2d>();
          //std::cout << eklmHit2ds[0]->getTime() << "\n";
          if (minimumTimeSecondTrack > eklmHit2ds[0]->getTime()) {
            minimumTimeSecondTrack = eklmHit2ds[0]->getTime();
          }
        }
        //std::cout << minimumTimeFirstTrack << " " << minimumTimeSecondTrack << "\n";
        //return (lhs->getPositionSeed().Y() > rhs->getPositionSeed().Y());
        return (minimumTimeFirstTrack < minimumTimeSecondTrack);
      };
      std::sort(recoTracks.begin(), recoTracks.end(), lmdSort);
      RecoTrack* upperTrack = recoTracks[0];
      RecoTrack* lowerTrack = recoTracks[1];
      //B2DEBUG(99, "upper track posSeed :" << upperTrack->getPositionSeed().Y());
      //B2DEBUG(99, "Lowee track posSeed :" << lowerTrack->getPositionSeed().Y());
      RecoTrack* MergedRecoTrack = MergedRecoTracks.appendNew(upperTrack->getPositionSeed(),
                                                              upperTrack->getMomentumSeed(),
                                                              upperTrack->getChargeSeed());
      // retain the seed time of the original track. Important for t0 extraction.
      MergedRecoTrack->setTimeSeed(upperTrack->getTimeSeed());

      //MergedRecoTrack->addHitsFromRecoTrack(upperTrack);

      //int upperTotalHits = upperTrack->getNumberOfTotalHits();
      //std::cout << "upperTotalHits:" << upperTotalHits << "\n";
      int sortingNumber = 0;

      if (upperTrack->hasBKLMHits()) {
        int BKLMHits = upperTrack->getNumberOfBKLMHits();
        for (int i = BKLMHits - 1; i >= 0; i--) {
          MergedRecoTrack->addBKLMHit(upperTrack->getSortedBKLMHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (upperTrack->hasEKLMHits()) {
        int EKLMHits = upperTrack->getNumberOfEKLMHits();
        for (int i = EKLMHits - 1; i >= 0; i--) {
          MergedRecoTrack->addEKLMHit(upperTrack->getSortedEKLMHitList()[i], sortingNumber);
        }
      }

      if (upperTrack->hasCDCHits()) {
        int CDCHits = upperTrack->getNumberOfCDCHits();
        for (int i = CDCHits - 1; i >= 0; i--) {
          MergedRecoTrack->addCDCHit(upperTrack->getSortedCDCHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (upperTrack->hasSVDHits()) {
        int SVDHits = upperTrack->getNumberOfSVDHits();
        for (int i = SVDHits - 1; i >= 0; i--) {
          MergedRecoTrack->addSVDHit(upperTrack->getSortedSVDHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (upperTrack->hasPXDHits()) {
        int PXDHits = upperTrack->getNumberOfPXDHits();
        for (int i = PXDHits - 1; i >= 0; i--) {
          MergedRecoTrack->addPXDHit(upperTrack->getSortedPXDHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      else if (upperTrack->hasPXDHits()) {
        int PXDHits = upperTrack->getNumberOfPXDHits();
        for (int i = PXDHits - 1; i >= 0; i--) {
          MergedRecoTrack->addPXDHit(upperTrack->getSortedPXDHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }


      //MergedRecoTrack->addHitsFromRecoTrack(lowerTrack, upperTrack->getNumberOfTotalHits()); //getNumberOfTrackingHits());

      if (lowerTrack->hasPXDHits()) {
        int PXDHits = lowerTrack->getNumberOfPXDHits();
        for (int i = 0; i < PXDHits; i++) {
          MergedRecoTrack->addPXDHit(lowerTrack->getSortedPXDHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (lowerTrack->hasSVDHits()) {
        int SVDHits = lowerTrack->getNumberOfSVDHits();
        for (int i = 0; i < SVDHits; i++) {
          MergedRecoTrack->addSVDHit(lowerTrack->getSortedSVDHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (lowerTrack->hasCDCHits()) {
        int CDCHits = lowerTrack->getNumberOfCDCHits();
        for (int i = 0; i < CDCHits; i++) {
          MergedRecoTrack->addCDCHit(lowerTrack->getSortedCDCHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (lowerTrack->hasBKLMHits()) {
        int BKLMHits = lowerTrack->getNumberOfBKLMHits();
        for (int i = 0; i < BKLMHits; i++) {
          MergedRecoTrack->addBKLMHit(lowerTrack->getSortedBKLMHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (lowerTrack->hasEKLMHits()) {
        int EKLMHits = lowerTrack->getNumberOfEKLMHits();
        for (int i = 0; i < EKLMHits; i++) {
          MergedRecoTrack->addEKLMHit(lowerTrack->getSortedEKLMHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (upperTrack->hasBKLMHits()) {
        int BKLMHits = upperTrack->getNumberOfBKLMHits();
        for (int i = BKLMHits - 1; i >= 0; i--) {
          MergedRecoTrack->addBKLMHit(upperTrack->getSortedBKLMHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (upperTrack->hasEKLMHits()) {
        int EKLMHits = upperTrack->getNumberOfEKLMHits();
        for (int i = EKLMHits - 1; i >= 0; i--) {
          MergedRecoTrack->addEKLMHit(upperTrack->getSortedEKLMHitList()[i], sortingNumber);
        }
      }

      if (upperTrack->hasCDCHits()) {
        int CDCHits = upperTrack->getNumberOfCDCHits();
        for (int i = CDCHits - 1; i >= 0; i--) {
          MergedRecoTrack->addCDCHit(upperTrack->getSortedCDCHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (upperTrack->hasSVDHits()) {
        int SVDHits = upperTrack->getNumberOfSVDHits();
        for (int i = SVDHits - 1; i >= 0; i--) {
          MergedRecoTrack->addSVDHit(upperTrack->getSortedSVDHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (upperTrack->hasPXDHits()) {
        int PXDHits = upperTrack->getNumberOfPXDHits();
        for (int i = PXDHits - 1; i >= 0; i--) {
          MergedRecoTrack->addPXDHit(upperTrack->getSortedPXDHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      //MergedRecoTrack->addHitsFromRecoTrack(lowerTrack, upperTrack->getNumberOfTotalHits()); //getNumberOfTrackingHits());

      if (lowerTrack->hasPXDHits()) {
        int PXDHits = lowerTrack->getNumberOfPXDHits();
        for (int i = 0; i < PXDHits; i++) {
          MergedRecoTrack->addPXDHit(lowerTrack->getSortedPXDHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (lowerTrack->hasSVDHits()) {
        int SVDHits = lowerTrack->getNumberOfSVDHits();
        for (int i = 0; i < SVDHits; i++) {
          MergedRecoTrack->addSVDHit(lowerTrack->getSortedSVDHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (lowerTrack->hasCDCHits()) {
        int CDCHits = lowerTrack->getNumberOfCDCHits();
        for (int i = 0; i < CDCHits; i++) {
          MergedRecoTrack->addCDCHit(lowerTrack->getSortedCDCHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (lowerTrack->hasBKLMHits()) {
        int BKLMHits = lowerTrack->getNumberOfBKLMHits();
        for (int i = 0; i < BKLMHits; i++) {
          MergedRecoTrack->addBKLMHit(lowerTrack->getSortedBKLMHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (lowerTrack->hasEKLMHits()) {
        int EKLMHits = lowerTrack->getNumberOfEKLMHits();
        for (int i = 0; i < EKLMHits; i++) {
          MergedRecoTrack->addEKLMHit(lowerTrack->getSortedEKLMHitList()[i], sortingNumber);
          sortingNumber++;
        }
      }

      if (m_param_deleteOtherRecoTracks) {
        // Delete the other RecoTracks, as they were probably found under a wrong T0 assumption.
        recoTracks.clear();
      }
    }
  }
}
