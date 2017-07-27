/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcToVXDExtrapolator/CDCToSpacePointCKFFindlet.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCToSpacePointCKFFindlet::CDCToSpacePointCKFFindlet()
{
  addProcessingSignalListener(&m_cdcTracksLoader);
  addProcessingSignalListener(&m_hitsLoader);
  addProcessingSignalListener(&m_treeSearchFindlet);
  addProcessingSignalListener(&m_storeArrayHandler);
  addProcessingSignalListener(&m_overlapTeacher);
  addProcessingSignalListener(&m_overlapResolver);
  addProcessingSignalListener(&m_svdSpacePointTagger);
}

void CDCToSpacePointCKFFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_cdcTracksLoader.exposeParameters(moduleParamList, prefix);
  m_hitsLoader.exposeParameters(moduleParamList, prefix);
  m_treeSearchFindlet.exposeParameters(moduleParamList, prefix);
  m_storeArrayHandler.exposeParameters(moduleParamList, prefix);
  m_overlapTeacher.exposeParameters(moduleParamList, prefix);
  m_overlapResolver.exposeParameters(moduleParamList, prefix);
  m_svdSpacePointTagger.exposeParameters(moduleParamList, prefix);
}

void CDCToSpacePointCKFFindlet::beginEvent()
{
  Super::beginEvent();

  m_cdcRecoTrackVector.clear();
  m_spacePointVector.clear();
  m_results.clear();
}

void CDCToSpacePointCKFFindlet::apply()
{
  m_cdcTracksLoader.apply(m_cdcRecoTrackVector);
  m_hitsLoader.apply(m_spacePointVector);

  // Remove all non-fitted tracks
  const auto& cdcTrackWasNotFitted = [](const RecoTrack * recoTrack) {
    return not recoTrack->wasFitSuccessful();
  };
  erase_remove_if(m_cdcRecoTrackVector, cdcTrackWasNotFitted);

  m_treeSearchFindlet.apply(m_cdcRecoTrackVector, m_spacePointVector, m_results);

  // Remove all empty results
  const auto& resultIsEmpty = [](const CKFCDCToVXDStateObject::ResultObject & result) {
    return result.getHits().empty();
  };
  erase_remove_if(m_results, resultIsEmpty);

  m_overlapTeacher.apply(m_results);
  m_overlapResolver.apply(m_results);

  m_storeArrayHandler.apply(m_results);
  m_svdSpacePointTagger.apply(m_results, m_spacePointVector);
}
