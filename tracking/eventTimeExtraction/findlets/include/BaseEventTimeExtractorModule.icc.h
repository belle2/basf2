/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractorModule.dcl.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  template<class AFindlet>
  BaseEventTimeExtractorModuleFindlet<AFindlet>::BaseEventTimeExtractorModuleFindlet()
  {
    addProcessingSignalListener(&m_findlet);
    addProcessingSignalListener(&m_trackSelector);
  }

  template<class AFindlet>
  void BaseEventTimeExtractorModuleFindlet<AFindlet>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    m_trackSelector.exposeParameters(moduleParamList, prefix);
    m_findlet.exposeParameters(moduleParamList, prefix);
  }

  template<class AFindlet>
  void BaseEventTimeExtractorModuleFindlet<AFindlet>::apply(std::vector<RecoTrack*>& recoTracks)
  {
    std::vector<RecoTrack*> copiedRecoTracks = recoTracks;
    m_trackSelector.apply(copiedRecoTracks);
    m_findlet.apply(copiedRecoTracks);

    for (RecoTrack* recoTrack : recoTracks) {
      recoTrack->setDirtyFlag();
    }
  }
}
