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

using namespace Belle2;

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

  std::vector<bool> isAssigned;
  isAssigned.resize(cdcHits.getEntries(), false);

  for (const genfit::TrackCand & legendreTrackCand : trackCands) {
    for (unsigned int cdcHitID : legendreTrackCand.getHitIDs(Const::CDC)) {
      isAssigned[cdcHitID] = true;
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

