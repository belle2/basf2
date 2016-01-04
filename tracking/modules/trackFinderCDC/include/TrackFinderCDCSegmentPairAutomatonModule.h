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

#include <tracking/trackFindingCDC/creators/SegmentPairCreator.h>
#include <tracking/trackFindingCDC/ca/MultipassCellularPathFinder.h>

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
      TrackFinderCDCSegmentPairAutomatonImplModule() :
        m_ptrSegmentPairFilter(new SegmentPairFilter()),
        m_ptrSegmentPairRelationFilter(new SegmentPairRelationFilter()),
        m_ptrTrackRelationFilter(new TrackRelationFilter()),
        m_param_writeSegmentPairs(false),
        m_param_segmentPairsStoreObjName("CDCSegmentPairVector")
      {

        setDescription("Generates tracks from segments using a cellular automaton build from segment pairs.");

        addParam("WriteSegmentPairs",
                 m_param_writeSegmentPairs,
                 "Switch if segment pairs shall be written to the DataStore",
                 false);

        addParam("SegmentPairsStoreObjName",
                 m_param_segmentPairsStoreObjName,
                 "Name of the output StoreObjPtr of the axial stereo segment piars generated within this module.",
                 std::string("CDCSegmentPairVector"));

      }

      ///  Initialize the Module before event processing
      virtual void initialize() override
      {
        TrackFinderCDCFromSegmentsModule::initialize();

        if (m_param_writeSegmentPairs) {
          StoreWrappedObjPtr< std::vector<CDCSegmentPair> >::registerTransient(m_param_segmentPairsStoreObjName);
        }

        if (m_ptrSegmentPairFilter) {
          m_ptrSegmentPairFilter->initialize();
        }

        if (m_ptrSegmentPairRelationFilter) {
          m_ptrSegmentPairRelationFilter->initialize();
        }

        if (m_ptrTrackRelationFilter) {
          m_ptrTrackRelationFilter->initialize();
        }

      }

      /// Generates the tracks from the given segments into the output argument.
      virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                            std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override final;

      virtual void terminate() override
      {
        if (m_ptrSegmentPairFilter) {
          m_ptrSegmentPairFilter->terminate();
        }

        if (m_ptrSegmentPairRelationFilter) {
          m_ptrSegmentPairRelationFilter->terminate();
        }

        if (m_ptrTrackRelationFilter) {
          m_ptrTrackRelationFilter->terminate();
        }

        TrackFinderCDCFromSegmentsModule::terminate();
      }

    public:
      /// Getter for the current segment pair filter. The module keeps ownership of the pointer.
      SegmentPairFilter* getSegmentPairFilter()
      {
        return m_ptrSegmentPairFilter.get();
      }

      /// Setter for the segment pair filter used in the segment pair creation. The module takes ownership of the pointer.
      void setSegmentPairFilter(std::unique_ptr<SegmentPairFilter>
                                ptrSegmentPairFilter)
      {
        m_ptrSegmentPairFilter = std::move(ptrSegmentPairFilter);
      }

      /// Getter for the current segment pair relation filter. The module keeps ownership of the pointer.
      SegmentPairRelationFilter* getSegmentPairRelationFilter()
      {
        return m_ptrSegmentPairRelationFilter.get();
      }

      /// Setter for the segment relation filter. The module takes ownership of the pointer.
      void setSegmentPairRelationFilter(std::unique_ptr<SegmentPairRelationFilter>
                                        ptrSegmentPairRelationFilter)
      {
        m_ptrSegmentPairRelationFilter = std::move(ptrSegmentPairRelationFilter);
      }

      /// Getter for the current track relation filter. The module keeps ownership of the pointer.
      TrackRelationFilter* getTrackRelationFilter()
      {
        return m_ptrTrackRelationFilter.get();
      }

      /// Setter for the track relation filter. The module takes ownership of the pointer.
      void setTrackRelationFilter(std::unique_ptr<TrackRelationFilter> ptrTrackRelationFilter)
      {
        m_ptrTrackRelationFilter = std::move(ptrTrackRelationFilter);
      }



    private:
      /// Reference to the filter to be used for the segment pair generation.
      std::unique_ptr<SegmentPairFilter> m_ptrSegmentPairFilter;

      /// Reference to the relation filter to be used to construct the segment pair network.
      std::unique_ptr<SegmentPairRelationFilter> m_ptrSegmentPairRelationFilter;

      /// Reference to the relation filter to be used to construct the track network for merging.
      std::unique_ptr<TrackRelationFilter> m_ptrTrackRelationFilter;


    private:
      /// Parameter: Switch if segment pairs shall be written to the DataStore
      bool m_param_writeSegmentPairs;

      /// Parameter: Name of the output StoreObjPtr of the axial stereo segment pairs generated within this module.
      std::string m_param_segmentPairsStoreObjName;


    private:
      //object pools
      /// Memory for the axial stereo segment pairs.
      std::vector<CDCSegmentPair> m_segmentPairs;

      /// Memory for the axial stereo segment pair neighborhood.
      WeightedNeighborhood<const CDCSegmentPair> m_segmentPairNeighborhood;

      /// Memory for the segment triple paths generated from the graph.
      std::vector<std::vector<const CDCSegmentPair*> > m_segmentPairPaths;

      /// Memory for the tracks before merging was applied.
      std::vector<CDCTrack> m_preMergeTracks;

      /// Memory for the symmetrised tracks before merging was applied.
      std::vector<CDCTrack> m_preMergeSymmetricTracks;

      //object creators
      /// Instance of the axial stereo segment pair creator.
      SegmentPairCreator m_segmentPairCreator;

      //cellular automat
      /// Instance of the cellular automaton.
      MultipassCellularPathFinder<const CDCSegmentPair> m_cellularPathFinder;

      // Deprication:
      /// Instance of the track creator from paths.
      TrackCreator m_trackCreator;
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
      // Attain segment pair vector on the DataStore if needed.
      std::vector<CDCSegmentPair>* ptrSegmentPairs = nullptr;
      if (m_param_writeSegmentPairs) {
        StoreWrappedObjPtr< std::vector<CDCSegmentPair> > storedSegmentPairs(m_param_segmentPairsStoreObjName);
        storedSegmentPairs.create();
        std::vector<CDCSegmentPair>& segmentPairs = *storedSegmentPairs;
        ptrSegmentPairs = &segmentPairs;
      }

      // Create the segment pairs
      B2DEBUG(100, "Combining CDCReco2DSegments to CDCSegmentPairs");
      m_segmentPairs.clear();
      m_segmentPairCreator.create(*m_ptrSegmentPairFilter, segments, m_segmentPairs);
      B2DEBUG(100, "  Created " << m_segmentPairs.size()  << " CDCSegmentPair");

      if (m_param_writeSegmentPairs) {
        std::vector<CDCSegmentPair>& segmentPairs = *ptrSegmentPairs;
        for (const CDCSegmentPair& segmentPair : m_segmentPairs) {
          segmentPairs.push_back(segmentPair);
        }
      }

      //create the segment pair neighborhood
      B2DEBUG(100, "Creating the CDCSegmentPair neighborhood");
      m_segmentPairNeighborhood.clear();
      m_segmentPairNeighborhood.appendUsing(*m_ptrSegmentPairRelationFilter, m_segmentPairs);
      B2DEBUG(100, "  Created " << m_segmentPairNeighborhood.size()  << " AxialStereoPairNeighborhoods");

      //multiple passes if growMany is active and one track is created at a time
      //no best candidate analysis needed
      m_segmentPairPaths.clear();
      m_cellularPathFinder.apply(m_segmentPairs, m_segmentPairNeighborhood, m_segmentPairPaths);

      B2DEBUG(100, "  Created " << m_segmentPairPaths.size()  << " SegmentTripleTracks");

      //reduce to plain tracks
      B2DEBUG(100, "Reducing the AxialStereoPairTracks to CDCTracks");
      m_preMergeTracks.clear();
      m_trackCreator.create(m_segmentPairPaths, m_preMergeTracks);

      /// Apply track merging.
      m_preMergeSymmetricTracks.clear();
      m_preMergeSymmetricTracks.reserve(2 * m_preMergeTracks.size());

      for (const CDCTrack& track : m_preMergeTracks) {
        m_preMergeSymmetricTracks.push_back(track);
        m_preMergeSymmetricTracks.push_back(track.reversed());
      }

      WeightedNeighborhood<const CDCTrack> tracksNeighborhood;
      tracksNeighborhood.clear();
      tracksNeighborhood.appendUsing(*m_ptrTrackRelationFilter,
                                     m_preMergeSymmetricTracks);

      MultipassCellularPathFinder<const CDCTrack> cellularPathFinder;
      std::vector< std::vector<const CDCTrack*> > trackPaths;
      cellularPathFinder.apply(m_preMergeSymmetricTracks,
                               tracksNeighborhood,
                               trackPaths);

      for (const std::vector<const CDCTrack*>& trackPath : trackPaths) {
        tracks.push_back(CDCTrack::condense(trackPath));
      }

    }

  } //end namespace TrackFindingCDC
} //end namespace Belle2
