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
#include <tracking/trackFindingCDC/numerics/WithWeight.h>

#include <tracking/trackFindingCDC/ca/CellularAutomaton.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  /**
   * Findlet for constructing result paths out of a list of states, which are connected
   * with weighted relations. At each step, the states are again tested using a state rejector,
   * which also knows the current path of states.
   *
   * This rejector is allowed to alter the states, so using a cellular automaton it is assured,
   * that the states are traversed in the correct order without overriding each other.
   * It is however crucial, that the relations do not create cycles in the graph!
   */
  template <class AState, class AStateRejecter, class AResult>
  class TreeSearcher : public
    TrackFindingCDC::Findlet<const AState, AState, const TrackFindingCDC::WeightedRelation<AState>, AResult> {
  private:
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const AState, AState, const TrackFindingCDC::WeightedRelation<AState>, AResult>;

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
               std::vector<AState>& hitStates,
               const std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations,
               std::vector<AResult>& results) final;

  private:
    /// Implementation of the traverseTree function
    void traverseTree(std::vector<TrackFindingCDC::WithWeight<const AState*>>& path,
                      const std::vector<TrackFindingCDC::WeightedRelation<AState>>& relations,
                      std::vector<AResult>& results);

  private:
    /// State rejecter to decide which available continuations should be traversed next.
    AStateRejecter m_stateRejecter;

    /// Findlet for adding a recursion cell state to the states
    TrackFindingCDC::CellularAutomaton<AState> m_automaton;

    /// Parameter: Make it possible to have all subresults in the end results vector.
    bool m_param_endEarly = true;
  };
}
