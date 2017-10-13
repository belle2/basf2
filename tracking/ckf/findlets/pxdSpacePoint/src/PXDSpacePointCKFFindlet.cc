/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/findlets/pxdSpacePoint/PXDSpacePointCKFFindlet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

PXDSpacePointCKFFindlet::PXDSpacePointCKFFindlet()
{
  addProcessingSignalListener(&m_dataLoader);
  addProcessingSignalListener(&m_treeSearchFindlet);
  addProcessingSignalListener(&m_overlapResolver);
  addProcessingSignalListener(&m_spacePointTagger);
}

void PXDSpacePointCKFFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_dataLoader.exposeParameters(moduleParamList, prefix);
  m_treeSearchFindlet.exposeParameters(moduleParamList, prefix);
  m_overlapResolver.exposeParameters(moduleParamList, prefix);
  m_spacePointTagger.exposeParameters(moduleParamList, prefix);
}

void PXDSpacePointCKFFindlet::beginEvent()
{
  Super::beginEvent();

  m_cdcRecoTrackVector.clear();
  m_spacePointVector.clear();
  m_results.clear();
}

void PXDSpacePointCKFFindlet::apply()
{
  m_dataLoader.apply(m_cdcRecoTrackVector, m_spacePointVector);
  const auto notFromPXD = [](const SpacePoint * spacePoint) {
    return spacePoint->getType() != VXD::SensorInfoBase::PXD;
  };
  TrackFindingCDC::erase_remove_if(m_spacePointVector, notFromPXD);
  const auto hasNoSVD = [](const RecoTrack * recoTrack) {
    const auto& svdHitList = recoTrack->getSVDHitList();
    return svdHitList.empty() or svdHitList.front()->getSensorID().getLayerNumber() > 4;
  };
  TrackFindingCDC::erase_remove_if(m_cdcRecoTrackVector, hasNoSVD);

  m_treeSearchFindlet.apply(m_cdcRecoTrackVector, m_spacePointVector, m_results);

  m_overlapResolver.apply(m_results);

  m_dataLoader.store(m_results);
  m_spacePointTagger.apply(m_results, m_spacePointVector);
}
