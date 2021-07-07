/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
