/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackingUtilities/utilities/WeightedRelation.h>
#include <tracking/trackingUtilities/numerics/WithWeight.h>

#include <tracking/trackingUtilities/ca/CellularAutomaton.h>

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
    TrackingUtilities::Findlet<const AState, AState, const TrackingUtilities::WeightedRelation<AState>, AResult> {
  private:
    /// Parent class
    using Super = TrackingUtilities::Findlet<const AState, AState, const TrackingUtilities::WeightedRelation<AState>, AResult>;

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
               const std::vector<TrackingUtilities::WeightedRelation<AState>>& relations,
               std::vector<AResult>& results) final;

  private:
    /// Implementation of the traverseTree function
    void traverseTree(std::vector<TrackingUtilities::WithWeight<const AState*>>& path,
                      const std::vector<TrackingUtilities::WeightedRelation<AState>>& relations,
                      std::vector<AResult>& results);

  private:
    /// State rejecter to decide which available continuations should be traversed next.
    AStateRejecter m_stateRejecter;

    /// Findlet for adding a recursion cell state to the states
    TrackingUtilities::CellularAutomaton<AState> m_automaton;

    /// Parameter: Make it possible to have all subresults in the end results vector.
    bool m_param_endEarly = true;
  };
}
