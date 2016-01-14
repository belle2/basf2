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


#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialSegmentPairCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentTripleCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentTripleAutomaton.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/SimpleAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTriple/SimpleSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTripleRelation/SimpleSegmentTripleRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCFromSegmentsModule.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Forward declaration of the module implementing the track generation by cellular automaton on segment triples using specific filter instances.
    template < class AxialSegmentPairFilter = BaseAxialSegmentPairFilter,
               class SegmentTripleFilter = BaseSegmentTripleFilter,
               class SegmentTripleRelationFilter = BaseSegmentTripleRelationFilter >
    class TrackFinderCDCSegmentTripleAutomatonImplModule;
  }

  /// Module specialisation using the default Monte Carlo free filters. To be used in production.
  typedef TrackFindingCDC::TrackFinderCDCSegmentTripleAutomatonImplModule <
  TrackFindingCDC::SimpleAxialSegmentPairFilter,
                  TrackFindingCDC::SimpleSegmentTripleFilter,
                  TrackFindingCDC::SimpleSegmentTripleRelationFilter
                  > TrackFinderCDCSegmentTripleAutomatonModule;

  namespace TrackFindingCDC {
    template<class AxialSegmentPairFilter, class SegmentTripleFilter, class SegmentTripleRelationFilter>
    class TrackFinderCDCSegmentTripleAutomatonImplModule : public TrackFinderCDCFromSegmentsModule {

    public:
      /// Default constructor initialising the filters with the default settings
      TrackFinderCDCSegmentTripleAutomatonImplModule()
      {
        setDescription("Generates tracks from segments using a cellular automaton build from segment triples.");

        ModuleParamList moduleParamList = this->getParamList();
        m_axialSegmentPairCreator.exposeParameters(&moduleParamList, "axialSegmentPair");
        m_segmentTripleCreator.exposeParameters(&moduleParamList, "segmentTriple");
        m_trackCreatorSegmentTripleAutomaton.exposeParameters(&moduleParamList, "segmentTripleRelation");
        this->setParamList(moduleParamList);
      }

      ///  Initialize the Module before event processing
      virtual void initialize() override
      {
        TrackFinderCDCFromSegmentsModule::initialize();
        m_axialSegmentPairCreator.initialize();
        m_segmentTripleCreator.initialize();
        m_trackCreatorSegmentTripleAutomaton.initialize();
        m_segmentTripleSwapper.initialize();
      }

      /// Generates the tracks from the given segments into the output argument.
      virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                            std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override final
      {

        m_axialSegmentPairCreator.beginEvent();
        m_segmentTripleCreator.beginEvent();
        m_trackCreatorSegmentTripleAutomaton.beginEvent();
        m_segmentTripleSwapper.beginEvent();

        m_axialSegmentPairs.clear();
        m_segmentTriples.clear();
        m_segmentTriplePaths.clear();

        m_axialSegmentPairCreator.apply(segments, m_axialSegmentPairs);
        m_segmentTripleCreator.apply(segments, m_axialSegmentPairs, m_segmentTriples);
        m_trackCreatorSegmentTripleAutomaton.apply(m_segmentTriples, tracks);
        m_segmentTripleSwapper.apply(m_segmentTriples);

      }

      virtual void terminate() override
      {
        m_segmentTripleSwapper.terminate();
        m_trackCreatorSegmentTripleAutomaton.terminate();
        m_segmentTripleCreator.terminate();
        m_axialSegmentPairCreator.terminate();
        TrackFinderCDCFromSegmentsModule::terminate();
      }

    private:
      // object pools
      /// Memory for the axial to axial segment pairs
      std::vector<CDCAxialSegmentPair> m_axialSegmentPairs;

      /// Memory for the segment triples
      std::vector<CDCSegmentTriple> m_segmentTriples;

      /// Memory for the segment triple paths generated from the graph.
      std::vector< std::vector<const CDCSegmentTriple*> > m_segmentTriplePaths;

      // Findlets
      /// Instance of the axial to axial segment pair creator
      AxialSegmentPairCreator<AxialSegmentPairFilter> m_axialSegmentPairCreator;

      /// Instance of the segment triple creator
      SegmentTripleCreator<SegmentTripleFilter> m_segmentTripleCreator;

      /// Instance of the cellular automaton creating  creating tracks over segment triple
      TrackCreatorSegmentTripleAutomaton<SegmentTripleRelationFilter> m_trackCreatorSegmentTripleAutomaton;

      /// Helper to swap the local segment triples out to the DataStore
      StoreVectorSwapper<CDCSegmentTriple> m_segmentTripleSwapper{"CDCSegmentTripleVector"};

    }; // end class TrackFinderCDCSegmentTripleAutomatonModule

  } //end namespace TrackFindingCDC
} //end namespace Belle2
