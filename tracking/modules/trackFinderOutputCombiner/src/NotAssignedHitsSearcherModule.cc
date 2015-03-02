/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderOutputCombiner/NotAssignedHitsSearcherModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>

#include <framework/gearbox/Const.h>

#include <genfit/TrackCand.h>

#include <cdc/dataobjects/CDCHit.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(NotAssignedHitsSearcher);


NotAssignedHitsSearcherModule::NotAssignedHitsSearcherModule() : Module()
{
  setDescription("Copies the CDCHits not used by any track candidate from a track finder into another StoreArray. This module will not be necessary once we can mark hits as used.");

  addParam("TracksFromFinder",
           m_param_tracksFromTrackFinder,
           "TrackCandidates store array name from the track finder.",
           std::string("GoodTrackCands"));
  addParam("CDCHits",
           m_param_cdcHits,
           "Name of the store array containing the hits.",
           std::string("CDCHits"));
  addParam("NotAssignedCDCHits",
           m_param_notAssignedCDCHits,
           "Name of the store array containing the hits not used by the track finding algorithm. This store array is created by this module.",
           std::string("UnknownCDCHits"));
  addParam("MinimumDistanceToSplit",
           m_param_minimumDistanceToSplit,
           "Minimal distance to split up a track. This distance is measured in percentage of the whole track length.",
           0.2);
  addParam("InitialAssignmentValue",
           m_param_initialAssignmentValue,
           "",
           false);
}

void NotAssignedHitsSearcherModule::initialize()
{
  StoreArray<genfit::TrackCand> trackCands(m_param_tracksFromTrackFinder);
  StoreArray<CDCHit> cdcHits(m_param_cdcHits);

  trackCands.isRequired();
  cdcHits.isRequired();

  StoreArray<CDCHit> notAssignedCDCHits(m_param_notAssignedCDCHits);
  notAssignedCDCHits.registerInDataStore();
  notAssignedCDCHits.registerRelationTo(cdcHits);
}

void NotAssignedHitsSearcherModule::event()
{
  StoreArray<genfit::TrackCand> trackCands(m_param_tracksFromTrackFinder);
  StoreArray<CDCHit> cdcHits(m_param_cdcHits);
  StoreArray<CDCHit> notAssignedCDCHits(m_param_notAssignedCDCHits);
  const CDCRiemannFitter& circleFitter = CDCRiemannFitter::getFitter();

  std::vector<bool> isAssigned;
  isAssigned.resize(cdcHits.getEntries(), m_param_initialAssignmentValue);

  for (genfit::TrackCand & legendreTrackCand : trackCands) {
    // check for breaks in the hit pattern. If there are no such breaks, we mark every hit as assigned. If we find such a break, only add the longest train of hits if it is over a certain limit
    typedef std::pair<double, unsigned int> hitWithSegmentLength;
    std::list<hitWithSegmentLength> perpSList;
    CDCTrajectory3D trajectory(legendreTrackCand.getPosSeed(), legendreTrackCand.getMomSeed(), legendreTrackCand.getChargeSeed());
    const CDCTrajectory2D& trajectory2D = trajectory.getTrajectory2D();
    double radius = trajectory2D.getGlobalCircle().absRadius();

    // calculate all circle segments lengths
    for (unsigned int cdcHitID : legendreTrackCand.getHitIDs(Const::CDC)) {
      CDCHit* cdcHit = cdcHits[cdcHitID];
      CDCWireHit wireHit(cdcHit);
      double perpS;
      if (wireHit.getStereoType() == AXIAL) {
        perpS = wireHit.getPerpS(trajectory2D);
      } else {
        TrackFindingCDC::CDCRecoHit3D recoHit3D = TrackFindingCDC::CDCRecoHit3D::reconstruct(TrackFindingCDC::CDCRLWireHit(wireHit), trajectory2D);
        perpS = recoHit3D.getPerpS();
      }

      if (perpS < 0) {
        perpS = perpS + 2 * TMath::Pi() * radius;
      }

      perpSList.emplace_back(perpS, cdcHitID);

      // we mark this hit as assigned. Later, we will maybe unmark it again.
      isAssigned[cdcHitID] = true;
    }

    // sort the hits according to their segment length
    perpSList.sort([](const hitWithSegmentLength & first, const hitWithSegmentLength & second) -> bool {
      return first.first > second.first;
    });

    double trackLength = perpSList.back().first - perpSList.front().first;

    // search for the hit breaks
    std::list<std::vector<hitWithSegmentLength>> hitTrains;
    hitWithSegmentLength lastHit = std::make_pair(-99, 0);
    auto endOfLastHitTrain = perpSList.begin();
    for (const hitWithSegmentLength & hit : perpSList) {
      if (lastHit.first != -99) {
        // calculate "distance" between the hits. As the hits are sorted this should be positive every time.
        double distance = hit.first - lastHit.first;
        if (distance / trackLength > m_param_minimumDistanceToSplit) {
          // add the hit train to the list
          hitTrains.emplace_back();
          std::vector<hitWithSegmentLength>& addedHitTrain = hitTrains.back();
          for (; endOfLastHitTrain->second != hit.second; endOfLastHitTrain++) {
            addedHitTrain.push_back(*endOfLastHitTrain);
          }
        }
      }

      lastHit = hit;
    }

    // Also add the last train
    hitTrains.emplace_back();
    std::vector<hitWithSegmentLength>& addedHitTrain = hitTrains.back();
    for (; endOfLastHitTrain != perpSList.end(); endOfLastHitTrain++) {
      addedHitTrain.push_back(*endOfLastHitTrain);
    }

    hitTrains.sort([](const std::vector<hitWithSegmentLength>& first, const std::vector<hitWithSegmentLength>& second) -> bool {
      return first.size() > second.size();
    });

    // Reset the track candidate and only add the hits from the first hit train - the hit train with the maximum hits
    legendreTrackCand.reset();
    unsigned int sortingParameter = 0;
    CDCObservations2D axialObservations;
    for (const hitWithSegmentLength & hit : hitTrains.front()) {
      // I DO NOT KNOW THE PLANE ID!
      legendreTrackCand.addHit(Const::CDC, hit.second, -1, sortingParameter);
      CDCWireHit cdcWireHit(cdcHits[hit.second]);
      if (cdcWireHit.getStereoType() == AXIAL)
        axialObservations.append(cdcWireHit);
      sortingParameter++;
    }

    // now check the hit trains. If there is only on hit train, we mark all hits. As this is done already, we just go on.
    // Else, we only unmark the hits not coming from the longest segment
    if (hitTrains.size() == 1) {
      continue;
    } else if (hitTrains.size() > 1) {

      B2INFO("Left: " << hitTrains.front().size())

      // Delete the first element as it is the one with the maximum number of hits in it
      hitTrains.pop_front();
      B2INFO("Deleting " << hitTrains.size() << " hit trains.")
      for (const std::vector<hitWithSegmentLength>& hitTrain : hitTrains) {
        B2INFO("Deleting: " << hitTrain.size())
        for (const hitWithSegmentLength & hit : hitTrain) {
          isAssigned[hit.second] = false;
        }
      }

      // TODO: The fitting procedure does not work!
      /*CDCTrajectory2D fittedTrajectory2D;
      CDCTrajectorySZ fittedTrajectorySZ = CDCTrajectorySZ::basicAssumption();
      circleFitter.update(fittedTrajectory2D, axialObservations);
      CDCTrajectory3D fittedTrajectory3D(fittedTrajectory2D, fittedTrajectorySZ);

      Vector3D position(legendreTrackCand.getPosSeed());
      Vector3D momentum(fittedTrajectory2D.getUnitMom2D(position.xy()) * fittedTrajectory2D.getAbsMom2D(), 0);

      legendreTrackCand.setPosMomSeed(position, momentum, legendreTrackCand.getChargeSeed());*/
    } else {
      B2WARNING("Something is going wrong with the hit splitter. We could not find any hit trains.")
    }
  }

  for (int cdcHitID = 0; cdcHitID < cdcHits.getEntries(); cdcHitID++) {
    if (not isAssigned[cdcHitID]) {
      CDCHit* oldCDCHit = cdcHits[cdcHitID];
      CDCHit* newCDCHit = notAssignedCDCHits.appendNew(*oldCDCHit);
      newCDCHit->addRelationTo(oldCDCHit);
    }
  }
}

