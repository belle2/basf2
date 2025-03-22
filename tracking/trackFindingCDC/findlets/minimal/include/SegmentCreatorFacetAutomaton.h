/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackingUtilities/ca/MultipassCellularPathFinder.h>
#include <tracking/trackingUtilities/utilities/WeightedRelation.h>
#include <tracking/trackingUtilities/ca/Path.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCFacet;

    /// Findlet that generates segments within clusters based on a cellular automaton on triples of hits
    class SegmentCreatorFacetAutomaton
      : public TrackingUtilities::Findlet<const CDCFacet, const TrackingUtilities::WeightedRelation<const CDCFacet>, CDCSegment2D> {

    private:
      /// Type of the base class
      using Super =
        TrackingUtilities::Findlet<const CDCFacet, const TrackingUtilities::WeightedRelation<const CDCFacet>, CDCSegment2D>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main function of the segment finding by the cellular automaton.
      void apply(const std::vector<CDCFacet>& inputFacets,
                 const std::vector<TrackingUtilities::WeightedRelation<const CDCFacet>>& inputFacetRelations,
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
      TrackingUtilities::MultipassCellularPathFinder<const CDCFacet> m_cellularPathFinder;

    private: // object pools
      /// Memory for the facet paths generated from the graph.
      std::vector< TrackingUtilities::Path<const CDCFacet> > m_facetPaths;
    };
  }
}
