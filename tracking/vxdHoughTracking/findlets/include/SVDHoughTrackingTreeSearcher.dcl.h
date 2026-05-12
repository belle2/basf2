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

namespace Belle2 {
  class ModuleParamList;

  namespace vxdHoughTracking {

    /**
    * Findlet for constructing result paths out of a list of hits, which are connected
    * with weighted relations. At each step, the hits are again tested using a state rejector,
    * which also knows the current path of hits.
    *
    * This rejector is allowed to alter the hits, so using a cellular automaton it is assured,
    * that the hits are traversed in the correct order without overriding each other.
    * It is however crucial, that the relations do not create cycles in the graph!
    */
    template <class AHit, class APathFilter, class AResult>
    class SVDHoughTrackingTreeSearcher : public
      TrackingUtilities::Findlet<AHit*, const TrackingUtilities::WeightedRelation<AHit>, AResult> {
    private:
      /// Parent class
      using Super = TrackingUtilities::Findlet<AHit*, const TrackingUtilities::WeightedRelation<AHit>, AResult>;

    public:
      /// Construct this findlet and add the subfindlet as listener
      SVDHoughTrackingTreeSearcher();

      /// Expose the parameters of the subfindlet
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /**
      * Main function of this findlet: traverse a tree starting from a given seed hits.
      *
      * ATTENTION: As described above, the hits themselves can be altered during the tree
      * traversal.
      */
      void apply(std::vector<AHit*>& hits,
                 const std::vector<TrackingUtilities::WeightedRelation<AHit>>& relations,
                 std::vector<AResult>& results) final;

    private:
      /// Implementation of the traverseTree function
      void traverseTree(std::vector<TrackingUtilities::WithWeight<const AHit*>>& path,
                        const std::vector<TrackingUtilities::WeightedRelation<AHit>>& relations,
                        std::vector<AResult>& results);

    private:
      /// State rejecter to decide which available continuations should be traversed next.
      APathFilter m_pathFilter;

      /// Findlet for adding a recursion cell state to the hits
      TrackingUtilities::CellularAutomaton<AHit> m_automaton;

      /// TwoHitFilter activarion cut
      uint m_applyTwoHitFilterIfMoreChildStates = 50;
    };

  }
}
