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
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <vector>
#include <iterator>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Findlet that generates tracks based on a cellular automaton of segment pairs
    template<class ASegmentPairRelationFilter>
    class TrackCreatorSegmentPairAutomaton :
      public Findlet<const CDCSegmentPair, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegmentPair, CDCTrack>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      TrackCreatorSegmentPairAutomaton()
      {
        addProcessingSignalListener(&m_segmentPairRelationFilter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Constructs tracks by extraction of segment pair paths in a cellular automaton.";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList,
                            const std::string& prefix = "") override final
      {
        m_segmentPairRelationFilter.exposeParameters(moduleParamList, prefix);
      }

      /// Main function of the segment finding by the cellular automaton.
      virtual void apply(const std::vector<CDCSegmentPair>& inputSegmentPairs,
                         std::vector<CDCTrack>& outputTracks) override final
      {
        // Create the segment pair neighborhood
        B2DEBUG(100, "Creating the CDCSegmentPair neighborhood");
        m_segmentPairNeighborhood.clear();
        m_segmentPairNeighborhood.appendUsing(m_segmentPairRelationFilter, inputSegmentPairs);
        B2DEBUG(100, "  Created " << m_segmentPairNeighborhood.size()  << " AxialStereoPairNeighborhoods");
        // Multiple passes if growMany is active and one track is created at a time
        // No best candidate analysis needed
        m_segmentPairPaths.clear();
        m_cellularPathFinder.apply(inputSegmentPairs, m_segmentPairNeighborhood, m_segmentPairPaths);
        B2DEBUG(100, "  Created " << m_segmentPairPaths.size()  << " SegmentTripleTracks");

        // Reduce to plain tracks
        m_trackCreator.create(m_segmentPairPaths, outputTracks);
      }

    private:
      /// Reference to the relation filter to be used to construct the segmentPair network.
      ASegmentPairRelationFilter m_segmentPairRelationFilter;

    private: // object pools
      /// Memory for the segmentPair neighborhood.
      WeightedNeighborhood<const CDCSegmentPair> m_segmentPairNeighborhood;

      /// Memory for the segmentPair paths generated from the graph.
      std::vector< Path<const CDCSegmentPair> > m_segmentPairPaths;

    private:
      //cellular automaton
      /// Instance of the cellular automaton path finder
      MultipassCellularPathFinder<const CDCSegmentPair> m_cellularPathFinder;

      // Deprication:
      /// Instance of the track creator from paths.
      TrackCreator m_trackCreator;
    }; // end class SegmentCreator


  } //end namespace TrackFindingCDC
} //end namespace Belle2
