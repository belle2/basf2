/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/TrackSetEvaluatorGreedyDEVModule.h"

#include <tracking/trackFindingVXD/trackSetEvaluator/Scrooge.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapResolverNodeInfo.h>

#include <vector>

using namespace Belle2;


REG_MODULE(TrackSetEvaluatorGreedyDEV)

TrackSetEvaluatorGreedyDEVModule::TrackSetEvaluatorGreedyDEVModule() : Module()
{
  setDescription("Expects a container of SpacePointTrackCandidates,\
 selects a subset of non-overlapping TCs using the Greedy algorithm.");

  addParam("NameSpacePointTrackCands", m_nameSpacePointTrackCands, "Name of expected StoreArray of SpacePoint track candidates.",
           std::string(""));

  addParam("NameOverlapNetworks", m_nameOverlapNetworks, "Name of expected StoreArray with overlap "
           "networks.", std::string(""));
}

void TrackSetEvaluatorGreedyDEVModule::event()
{
  //Create an empty object of the type,
  //that needs to be given to Scrooge.
  std::vector<OverlapResolverNodeInfo> qiTrackOverlap;
  unsigned int const nSpacePointTrackCands = m_spacePointTrackCands.getEntries();
  qiTrackOverlap.reserve(nSpacePointTrackCands);

  //fill this object with the necessary information:
  for (auto && spacePointTrackCand : m_spacePointTrackCands) {
    qiTrackOverlap.emplace_back(spacePointTrackCand.getQualityIndicator(), spacePointTrackCand.getArrayIndex(),
                                m_overlapNetworks[0]->getOverlapForTrackIndex(spacePointTrackCand.getArrayIndex()),
                                true);
  }

  //make a Scrooge and udpate the activity
  Scrooge scrooge;
  scrooge.performSelection(qiTrackOverlap);

  for (auto && track : qiTrackOverlap) {
    if (track.activityState < 0.75) {
      m_spacePointTrackCands[track.trackIndex]->removeRefereeStatus(SpacePointTrackCand::c_isActive);
    }
  }

  //-----------------------------------------------------------------------------------------------
}
