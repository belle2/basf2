/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/ca/Path.h>

#include <vector>
#include <string>

namespace Belle2 {


  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCFacet;

    /// Findlet that generates segments within clusters based on a cellular automaton on triples of hits
    class SegmentCreatorFacetAutomaton
      : public Findlet<const CDCFacet, const WeightedRelation<const CDCFacet>, CDCSegment2D> {

    private:
      /// Type of the base class
      using Super =
        Findlet<const CDCFacet, const WeightedRelation<const CDCFacet>, CDCSegment2D>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main function of the segment finding by the cellular automaton.
      void apply(const std::vector<CDCFacet>& inputFacets,
                 const std::vector<WeightedRelation<const CDCFacet>>& inputFacetRelations,
                 std::vector<CDCSegment2D>& outputSegments) final;

    private:
      /// Parameter : Switch to construct the reversed segment if it is available in the facet graph as well
      bool m_param_searchReversed = false;

      /// Parameter : Switch to construct the alias segment if it is available in the facet graph as well
      bool m_param_searchAlias = true;

      /// Parameter : Switch to relax the alias and reverse search for segments contained in a single layer
      bool m_param_relaxSingleLayerSearch = true;

      /// Paraneter : Switch to activate the write out of all available orientations of single facet segments
      bool m_param_allSingleAliases = false;

    private: // cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCFacet> m_cellularPathFinder;

    private: // object pools
      /// Memory for the facet paths generated from the graph.
      std::vector< Path<const CDCFacet> > m_facetPaths;
    };
  }
}
