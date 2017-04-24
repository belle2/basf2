/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcToVXDExtrapolator/filterBased/CDCToVXDExtrapolatorFindlet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCToVXDExtrapolatorFindlet::CDCToVXDExtrapolatorFindlet()
{
  addProcessingSignalListener(&m_treeSearchFindlet);
  addProcessingSignalListener(&m_storeArrayMerger);
}

void CDCToVXDExtrapolatorFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_treeSearchFindlet.exposeParameters(moduleParamList, prefix);
  m_storeArrayMerger.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter("exportTracks", m_param_exportTracks, "", m_param_exportTracks);
}

void CDCToVXDExtrapolatorFindlet::beginEvent()
{
  Super::beginEvent();

  m_cdcRecoTrackVector.clear();
  m_spacePointVector.clear();
  m_results.clear();
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
  m_treeSearchFindlet.apply(m_cdcRecoTrackVector, m_spacePointVector, m_results);

  B2INFO("Found " << m_results.size() << " tracks");

  // Use the found hits for each track, create new VXD reco tracks, add relations, merge the tracks and fill them
  // into a new store array
  if (m_param_exportTracks) {
    m_storeArrayMerger.apply(m_results);
  }
}
