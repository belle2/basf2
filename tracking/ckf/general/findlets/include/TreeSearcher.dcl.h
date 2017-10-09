/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /**
   */
  template <class AState, class AStateRejecter, class AResult>
  class TreeSearcher : public TrackFindingCDC::Findlet<const AState, const TrackFindingCDC::WeightedRelation<AState>, AResult> {
  private:
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const AState, const TrackFindingCDC::WeightedRelation<AState>, AResult>;

  public:
    /// Construct this findlet and add the subfindlet as listener
    TreeSearcher();

    /// Expose the parameters of the subfindlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

    /**
     * Main function of this findlet: traverse a tree starting from a given seed states.
     *
     * ATTENTION: As described above, the states themselves can be altered during the tree
     * traversal.
     */
    void apply(const std::vector<AState>& seededStates,
               const std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations,
               std::vector<AResult>& results) final;

  private:
    /// Implementation of the traverseTree function
    void traverseTree(std::vector<const AState*>& path,
                      const std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations,
                      std::vector<AResult>& results);

  private:
    /// State rejecter to decide which available continuations should be traversed next.
    AStateRejecter m_stateRejecter;
  };
}
