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

#include <tracking/trackFindingCDC/findlets/combined/WireHitTopologyPreparer.h>
#include <tracking/trackFindingCDC/findlets/combined/SegmentFinderFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentPairAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackFlightTimeAdjuster.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackExporter.h>

#include <tracking/trackFindingCDC/filters/cluster/ChooseableClusterFilter.h>
#include <tracking/trackFindingCDC/filters/facet/ChooseableFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/ChooseableFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/ChooseableSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/ChooseableSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/ChooseableSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/ChooseableTrackRelationFilter.h>

#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Complete findlet implementing track finding with the cellular automaton in two stages.
    class TrackFinderAutomaton
      : public Findlet<> {

    private:
      /// Type of the base class
      using Super = Findlet<>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      TrackFinderAutomaton();

      /// Short description of the findlet
      virtual std::string getDescription() override;

      /// Expose the parameters of the cluster filter to a module
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix = "") override;

      /// Generates the segment.
      virtual void apply() override final;

    private:
      // Findlets
      /// Preparation findlet creating the wire hits from the packed CDCHits
      WireHitTopologyPreparer m_wireHitTopologyPreparer;

      /// First stage cellular automaton segment finder
      SegmentFinderFacetAutomaton<ChooseableClusterFilter,
                                  ChooseableFacetFilter,
                                  ChooseableFacetRelationFilter,
                                  ChooseableSegmentRelationFilter> m_segmentFinderFacetAutomaton;

      /// First stage cellular automaton track finder from segments
      TrackFinderSegmentPairAutomaton<ChooseableSegmentPairFilter,
                                      ChooseableSegmentPairRelationFilter,
                                      ChooseableTrackRelationFilter> m_trackFinderSegmentPairAutomaton;

      /// Adjusts the flight time of the tracks to a setable trigger point
      TrackFlightTimeAdjuster m_trackFlightTimeAdjuster;

      /// Exports the generated CDCTracks as track candidates to be fitted by Genfit.
      TrackExporter m_trackExporter;

      /// Puts the internal segments on the DataStore
      StoreVectorSwapper<CDCWireHit, true> m_wireHitsSwapper{"CDCWireHitVector"};

      /// Puts the internal segments on the DataStore
      StoreVectorSwapper<CDCRecoSegment2D> m_segmentsSwapper{"CDCRecoSegment2DVector"};

      /// Puts the internal segments on the DataStore
      StoreVectorSwapper<CDCTrack> m_tracksSwapper{"CDCTrackVector"};

    };

  }
}
