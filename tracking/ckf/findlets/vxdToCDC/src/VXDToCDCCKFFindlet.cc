/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/findlets/vxdToCDC/VXDToCDCCKFFindlet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

VXDToCDCCKFFindlet::VXDToCDCCKFFindlet()
{
  addProcessingSignalListener(&m_tracksLoader);
  addProcessingSignalListener(&m_trackFitter);
  addProcessingSignalListener(&m_treeSearchFindlet);
  addProcessingSignalListener(&m_overlapResolver);
  addProcessingSignalListener(&m_storeArrayHandler);
  addProcessingSignalListener(&m_wireHitTagger);
}

void VXDToCDCCKFFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_tracksLoader.exposeParameters(moduleParamList, prefix);
  m_trackFitter.exposeParameters(moduleParamList, prefix);
  m_treeSearchFindlet.exposeParameters(moduleParamList, prefix);
  m_overlapResolver.exposeParameters(moduleParamList, prefix);
  m_storeArrayHandler.exposeParameters(moduleParamList, prefix);
  m_wireHitTagger.exposeParameters(moduleParamList, prefix);
}

void VXDToCDCCKFFindlet::beginEvent()
{
  Super::beginEvent();

  m_vxdRecoTrackVector.clear();
  m_rlWireHitVector.clear();
  m_rlWireHitPointerVector.clear();
  m_results.clear();
}

void VXDToCDCCKFFindlet::apply(std::vector<CDCWireHit>& wireHits)
{
  m_tracksLoader.apply(m_vxdRecoTrackVector);
  m_trackFitter.apply(m_vxdRecoTrackVector);

  // TODO: This can also be in its own findlet
  m_rlWireHitVector.reserve(wireHits.size());
  for (const CDCWireHit& wireHit : wireHits) {
    if (not wireHit->hasTakenFlag()) {
      for (ERightLeft rlInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
        m_rlWireHitVector.emplace_back(&wireHit, rlInfo);
      }
    }
  }

  // TODO for Nils: maybe it is better to not use pointer as the underlaying hit type
  m_rlWireHitPointerVector.reserve(m_rlWireHitVector.size());

  for (const CDCRLWireHit& rlWireHit : m_rlWireHitVector) {
    m_rlWireHitPointerVector.push_back(&rlWireHit);
  }

  m_treeSearchFindlet.apply(m_vxdRecoTrackVector, m_rlWireHitPointerVector, m_results);
  m_overlapResolver.apply(m_results);

  m_storeArrayHandler.apply(m_results);
  m_wireHitTagger.apply(m_results);
}
