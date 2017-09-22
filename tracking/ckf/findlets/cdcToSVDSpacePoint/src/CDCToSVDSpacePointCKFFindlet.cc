/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/findlets/cdcToSVDSpacePoint/CDCToSVDSpacePointCKFFindlet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCToSVDSpacePointCKFFindlet::CDCToSVDSpacePointCKFFindlet()
{
  addProcessingSignalListener(&m_dataLoader);
  addProcessingSignalListener(&m_treeSearchFindlet);
  addProcessingSignalListener(&m_overlapResolver);
  addProcessingSignalListener(&m_spacePointTagger);
}

void CDCToSVDSpacePointCKFFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_dataLoader.exposeParameters(moduleParamList, prefix);
  m_treeSearchFindlet.exposeParameters(moduleParamList, prefix);
  m_overlapResolver.exposeParameters(moduleParamList, prefix);
  m_spacePointTagger.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter("minimalHitRequirement", m_param_minimalHitRequirement,
                                "Minimal Hit requirement for the results (counted in space points)",
                                m_param_minimalHitRequirement);
  moduleParamList->addParameter("minimalPtRequirement", m_param_minimalPtRequirement,
                                "Minimal Pt requirement for the input CDC tracks",
                                m_param_minimalPtRequirement);

}

void CDCToSVDSpacePointCKFFindlet::beginEvent()
{
  Super::beginEvent();

  m_cdcRecoTrackVector.clear();
  m_spacePointVector.clear();
  m_results.clear();
}

void CDCToSVDSpacePointCKFFindlet::apply()
{
  m_dataLoader.apply(m_cdcRecoTrackVector, m_spacePointVector);

  const auto hasLowPt = [this](const auto & track) {
    return track->getMomentumSeed().Pt() < m_param_minimalPtRequirement;
  };
  TrackFindingCDC::erase_remove_if(m_cdcRecoTrackVector, hasLowPt);

  const auto hitIsAlreadyUsed = [](const auto & hit) {
    return hit->getAssignmentState();
  };
  TrackFindingCDC::erase_remove_if(m_spacePointVector, hitIsAlreadyUsed);

  m_treeSearchFindlet.apply(m_cdcRecoTrackVector, m_spacePointVector, m_results);
  m_overlapResolver.apply(m_results);

  const auto hasLowHitNumber = [this](const CKFResultObject<RecoTrack, SpacePoint>& result) {
    return result.getHits().size() < m_param_minimalHitRequirement;
  };
  TrackFindingCDC::erase_remove_if(m_results, hasLowHitNumber);

  m_dataLoader.store(m_results);
  m_spacePointTagger.apply(m_results, m_spacePointVector);
}
