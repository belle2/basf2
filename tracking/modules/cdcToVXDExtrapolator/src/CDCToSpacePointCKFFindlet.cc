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
  addProcessingSignalListener(&m_treeSearchFindlet);
  addProcessingSignalListener(&m_storeArrayHandler);
  addProcessingSignalListener(&m_overlapResolver);
}

void CDCToSpacePointCKFFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_treeSearchFindlet.exposeParameters(moduleParamList, prefix);
  m_storeArrayHandler.exposeParameters(moduleParamList, prefix);
  m_overlapResolver.exposeParameters(moduleParamList, prefix);
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
  m_storeArrayHandler.fetch(m_cdcRecoTrackVector, m_spacePointVector);

  m_treeSearchFindlet.apply(m_cdcRecoTrackVector, m_spacePointVector, m_results);

  // Remove all empty results
  erase_remove_if(m_results, [](const CKFCDCToVXDStateObject::ResultObject & result) {
    return result.second.empty();
  });

  B2INFO("Found " << m_results.size() << " tracks");

  m_overlapResolver.apply(m_results);
  B2INFO("After overlap resolving: Found " << m_results.size() << " tracks");

  m_storeArrayHandler.apply(m_results);
}
