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

#include <tracking/trackFindingCDC/findlets/combined/TrackFinderSegmentPairAutomaton.h>

#include <tracking/trackFindingCDC/filters/segmentPair/SimpleSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/SimpleSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCFromSegmentsModule.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Forward declaration of the module implementing the track generation by cellular automaton on segment pairs using specific filter instances.
    template < class SegmentPairFilter = BaseSegmentPairFilter,
               class SegmentPairRelationFilter = BaseSegmentPairRelationFilter,
               class TrackRelationFilter = BaseTrackRelationFilter>
    class TrackFinderCDCSegmentPairAutomatonImplModule;
  }

  /// Module specialisation using the default Monte Carlo free filters. To be used in production.
  using TrackFinderCDCSegmentPairAutomatonModule =
    TrackFindingCDC::
    TrackFinderCDCSegmentPairAutomatonImplModule<TrackFindingCDC::SimpleSegmentPairFilter,
    TrackFindingCDC::SimpleSegmentPairRelationFilter,
    TrackFindingCDC::BaseTrackRelationFilter>;

  namespace TrackFindingCDC {
    template<class SegmentPairFilter,
             class SegmentPairRelationFilter,
             class TrackRelationFilter>
    class TrackFinderCDCSegmentPairAutomatonImplModule :
      public TrackFinderCDCFromSegmentsModule {

    public:
      /// Default constructor initialising the filters with the default settings
      TrackFinderCDCSegmentPairAutomatonImplModule()
      {
        setDescription("Generates tracks from segments using a cellular automaton build from segment pairs.");

        ModuleParamList moduleParamList = this->getParamList();
        m_trackFinderSegmentPairAutomaton.exposeParameters(&moduleParamList);
        this->setParamList(moduleParamList);
      }

      ///  Initialize the Module before event processing
      virtual void initialize() override
      {
        TrackFinderCDCFromSegmentsModule::initialize();
        m_trackFinderSegmentPairAutomaton.initialize();
      }

      /// Generates the tracks from the given segments into the output argument.
      virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                            std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override final
      {
        m_trackFinderSegmentPairAutomaton.beginEvent();
        m_trackFinderSegmentPairAutomaton.apply(segments, tracks);
      }

      virtual void terminate() override
      {
        m_trackFinderSegmentPairAutomaton.terminate();
        TrackFinderCDCFromSegmentsModule::terminate();
      }

    private:
      /// Combined findlet implementing the generation of tracks from segments in a segment pair cellular automaton.
      TrackFinderSegmentPairAutomaton<SegmentPairFilter,
                                      SegmentPairRelationFilter,
                                      TrackRelationFilter> m_trackFinderSegmentPairAutomaton;

    }; // end class TrackFinderCDCSegmentPairAutomatonImplModule

  } //end namespace TrackFindingCDC
} //end namespace Belle2
