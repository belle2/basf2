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
#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentPairAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackFlightTimeAdjuster.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackExporter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Complete findlet implementing track finding with the cellular automaton in two stages.
    class TrackFinderAutomaton : public Findlet<> {

    private:
      /// Type of the base class
      using Super = Findlet<>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      TrackFinderAutomaton();

      /// Short description of the findlet
      std::string getDescription() override;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Signal the beginning of a new event
      void beginEvent() final;

      /// Execute the findlet
      void apply() final;

    private:
      // Findlets
      /// Preparation findlet creating the wire hits from the packed CDCHits
      WireHitPreparer m_wireHitPreparer;

      /// Preparation findlet creating the clusters wire hits forming locally connected groups
      ClusterPreparer m_clusterPreparer;

      /// First stage cellular automaton segment finder
      SegmentFinderFacetAutomaton m_segmentFinderFacetAutomaton;

      /// Second stage cellular automaton track finder from segments
      TrackFinderSegmentPairAutomaton m_trackFinderSegmentPairAutomaton;

      /// Adjusts the flight time of the tracks to a setable trigger point
      TrackFlightTimeAdjuster m_trackFlightTimeAdjuster;

      /// Exports the generated CDCTracks as RecoTracks.
      TrackExporter m_trackExporter;

      /// Puts the internal segments on the DataStore
      StoreVectorSwapper<CDCWireHit, true> m_wireHitsSwapper{"CDCWireHitVector"};

      /// Puts the internal segments on the DataStore
      StoreVectorSwapper<CDCSegment2D> m_segmentsSwapper{"CDCSegment2DVector"};

      /// Puts the internal segments on the DataStore
      StoreVectorSwapper<CDCTrack> m_tracksSwapper{"CDCTrackVector"};

      // Object pools
      /// Memory for the wire hits
      std::vector<CDCWireHit> m_wireHits;

      /// Memory for the wire hits cluster
      std::vector<CDCWireHitCluster> m_clusters;

      /// Memory for the wire hits super clusters
      std::vector<CDCWireHitCluster> m_superClusters;

      /// Memory for the segments
      std::vector<CDCSegment2D> m_segments;

      /// Memory for the tracks
      std::vector<CDCTrack> m_tracks;
    };
  }
}
