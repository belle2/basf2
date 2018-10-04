/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/cdc/findlets/CKFToCDCFindlet.h>
#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.h>

#include <tracking/ckf/general/utilities/ClassMnemomics.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CKFToCDCFindlet::~CKFToCDCFindlet() = default;

CKFToCDCFindlet::CKFToCDCFindlet()
{
  addProcessingSignalListener(&m_trackHandler);
  addProcessingSignalListener(&m_treeSearcher);
}

void CKFToCDCFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_trackHandler.exposeParameters(moduleParamList, prefix);
}

void CKFToCDCFindlet::beginEvent()
{
  Super::beginEvent();

  m_vxdRecoTrackVector.clear();
}

void CKFToCDCFindlet::apply(const std::vector<TrackFindingCDC::CDCWireHit>& wireHits)
{
  m_trackHandler.apply(m_vxdRecoTrackVector);

  const auto& wireHitPtrs = TrackFindingCDC::as_pointers<const TrackFindingCDC::CDCWireHit>(wireHits);

  // Do the tree search
  for (const RecoTrack* recoTrack : m_vxdRecoTrackVector) {
    std::vector<CDCCKFPath> paths;

    CDCCKFState seedState(recoTrack, recoTrack->getMeasuredStateOnPlaneFromLastHit());
    paths.push_back({seedState});
    m_treeSearcher.apply(paths, wireHitPtrs);

    // TODO: do something with the paths
  }

  // TODO: write out the path candidates
}
