/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/combined/WireHitPreparer.h>
#include <tracking/trackFindingCDC/findlets/combined/ClusterPreparer.h>
#include <tracking/trackFindingCDC/findlets/combined/SegmentFinderFacetAutomaton.h>

#include <tracking/trackFindingCDC/findlets/combined/AxialTrackFinderLegendre.h>

#include <tracking/trackFindingCDC/findlets/combined/StereoHitFinder.h>
#include <tracking/trackFindingCDC/findlets/combined/SegmentTrackCombiner.h>

#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentPairAutomaton.h>

#include <tracking/trackFindingCDC/findlets/minimal/TrackQualityAsserter.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSingleSegments.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCombiner.h>

#include <tracking/trackFindingCDC/findlets/minimal/TrackExporter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Complete findlet implementing track finding with the cellular automaton in two stages.
    class TrackFinder : public Findlet<> {

    private:
      /// Type of the base class
      using Super = Findlet<>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      TrackFinder();

      /// Short description of the findlet
      std::string getDescription() override;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Execute the findlet
      void apply() final;

    private:
      /// Parameter: Activate the combination of the local segment linking
      bool m_param_withCA = false;

    private:
      // Findlets
      /// Preparation findlet creating the wire hits from the packed CDCHits
      WireHitPreparer m_wireHitPreparer;

      /// Preparation findlet creating the clusters wire hits forming locally connected groups
      ClusterPreparer m_clusterPreparer;

      /// First stage cellular automaton segment finder
      SegmentFinderFacetAutomaton m_segmentFinderFacetAutomaton;

      /// Axial track finder
      AxialTrackFinderLegendre m_axialTrackFinderLegendre;

      /// Improve the quality of the axial tracks
      TrackQualityAsserter m_trackQualityAsserter;

      /// Assoziate the stereo hits that best match to the axial tracks
      StereoHitFinder m_stereoHitFinder;

      /// Join the matching segments into the tracks
      SegmentTrackCombiner m_segmentTrackCombiner;

      /// Second stage cellular automaton track finder from segments
      TrackFinderSegmentPairAutomaton m_trackFinderSegmentPairAutomaton;

      /// Combine the tracks from the global search with the tracks from the local search
      TrackCombiner m_trackCombiner;

      /// Final track quality assertions
      TrackQualityAsserter m_finalTrackQualityAsserter;

      /// Add tracks from the first super layer that are contained with in the first super layer.
      TrackCreatorSingleSegments m_trackCreatorSingleSegments;

      /// Exports the generated CDCTracks as RecoTracks.
      TrackExporter m_trackExporter;
    };
  }
}
