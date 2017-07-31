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
  addProcessingSignalListener(&m_storeArrayHandler);
  addProcessingSignalListener(&m_overlapResolver);
  addProcessingSignalListener(&m_spacePointTagger);
}

void CDCToSVDSpacePointCKFFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_dataLoader.exposeParameters(moduleParamList, prefix);
  m_treeSearchFindlet.exposeParameters(moduleParamList, prefix);
  m_storeArrayHandler.exposeParameters(moduleParamList, prefix);
  m_overlapResolver.exposeParameters(moduleParamList, prefix);
  m_spacePointTagger.exposeParameters(moduleParamList, prefix);
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

  m_treeSearchFindlet.apply(m_cdcRecoTrackVector, m_spacePointVector, m_results);

  m_overlapResolver.apply(m_results);

  m_storeArrayHandler.apply(m_results);
  m_spacePointTagger.apply(m_results, m_spacePointVector);
}
