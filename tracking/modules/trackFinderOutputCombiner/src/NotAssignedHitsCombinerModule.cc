/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderOutputCombiner/NotAssignedHitsCombinerModule.h>

//datastore types
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>

#include <framework/gearbox/Const.h>

#include <genfit/TrackCand.h>

#include <tracking/trackFinderOutputCombiner/TrackFinderOutputCombiner.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(NotAssignedHitsCombiner);

NotAssignedHitsCombinerModule::NotAssignedHitsCombinerModule()
{
  setDescription("Combines the newly found track candidates from the local track finder with the ones found by the legendre track finder to get all tracks.");

  addParam("TracksFromLegendreFinder",
           m_param_tracksFromLegendreFinder,
           "TrackCandidates store array name from the legendre track finder with hits from CDCHits.",
           std::string("PreLegendreTrackCands"));
  addParam("NotAssignedTracksFromLocalFinder",
           m_param_notAssignedTracksFromLocalFinder,
           "TrackCandidates store array name from the local track finder with hits from NotAssignedCDCHits. These tracks will be converted into tracks with hits from CDCHits",
           std::string("NotAssignedLocalTrackCands"));
  addParam("CDCHits",
           m_param_cdcHits,
           "Name of the store array containing the hits from the legendre track finder.",
           std::string("CDCHits"));
  addParam("NotAssignedCDCHits",
           m_param_notAssignedCDCHits,
           "Name of the store array containing the hits not used by the legendre track finding algorithm but from the local track finder.",
           std::string("UnknownCDCHits"));


  addParam("ResultTrackCands",
           m_param_resultTrackCands,
           "TrackCandidates collection name from the combined results of the two recognition algorithm. The hits come from the parameter CDCHits.",
           std::string("ResultTrackCands"));

  addParam("MinimumDistanceBetweenHits",
           m_param_minimumDistanceBetweenHits,
           "Minimum distance between two CDCHits to assume a break.",
           80);

  addParam("MinimumHitSizeForTrack",
           m_param_minimumHitSizeForTrack,
           "Minimum track hits a track candidate from the local track finder needs to have to be treated independent.",
           static_cast<unsigned int>(15));

  addParam("MinimalChi2",
           m_param_minimal_chi2,
           "Minimal chi2 for a merge.",
           0.98);

  addParam("ReAssignLocalTracks", m_param_reassignLocalTracks, "Reassign the hits of he local track candidates.", true);
}


void NotAssignedHitsCombinerModule::initialize()
{
  StoreArray<genfit::TrackCand> legendreTrackCands(m_param_tracksFromLegendreFinder);
  StoreArray<genfit::TrackCand> localTrackCands(m_param_notAssignedTracksFromLocalFinder);

  legendreTrackCands.isRequired();
  localTrackCands.isRequired();

  StoreArray<CDCHit> cdcHits(m_param_cdcHits);
  StoreArray<CDCHit> notAssigned(m_param_notAssignedCDCHits);

  StoreArray<genfit::TrackCand> resultTrackCands(m_param_resultTrackCands);
  resultTrackCands.registerInDataStore();
}


void NotAssignedHitsCombinerModule::event()
{
  B2DEBUG(100, "########## NotAssignedHitsCombinerModule ############")

  StoreArray<genfit::TrackCand> resultTrackCands(m_param_resultTrackCands);
  StoreArray<genfit::TrackCand> localTrackCands(m_param_notAssignedTracksFromLocalFinder);
  StoreArray<genfit::TrackCand> legendreTrackCands(m_param_tracksFromLegendreFinder);
  StoreArray<CDCHit> notAssigned(m_param_notAssignedCDCHits);
  StoreArray<CDCHit> cdcHits(m_param_cdcHits);

  if (m_param_reassignLocalTracks)
    TrackFinderOutputCombiner::resetLocalTrackCandsToCorrectCDCHits(localTrackCands, notAssigned, m_param_cdcHits);
  matchNewlyFoundLocalTracksToLegendreTracks(resultTrackCands, localTrackCands, legendreTrackCands, cdcHits);
}

bool NotAssignedHitsCombinerModule::isTooFarAway(const genfit::TrackCand& /*legendreTrackCand*/, CDCHit* /*firstCDCHitOfSegment*/)
{
  // Return false every time. This method does not help...
  return false;
}

void NotAssignedHitsCombinerModule::matchNewlyFoundLocalTracksToLegendreTracks(StoreArray<genfit::TrackCand>& resultTrackCands, const StoreArray<genfit::TrackCand>& localTrackCands, const StoreArray<genfit::TrackCand>& legendreTrackCands, const StoreArray<CDCHit>& cdcHits)
{
  const int notFoundID = -1;
  std::vector<TrackFinderOutputCombiner::HitSegment> hitSegmentList = TrackFinderOutputCombiner::collectHitSegmentsOfLocalTracks(localTrackCands, cdcHits, m_param_minimumDistanceBetweenHits);
  int counterLegendre = 0;

  for (const genfit::TrackCand & legendreTrackCand : legendreTrackCands) {
    std::vector<TrackFindingCDC::TrackHit*> hitsAxial;
    hitsAxial.reserve(legendreTrackCand.getNHits());

    for (int cdcHitID : legendreTrackCand.getHitIDs(Const::CDC)) {
      CDCHit* cdcHit = cdcHits[cdcHitID];
      if (cdcHit->getISuperLayer() % 2 == 0) {
        TrackHit* trackHit = new TrackFindingCDC::TrackHit(cdcHit, cdcHitID);
        hitsAxial.push_back(trackHit);
      }
    }

    int lastIndexForLegendreTrackAxial = hitsAxial.size();

    int counterLocal = -1;

    for (TrackFinderOutputCombiner::HitSegment & hitSegment : hitSegmentList) {

      hitsAxial.resize(lastIndexForLegendreTrackAxial);
      counterLocal++;

      CDCHit* firstCDCHitOfSegment = cdcHits[hitSegment.first[0]];
      if (isTooFarAway(legendreTrackCand, firstCDCHitOfSegment)) {
        B2DEBUG(100, "The pair " << counterLocal << " and " << counterLegendre << " are far away from each other.");
        continue;
      }

      for (int cdcHitID : hitSegment.first) {
        CDCHit* cdcHit = cdcHits[cdcHitID];
        if (cdcHit->getISuperLayer() % 2 == 0) {
          TrackHit* trackHit = new TrackFindingCDC::TrackHit(cdcHit, cdcHitID);
          hitsAxial.push_back(trackHit);
        }
      }

      // Only use the axial hits for fitting
      if (hitsAxial.size() - lastIndexForLegendreTrackAxial != 0) {
        double prob = TrackFinderOutputCombiner::calculateChi2(hitsAxial);

        B2INFO("Prob from " << counterLegendre << " to " << counterLocal << ": " << prob);
        TrackFinderOutputCombiner::BestFitInformation& fitResult = hitSegment.second;
        if (fitResult.second < prob) {
          fitResult.first = counterLegendre;
          fitResult.second = prob;
        }
      } else {
        B2DEBUG(90, "Found empty hit segment or a hit segment made of stereo hits only.")
      }
    }
    genfit::TrackCand* newAppendedTrackCand = resultTrackCands.appendNew(legendreTrackCand);
    TrackFinderOutputCombiner::constructTrackCandidate(newAppendedTrackCand, cdcHits);
    counterLegendre++;

    std::for_each(hitsAxial.begin(), hitsAxial.end(), [](TrackFindingCDC::TrackHit * hit) {delete hit;});
  }

  // Add the local track segments to the legendre tracks if there is a match.
  // Otherwise add them to the totallyNewLocalTracks
  int counterLocal = 0;
  for (const TrackFinderOutputCombiner::HitSegment & hitSegment : hitSegmentList) {

    TrackFinderOutputCombiner::BestFitInformation fitResult = hitSegment.second;
    genfit::TrackCand* newAppendedTrackCand = nullptr;

    if (fitResult.first != notFoundID and fitResult.second > m_param_minimal_chi2) {
      newAppendedTrackCand = resultTrackCands[fitResult.first];
      B2DEBUG(90, "Adding track local " << counterLocal << " to legendre " << fitResult.first << " with " << fitResult.second)
    } else if (hitSegment.first.size() > m_param_minimumHitSizeForTrack) {
      newAppendedTrackCand = resultTrackCands.appendNew();
      B2DEBUG(90, "Adding totally new local track: " << counterLocal)
    } else {
      continue;
    }

    for (int cdcHitID : hitSegment.first) {
      newAppendedTrackCand->addHit(Const::CDC, cdcHitID);
    }

    TrackFinderOutputCombiner::constructTrackCandidate(newAppendedTrackCand, cdcHits);
    counterLocal++;
  }

  /*

  // Go through all the totallyNewLocalTracks and try to merge them
  for (TrackFinderOutputCombiner::HitSegment& hitSegment : totallyNewLocalTracks) {
    std::vector<TrackFindingCDC::TrackHit*> hitsAxial;
    hitsAxial.reserve(2*hitSegment.first.size());
    for (int cdcHitID : hitSegment.first) {
      CDCHit * cdcHit = cdcHits[cdcHitID];
      if (cdcHit->getISuperLayer() % 2 == 0) {
        TrackHit* trackHit = new TrackFindingCDC::TrackHit(cdcHit, cdcHitID);
        hitsAxial.push_back(trackHit);
      }
    }

    int lastIndexForLegendreTrackAxial = hitsAxial.size();
    double probabilityBefore = TrackFinderOutputCombiner::calculateChi2(hitsAxial);

    int counterLocal = 0;
    // Do not process all pairs twice!
    for (const TrackFinderOutputCombiner::HitSegment& otherHitSegment : totallyNewLocalTracks) {
      if(otherHitSegment == hitSegment)
        continue;

      hitsAxial.resize(lastIndexForLegendreTrackAxial);

      for (int cdcHitID : otherHitSegment.first) {
        CDCHit * cdcHit = cdcHits[cdcHitID];
        if (cdcHit->getISuperLayer() % 2 == 0) {
          TrackHit* trackHit = new TrackFindingCDC::TrackHit(cdcHit, cdcHitID);
          hitsAxial.push_back(trackHit);
        }
      }

      if (hitsAxial.size() - lastIndexForLegendreTrackAxial != 0) {
        double prob = TrackFinderOutputCombiner::calculateChi2(hitsAxial);
        TrackFinderOutputCombiner::BestFitInformation& fitResult = hitSegment.second;
        if (prob > 0.4 * probabilityBefore and (fitResult.first == -1 or fitResult.second < prob)) {
          fitResult.first = counterLocal;
          fitResult.second = prob;
        }
      } else {
        B2DEBUG(90, "Found empty hit segment or a hit segment made of stereo hits only.")
      }

    }
    counterLocal++;

  }*/
}
