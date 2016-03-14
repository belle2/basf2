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
#include <tracking/trackFindingCDC/findlets/minimal/TrackExporter.h>

#include <tracking/trackFindingCDC/filters/cluster/ChooseableClusterFilter.h>
#include <tracking/trackFindingCDC/filters/facet/ChooseableFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facetRelation/ChooseableFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentRelation/ChooseableSegmentRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/ChooseableSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/ChooseableSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/ChooseableTrackRelationFilter.h>

#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Complete findlet implementing track finding with the cellular automaton in two stages.
    class TrackFinderAutomaton
      : public Findlet<> {

    private:
      /// Type of the base class
      typedef Findlet<> Super;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      TrackFinderAutomaton()
      {
        addProcessingSignalListener(&m_wireHitsSwapper);
        addProcessingSignalListener(&m_segmentsSwapper);
        addProcessingSignalListener(&m_tracksSwapper);

        addProcessingSignalListener(&m_wireHitTopologyPreparer);
        addProcessingSignalListener(&m_segmentFinderFacetAutomaton);
        addProcessingSignalListener(&m_trackFinderSegmentPairAutomaton);
        addProcessingSignalListener(&m_trackExporter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Performs patter recognition in the CDC based on local hit following and application of a cellular automaton in two stages.";
      }

      /// Expose the parameters of the cluster filter to a module
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix = "") override
      {
        m_wireHitsSwapper.exposeParameters(moduleParamList, prefix);
        m_segmentsSwapper.exposeParameters(moduleParamList, prefix);
        m_tracksSwapper.exposeParameters(moduleParamList, prefix);

        m_wireHitTopologyPreparer.exposeParameters(moduleParamList, prefix);
        m_segmentFinderFacetAutomaton.exposeParameters(moduleParamList, prefix);
        m_trackFinderSegmentPairAutomaton.exposeParameters(moduleParamList, prefix);
        m_trackExporter.exposeParameters(moduleParamList, prefix);

        moduleParamList->getParameter<std::string>("SegmentOrientation").setDefaultValue("symmetric");
        moduleParamList->getParameter<std::string>("SegmentOrientation").resetValue();

        moduleParamList->getParameter<std::string>("TrackOrientation").setDefaultValue("outwards");
        moduleParamList->getParameter<std::string>("TrackOrientation").resetValue();

        // Mimics earlier behaviour
        moduleParamList->getParameter<bool>("WriteSegments").setDefaultValue(true);
        moduleParamList->getParameter<bool>("WriteSegments").resetValue();
      }

      /// Generates the segment.
      virtual void apply() override final
      {
        std::vector<CDCWireHit> wireHits;
        std::vector<CDCRecoSegment2D> segments;
        std::vector<CDCTrack> tracks;

        // Aquire the wire hits, segments and tracks from the DataStore
        m_wireHitsSwapper.apply(wireHits);
        m_segmentsSwapper.apply(segments);
        m_tracksSwapper.apply(tracks);

        wireHits.reserve(1000);
        segments.reserve(100);
        tracks.reserve(20);

        m_wireHitTopologyPreparer.apply(wireHits);
        m_segmentFinderFacetAutomaton.apply(wireHits, segments);
        m_trackFinderSegmentPairAutomaton.apply(segments, tracks);
        m_trackExporter.apply(tracks);

        // Put the segments and tracks on the DataStore
        m_wireHitsSwapper.apply(wireHits);
        m_segmentsSwapper.apply(segments);
        m_tracksSwapper.apply(tracks);
      }

    private:
      // Findlets
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
      /// Exports the generated CDCTracks as track candidates to be fitted by Genfit.
      TrackExporter m_trackExporter;

      /// Puts the internal segments on the DataStore
      StoreVectorSwapper<CDCWireHit, true> m_wireHitsSwapper{"CDCWireHitVector"};

      /// Puts the internal segments on the DataStore
      StoreVectorSwapper<CDCRecoSegment2D> m_segmentsSwapper{"CDCRecoSegment2DVector"};

      /// Puts the internal segments on the DataStore
      StoreVectorSwapper<CDCTrack> m_tracksSwapper{"CDCTrackVector"};

    }; // end class TrackFinderAutomaton

  } //end namespace TrackFindingCDC
} //end namespace Belle2
