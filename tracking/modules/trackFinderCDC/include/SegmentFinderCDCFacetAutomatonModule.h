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

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

#include <tracking/trackFindingCDC/filters/cluster/ChooseableClusterFilter.h>
#include <tracking/trackFindingCDC/filters/facet/ChooseableFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet_relation/ChooseableFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segment_relation/ChooseableSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/findlets/combined/SegmentFinderFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Generates segments from hits using a cellular automaton build from hit triples (facets).
    class SegmentFinderCDCFacetAutomatonModule :
      public virtual TrackFinderCDCBaseModule {

    private:
      /// Type of the base class
      using Super = TrackFinderCDCBaseModule;

    public:
      /// Default constructor initialising the filters with the default settings
      SegmentFinderCDCFacetAutomatonModule();

      /// Initialize the Module before event processing
      virtual void initialize() override;

      /// Signal the start of a new run.
      virtual void beginRun() override;

      /// Processes the current event
      virtual void event() override;

      /// Signal the end of a run
      virtual void endRun() override;

      /// Signal the termination of the event processing.
      virtual void terminate() override;

    private:
      /// Findlet implementing the algorithm
      SegmentFinderFacetAutomaton<ChooseableClusterFilter,
                                  ChooseableFacetFilter,
                                  ChooseableFacetRelationFilter,
                                  ChooseableSegmentRelationFilter> m_segmentFinderFacetAutomaton;

      /// Puts the internal segments on the DataStore
      StoreVectorSwapper<CDCRecoSegment2D> m_segmentSwapper{"CDCRecoSegment2DVector"};

      /// Pool for the constructed segments
      std::vector<CDCRecoSegment2D> m_segments;

    }; // end class SegmentFinderCDCFacetAutomatonModule

  } //end namespace TrackFindingCDC
} //end namespace Belle2
