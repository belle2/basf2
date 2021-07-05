/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/ckf/cdc/findlets/CDCCKFStateCreator.h>
#include <tracking/ckf/cdc/findlets/CDCCKFStateFilter.h>
#include <tracking/ckf/cdc/findlets/CDCCKFPathMerger.h>
#include <tracking/ckf/cdc/findlets/CDCCKFPathSelector.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>


namespace Belle2 {
  /// CKF tree searcher which traces several best paths.
  class StackTreeSearcher : public
    TrackFindingCDC::Findlet<CDCCKFPath, const TrackFindingCDC::CDCWireHit* const> {
  public:
    StackTreeSearcher()
    {
      addProcessingSignalListener(&m_stateCreator);
      addProcessingSignalListener(&m_stateFilter);
      addProcessingSignalListener(&m_pathMerger);
      addProcessingSignalListener(&m_pathSelector);
    }

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      m_stateCreator.exposeParameters(moduleParamList, prefix);
      m_stateFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("state", prefix));
      m_pathMerger.exposeParameters(moduleParamList, prefix);
      m_pathSelector.exposeParameters(moduleParamList, TrackFindingCDC::prefixed("path", prefix));
    }

    /// Main method to update the paths. Input: vector of the selected paths and a vector of CDC wirehits to be considered.
    void apply(std::vector<CDCCKFPath>& paths,
               const std::vector<const TrackFindingCDC::CDCWireHit*>& wireHits) override
    {

      if (paths.empty()) {
        return;
      }

      std::vector<CDCCKFPath> newPaths;
      std::vector<CDCCKFState> nextStates;

      for (CDCCKFPath& path : paths) {
        B2DEBUG(29, "Testing one path from " << path.back());
        m_stateCreator.apply(nextStates, path, wireHits);
        m_stateFilter.apply(path, nextStates);

        // TODO: Attention: if there is no hit anymore, the path will not be added to the final set!
        for (const auto& nextState : nextStates) {
          path.push_back(nextState);

          B2DEBUG(29, "will go to " << nextState);
          newPaths.push_back(path);
          path.pop_back();
        }
        B2DEBUG(29, "Now having " << newPaths.size() << " in flight");
        nextStates.clear();
      }

      B2DEBUG(29, "Having found " << newPaths.size() << " new paths");
      for (const auto& path : newPaths) {
        B2DEBUG(29, path);
      }

      m_pathMerger.apply(newPaths);
      B2DEBUG(29, "Having found " << newPaths.size() << " new paths after merging");
      for (const auto& path : newPaths) {
        B2DEBUG(29, path);
      }

      m_pathSelector.apply(newPaths);
      B2DEBUG(29, "Having found " << newPaths.size() << " new paths after selection");
      for (const auto& path : newPaths) {
        B2DEBUG(29, path);
      }

      if (newPaths.empty()) {
        return;
      }

      paths.swap(newPaths);
      newPaths.clear();

      apply(paths, wireHits);
    }

  private:
    /// algorthim to create CDC-CDF states while traversing the path
    CDCCKFStateCreator m_stateCreator;
    /// algorithm to perform state filtering
    CDCCKFStateFilter m_stateFilter;
    /// algorithm to merge similar paths
    CDCCKFPathMerger m_pathMerger;
    /// algorithm to select N best paths, for further processing.
    CDCCKFPathSelector m_pathSelector;
  };
}
