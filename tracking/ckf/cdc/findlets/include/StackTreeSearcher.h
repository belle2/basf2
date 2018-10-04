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
  class StackTreeSearcher : public TrackFindingCDC::Findlet<CDCCKFPath, const TrackFindingCDC::CDCWireHit* const> {
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
      m_stateFilter.exposeParameters(moduleParamList, prefix);
      m_pathMerger.exposeParameters(moduleParamList, prefix);
      m_pathSelector.exposeParameters(moduleParamList, prefix);
    }

    void apply(std::vector<CDCCKFPath>& paths,
               const std::vector<const TrackFindingCDC::CDCWireHit*>& wireHits) override
    {

      if (paths.empty()) {
        return;
      }

      std::vector<CDCCKFPath> newPaths;
      std::vector<CDCCKFState> nextStates;

      for (CDCCKFPath& path : paths) {
        B2DEBUG(100, "Testing one path...");
        if (path.back().isSeed()) {
          B2DEBUG(100, "From seed");
        } else {
          const auto& wire = path.back().getWireHit()->getWire();
          B2DEBUG(100, wire.getICLayer() << " " << wire.getIWire());
        }
        m_stateCreator.apply(nextStates, path, wireHits);
        m_stateFilter.apply(path, nextStates);

        // TODO: Attention: if there is no hit anymore, the path will not be added to the final set!
        for (const auto& nextState : nextStates) {
          path.push_back(nextState);

          const auto& nextWire = nextState.getWireHit()->getWire();
          B2DEBUG(100, "will go to " << nextWire.getICLayer() << " " << nextWire.getIWire());
          newPaths.push_back(path);
          path.pop_back();
        }
        B2DEBUG(100, "Now having " << newPaths.size() << " in flight");
        nextStates.clear();
      }

      B2DEBUG(100, "Having found " << newPaths.size() << " new paths");

      m_pathMerger.apply(newPaths);
      B2DEBUG(100, "Having found " << newPaths.size() << " new paths after merging");

      m_pathSelector.apply(newPaths);
      B2DEBUG(100, "Having found " << newPaths.size() << " new paths after selection");

      if (newPaths.empty()) {
        return;
      }

      paths.swap(newPaths);
      newPaths.clear();

      apply(paths, wireHits);
    }

  private:
    CDCCKFStateCreator m_stateCreator;
    CDCCKFStateFilter m_stateFilter;
    CDCCKFPathMerger m_pathMerger;
    CDCCKFPathSelector m_pathSelector;
  };
}
