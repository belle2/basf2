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

#include <tracking/trackFindingCDC/creators/TrackCreator.h>
#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>
#include <tracking/trackFindingCDC/ca/Path.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <vector>
#include <iterator>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Findlet that generates tracks based on a cellular automaton of segment triples
    template<class ASegmentTripleRelationFilter>
    class TrackCreatorSegmentTripleAutomaton :
      public Findlet<const CDCSegmentTriple, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegmentTriple, CDCTrack>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      TrackCreatorSegmentTripleAutomaton()
      {
        addProcessingSignalListener(&m_segmentTripleRelationFilter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Constructs tracks by extraction of segment triple paths in a cellular automaton.";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList,
                            const std::string& prefix = "") override final
      {
        m_segmentTripleRelationFilter.exposeParameters(moduleParamList, prefix);
      }

      /// Main function of the segment finding by the cellular automaton.
      virtual void apply(const std::vector<CDCSegmentTriple>& inputSegmentTriples,
                         std::vector<CDCTrack>& outputTracks) override final
      {
        // Create the segment triple neighborhood
        B2DEBUG(100, "Creating the CDCSegmentTriple neighborhood");
        m_segmentTripleNeighborhood.clear();
        m_segmentTripleNeighborhood.appendUsing(m_segmentTripleRelationFilter, inputSegmentTriples);
        B2DEBUG(100, "  Created " << m_segmentTripleNeighborhood.size()  << " AxialStereoTripleNeighborhoods");
        // Multiple passes if growMany is active and one track is created at a time
        // No best candidate analysis needed
        m_segmentTriplePaths.clear();
        m_cellularPathFinder.apply(inputSegmentTriples, m_segmentTripleNeighborhood, m_segmentTriplePaths);
        B2DEBUG(100, "  Created " << m_segmentTriplePaths.size()  << " SegmentTripleTracks");

        // Reduce to plain tracks
        m_trackCreator.create(m_segmentTriplePaths, outputTracks);
      }

    private:
      /// Reference to the relation filter to be used to construct the segmentTriple network.
      ASegmentTripleRelationFilter m_segmentTripleRelationFilter;

    private: // object pools
      /// Memory for the segmentTriple neighborhood.
      WeightedNeighborhood<const CDCSegmentTriple> m_segmentTripleNeighborhood;

      /// Memory for the segmentTriple paths generated from the graph.
      std::vector< Path<const CDCSegmentTriple> > m_segmentTriplePaths;

    private:
      //cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCSegmentTriple> m_cellularPathFinder;

      // Deprication:
      /// Instance of the track creator from paths.
      TrackCreator m_trackCreator;
    }; // end class SegmentCreator


  } //end namespace TrackFindingCDC
} //end namespace Belle2
