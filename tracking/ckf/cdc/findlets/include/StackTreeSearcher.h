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

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>


namespace Belle2 {
  class StackTreeSearcher : public
    TrackFindingCDC::Findlet<CDCCKFPath, const TrackFindingCDC::WeightedRelation<TrackFindingCDC::CDCWireHit>> {
  public:
    StackTreeSearcher()
    {
      addProcessingSignalListener(&m_stateCreator);
      addProcessingSignalListener(&m_stateFilter);
      addProcessingSignalListener(&m_pathMerger);
      addProcessingSignalListener(&m_pathSelector);
    }

    void apply(std::vector<CDCCKFPath>& paths,
               const std::vector<TrackFindingCDC::WeightedRelation<TrackFindingCDC::CDCWireHit>>& relations)
    {

      std::vector<CDCCKFPath> newPaths;
      std::vector<CDCCKFState> nextStates;

      for (CDCCKFPath& path : paths) {
        nextStates.clear();
        m_stateCreator.apply(nextStates, path, relations);
        m_stateFilter.apply(path, nextStates);

        for (const auto& nextState : nextStates) {
          path.push_back(nextState);
          newPaths.push_back(path);
          path.pop_back();
        }
      }

      m_pathMerger.apply(newPaths);
      m_pathSelector.apply(newPaths);

      if (newPaths.empty()) {
        return;
      }

      paths.swap(newPaths);
      apply(paths, relations);
    }

  private:
    CDCCKFStateCreator m_stateCreator;
    CDCCKFStateFilter m_stateFilter;
    CDCCKFPathMerger m_pathMerger;
    CDCCKFPathSelector m_pathSelector;
  };
}
