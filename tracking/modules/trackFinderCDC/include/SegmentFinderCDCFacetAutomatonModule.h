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

#include <tracking/trackFindingCDC/filters/cluster/TMVAClusterFilter.h>
#include <tracking/trackFindingCDC/filters/facet/RealisticFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet_relation/SimpleFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segment_relation/BaseSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

#include <tracking/trackFindingCDC/findlets/combined/SegmentFinderFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Forward declaration of the module implementing the segment generation by cellular automaton on facets using specific filter instances.
    template<class ClusterFilter = BaseClusterFilter,
             class FacetFilter = BaseFacetFilter,
             class FacetRelationFilter = BaseFacetRelationFilter,
             class SegmentRelationFilter = BaseSegmentRelationFilter>
    class SegmentFinderCDCFacetAutomatonImplModule;
  }

  /// Module specialisation using the default Monte Carlo free filters. To be used in production.
  typedef TrackFindingCDC::SegmentFinderCDCFacetAutomatonImplModule <
  TrackFindingCDC::TMVAClusterFilter,
                  TrackFindingCDC::RealisticFacetFilter,
                  TrackFindingCDC::SimpleFacetRelationFilter >
                  SegmentFinderCDCFacetAutomatonModule;

  namespace TrackFindingCDC {
    template<class ClusterFilter,
             class FacetFilter,
             class FacetRelationFilter,
             class SegmentRelationFilter>
    class SegmentFinderCDCFacetAutomatonImplModule :
      public virtual TrackFinderCDCBaseModule {

    private:
      /// Type of the base class
      typedef TrackFinderCDCBaseModule Super;

    public:
      /// Default constructor initialising the filters with the default settings
      SegmentFinderCDCFacetAutomatonImplModule()
      {
        this->setDescription("Generates segments from hits using a cellular automaton build from hit triples (facets).");
        ModuleParamList moduleParamList = this->getParamList();
        m_segmentFinderFacetAutomaton.exposeParameters(&moduleParamList);
        m_segmentSwapper.exposeParameters(&moduleParamList);
        moduleParamList.getParameter<bool>("WriteSegments").setDefaultValue(true);
        this->setParamList(moduleParamList);
      }

      /// Initialize the Module before event processing
      virtual void initialize() override
      {
        Super::initialize();
        m_segmentFinderFacetAutomaton.initialize();
        m_segmentSwapper.initialize();
      }

      /// Processes the current event
      void event() override
      {
        m_segmentFinderFacetAutomaton.beginEvent();
        m_segmentSwapper.beginEvent();
        CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
        std::vector<CDCWireHit>& wireHits = wireHitTopology.getWireHits();
        m_segments.clear();
        m_segmentFinderFacetAutomaton.apply(wireHits, m_segments);
        m_segmentSwapper.apply(m_segments);
      }

      /// Signal the termination of the event processing.
      virtual void terminate() override
      {
        m_segmentSwapper.terminate();
        m_segmentFinderFacetAutomaton.terminate();
        Super::terminate();
      }

    private:
      /// Findlet implementing the algorithm
      SegmentFinderFacetAutomaton<ClusterFilter,
                                  FacetFilter,
                                  FacetRelationFilter,
                                  SegmentRelationFilter> m_segmentFinderFacetAutomaton;

      /// Puts the internal segments on the DataStore
      StoreVectorSwapper<CDCRecoSegment2D> m_segmentSwapper{"CDCRecoSegment2DVector"};

      /// Pool for the constructed segments
      std::vector<CDCRecoSegment2D> m_segments;

    }; // end class SegmentFinderCDCFacetAutomatonImplModule

  } //end namespace TrackFindingCDC
} //end namespace Belle2
