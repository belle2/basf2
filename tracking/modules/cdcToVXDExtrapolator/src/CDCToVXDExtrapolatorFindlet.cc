/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcToVXDExtrapolator/CDCToVXDExtrapolatorFindlet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCToVXDExtrapolatorFindlet::CDCToVXDExtrapolatorFindlet()
{
  addProcessingSignalListener(&m_treeSearchFindlet);
  addProcessingSignalListener(&m_storeArrayMerger);
}

void CDCToVXDExtrapolatorFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_treeSearchFindlet.exposeParameters(moduleParamList, prefix);
  m_storeArrayMerger.exposeParameters(moduleParamList, prefix);
}

void CDCToVXDExtrapolatorFindlet::beginEvent()
{
  m_cdcRecoTrackVector.clear();
  m_spacePointVector.clear();
}

void CDCToVXDExtrapolatorFindlet::apply()
{
  // Read in the CDC reco tracks
  m_storeArrayMerger.fetch(m_cdcRecoTrackVector);

  // Read in the SpacePoints
  StoreArray<SpacePoint> spacePoints;
  m_spacePointVector.reserve(spacePoints.getEntries());

  for (const SpacePoint& spacePoint : spacePoints) {
    m_spacePointVector.push_back(&spacePoint);
  }

  // Apply the tree search
  m_treeSearchFindlet.apply(m_cdcRecoTrackVector, m_spacePointVector);

  std::vector<std::vector<const SpacePoint*>> results;

  for (RecoTrack* seed : m_cdcRecoTrackVector) {
    m_treeSearchFindlet.traverseTree(seed, results);
    B2INFO("Number of tracks: " << results.size());
  }

  // Use the found hits for each track, create new VXD reco tracks, add relations, merge the tracks and fill them
  // into a new store array
  //m_storeArrayMerger.apply(m_cdcTracksWithMatchedSpacePoints);
}
