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

#include <tracking/trackFindingCDC/findlets/minimal/SegmentPairCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSegmentPairAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/TrackMerger.h>

#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <tracking/trackFindingCDC/filters/segmentPair/SimpleSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/SimpleSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCFromSegmentsModule.h>

#include <vector>
#include <memory>

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
        m_segmentPairCreator.exposeParameters(&moduleParamList, "SegmentPair");
        m_trackCreator.exposeParameters(&moduleParamList, "SegmentPairRelation");
        m_trackMerger.exposeParameters(&moduleParamList, "TrackRelation");
        m_segmentPairSwapper.exposeParameters(&moduleParamList);
        this->setParamList(moduleParamList);
      }

      ///  Initialize the Module before event processing
      virtual void initialize() override
      {
        TrackFinderCDCFromSegmentsModule::initialize();

        m_segmentPairCreator.initialize();
        m_trackCreator.initialize();
        m_trackMerger.initialize();
        m_segmentPairSwapper.initialize();
      }

      /// Generates the tracks from the given segments into the output argument.
      virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                            std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override final;

      virtual void terminate() override
      {
        m_segmentPairSwapper.terminate();
        m_trackMerger.terminate();
        m_trackCreator.terminate();
        m_segmentPairCreator.terminate();
        TrackFinderCDCFromSegmentsModule::terminate();
      }

    private:
      /// Findlet responsible for the creation of segment pairs
      SegmentPairCreator<SegmentPairFilter> m_segmentPairCreator;

      /// Reference to the relation filter to be used to construct the segment pair network.
      TrackCreatorSegmentPairAutomaton<SegmentPairRelationFilter> m_trackCreator;

      /// Findlet responsible for the merging of tracks
      TrackMerger<TrackRelationFilter> m_trackMerger;

      /// Puts the internal segment pairs on the DataStore
      StoreVectorSwapper<CDCSegmentPair> m_segmentPairSwapper{"CDCSegmentPairVector"};

    private:
      //object pools
      /// Memory for the axial stereo segment pairs.
      std::vector<CDCSegmentPair> m_segmentPairs;

      /// Memory for the tracks before merging was applied.
      std::vector<CDCTrack> m_preMergeTracks;
    }; // end class TrackFinderCDCSegmentPairAutomatonImplModule


    template < class SegmentPairFilter,
               class SegmentPairRelationFilter,
               class TrackRelationFilter>
    void
    TrackFinderCDCSegmentPairAutomatonImplModule <SegmentPairFilter,
                                                 SegmentPairRelationFilter,
                                                 TrackRelationFilter>::
                                                 generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                                                          std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
    {
      m_segmentPairCreator.beginEvent();
      m_trackCreator.beginEvent();
      m_trackMerger.beginEvent();
      m_segmentPairSwapper.beginEvent();

      m_segmentPairs.clear();
      m_preMergeTracks.clear();

      m_segmentPairCreator.apply(segments, m_segmentPairs);
      m_trackCreator.apply(m_segmentPairs, m_preMergeTracks);
      m_trackMerger.apply(m_preMergeTracks, tracks);
      m_segmentPairSwapper.apply(m_segmentPairs);

    }
  } //end namespace TrackFindingCDC
} //end namespace Belle2
