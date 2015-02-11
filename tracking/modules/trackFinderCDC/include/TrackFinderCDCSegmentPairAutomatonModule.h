/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRACKFINDERCDCSEGMENTPAIRAUTOMATONMODULE_H_
#define TRACKFINDERCDCSEGMENTPAIRAUTOMATONMODULE_H_

#include <tracking/trackFindingCDC/creators/TrackCreator.h>

#include <tracking/trackFindingCDC/creators/AxialStereoSegmentPairCreator.h>
#include <tracking/trackFindingCDC/algorithms/MultipassCellularPathFinder.h>

#include <tracking/trackFindingCDC/filters/axial_stereo/SimpleAxialStereoSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/axial_stereo_axial_stereo/SimpleAxialStereoSegmentPairNeighborChooser.h>

#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>

#include <tracking/trackFindingCDC/algorithms/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/modules/trackFinderCDC/TrackFinderCDCFromSegmentsModule.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Forward declaration of the module implementing the track generation by cellular automaton on segment pairs using specific filter instances.
    template < class AxialStereoSegmentPairFilter = BaseAxialStereoSegmentPairFilter,
             class AxialStereoSegmentPairNeighborChooser = BaseAxialStereoSegmentPairNeighborChooser >
    class TrackFinderCDCSegmentPairAutomatonImplModule;
  }

  /// Module specialisation using the default Monte Carlo free filters. To be used in production.
  typedef TrackFindingCDC::TrackFinderCDCSegmentPairAutomatonImplModule <
  TrackFindingCDC::SimpleAxialStereoSegmentPairFilter,
                  TrackFindingCDC::SimpleAxialStereoSegmentPairNeighborChooser
                  > TrackFinderCDCSegmentPairAutomatonModule;

  namespace TrackFindingCDC {
    template<class AxialStereoSegmentPairFilter, class AxialStereoSegmentPairNeighborChooser>
    class TrackFinderCDCSegmentPairAutomatonImplModule : public TrackFinderCDCFromSegmentsModule {

    public:
      /// Default constructor initialising the filters with the default settings
      TrackFinderCDCSegmentPairAutomatonImplModule(ETrackOrientation trackOrientation = c_None) :
        TrackFinderCDCBaseModule(trackOrientation),
        TrackFinderCDCFromSegmentsModule(trackOrientation),
        m_ptrAxialStereoSegmentPairFilter(new AxialStereoSegmentPairFilter()),
        m_ptrAxialStereoSegmentPairNeighborChooser(new AxialStereoSegmentPairNeighborChooser()),
        m_param_writeSegmentPairs(false),
        m_param_segmentPairsStoreObjName("CDCAxialStereoSegmentPairVector") {

        setDescription("Generates tracks from segments using a cellular automaton build from segment pairs.");

        addParam("WriteSegmentPairs",
                 m_param_writeSegmentPairs,
                 "Switch if segment pairs shall be written to the DataStore",
                 false);

        addParam("SegmentPairsStoreObjName",
                 m_param_segmentPairsStoreObjName,
                 "Name of the output StoreObjPtr of the axial stereo segment piars generated within this module.",
                 std::string("CDCAxialStereoSegmentPairVector"));

      }

      /// Destructor deleting the filters.
      ~TrackFinderCDCSegmentPairAutomatonImplModule() {
        if (m_ptrAxialStereoSegmentPairFilter) delete m_ptrAxialStereoSegmentPairFilter;
        m_ptrAxialStereoSegmentPairFilter = nullptr;

        if (m_ptrAxialStereoSegmentPairNeighborChooser) delete m_ptrAxialStereoSegmentPairNeighborChooser;
        m_ptrAxialStereoSegmentPairNeighborChooser = nullptr;
      }

      ///  Initialize the Module before event processing
      virtual void initialize() override {
        TrackFinderCDCFromSegmentsModule::initialize();

        if (m_param_writeSegmentPairs) {
          StoreWrappedObjPtr< std::vector<CDCAxialStereoSegmentPair> >::registerTransient(m_param_segmentPairsStoreObjName);
        }

        if (m_ptrAxialStereoSegmentPairFilter) {
          m_ptrAxialStereoSegmentPairFilter->initialize();
        }

        if (m_ptrAxialStereoSegmentPairNeighborChooser) {
          m_ptrAxialStereoSegmentPairNeighborChooser->initialize();
        }

      }

      /// Generates the tracks from the given segments into the output argument.
      virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                            std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override final;

      virtual void terminate() override {
        if (m_ptrAxialStereoSegmentPairFilter) {
          m_ptrAxialStereoSegmentPairFilter->terminate();
        }

        if (m_ptrAxialStereoSegmentPairNeighborChooser) {
          m_ptrAxialStereoSegmentPairNeighborChooser->terminate();
        }

        TrackFinderCDCFromSegmentsModule::terminate();
      }

    public:
      /// Getter for the current segment pair filter. The module keeps ownership of the pointer.
      AxialStereoSegmentPairFilter* getAxialStereoSegmentPairFilter() {
        return m_ptrAxialStereoSegmentPairFilter;
      }

      /// Setter for the segment pair filter used in the segment pair creation. The module takes ownership of the pointer.
      void setAxialStereoSegmentPairFilter(AxialStereoSegmentPairFilter* ptrAxialStereoSegmentPairFilter) {
        if (m_ptrAxialStereoSegmentPairFilter) delete m_ptrAxialStereoSegmentPairFilter;
        m_ptrAxialStereoSegmentPairFilter = ptrAxialStereoSegmentPairFilter;
      }

      /// Getter for the current segment pair neighbor chooser. The module keeps ownership of the pointer.
      AxialStereoSegmentPairNeighborChooser* getAxialStereoSegmentPairNeighborChooser() {
        return m_ptrAxialStereoSegmentPairNeighborChooser;
      }

      /// Setter for the segment neighbor chooser. The module takes ownership of the pointer.
      void setAxialStereoSegmentPairNeighborChooser(AxialStereoSegmentPairNeighborChooser* ptrAxialStereoSegmentPairNeighborChooser) {
        if (m_ptrAxialStereoSegmentPairNeighborChooser) delete m_ptrAxialStereoSegmentPairNeighborChooser;
        m_ptrAxialStereoSegmentPairNeighborChooser = ptrAxialStereoSegmentPairNeighborChooser;
      }

    private:
      /// Reference to the filter to be used for the segment pair generation.
      AxialStereoSegmentPairFilter* m_ptrAxialStereoSegmentPairFilter;

      /// Reference to the chooser to be used to construct the segment pair network.
      AxialStereoSegmentPairNeighborChooser* m_ptrAxialStereoSegmentPairNeighborChooser;

    private:
      /// Parameter: Switch if segment pairs shall be written to the DataStore
      bool m_param_writeSegmentPairs;

      /// Parameter: Name of the output StoreObjPtr of the axial stereo segment pairs generated within this module.
      std::string m_param_segmentPairsStoreObjName;


    private:
      //object pools
      /// Memory for the axial stereo segment pairs.
      std::vector<CDCAxialStereoSegmentPair> m_axialStereoSegmentPairs;

      /// Memory for the axial stereo segment pair neighborhood.
      WeightedNeighborhood<const CDCAxialStereoSegmentPair> m_axialStereoSegmentPairNeighborhood;

      /// Memory for the segment triple paths generated from the graph.
      std::vector< std::vector<const CDCAxialStereoSegmentPair*> > m_axialStereoSegmentPairPaths;


      //object creators
      /// Instance of the axial stereo segment pair creator.
      AxialStereoSegmentPairCreator m_axialStereoSegmentPairCreator;

      //cellular automat
      /// Instance of the cellular automaton.
      MultipassCellularPathFinder<CDCAxialStereoSegmentPair> m_cellularPathFinder;

      // Deprication:
      /// Instance of the track creator from paths.
      TrackCreator m_trackCreator;

    }; // end class TrackFinderCDCSegmentPairAutomatonImplModule


    template < class AxialStereoSegmentPairFilter,
             class AxialStereoSegmentPairNeighborChooser >
    void TrackFinderCDCSegmentPairAutomatonImplModule <
    AxialStereoSegmentPairFilter,
    AxialStereoSegmentPairNeighborChooser
    >::generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
    {
      // Attain segment pair vector on the DataStore if needed.
      std::vector<CDCAxialStereoSegmentPair>* ptrSegmentPairs = nullptr;
      if (m_param_writeSegmentPairs) {
        StoreWrappedObjPtr< std::vector<CDCAxialStereoSegmentPair> > storedSegmentPairs(m_param_segmentPairsStoreObjName);
        storedSegmentPairs.create();
        std::vector<CDCAxialStereoSegmentPair>& segmentPairs = *storedSegmentPairs;
        ptrSegmentPairs = &segmentPairs;
      }

      // Create the segment pairs
      B2DEBUG(100, "Combining CDCReco2DSegments to CDCAxialStereoSegmentPairs");
      m_axialStereoSegmentPairs.clear();
      m_axialStereoSegmentPairCreator.create(*m_ptrAxialStereoSegmentPairFilter, segments, m_axialStereoSegmentPairs);
      B2DEBUG(100, "  Created " << m_axialStereoSegmentPairs.size()  << " CDCAxialStereoSegmentPair");

      if (m_param_writeSegmentPairs) {
        std::vector<CDCAxialStereoSegmentPair>& segmentPairs = *ptrSegmentPairs;
        for (const CDCAxialStereoSegmentPair & segmentPair : m_axialStereoSegmentPairs) {
          segmentPairs.push_back(segmentPair);
        }
      }

      //create the segment pair neighborhood
      B2DEBUG(100, "Creating the CDCAxialStereoSegmentPair neighborhood");
      m_axialStereoSegmentPairNeighborhood.clear();
      m_axialStereoSegmentPairNeighborhood.createUsing(*m_ptrAxialStereoSegmentPairNeighborChooser, m_axialStereoSegmentPairs);
      B2DEBUG(100, "  Created " << m_axialStereoSegmentPairNeighborhood.size()  << " AxialStereoPairNeighborhoods");

      //multiple passes if growMany is active and one track is created at a time
      //no best candidate analysis needed
      m_axialStereoSegmentPairPaths.clear();
      m_cellularPathFinder.apply(m_axialStereoSegmentPairs, m_axialStereoSegmentPairNeighborhood, m_axialStereoSegmentPairPaths);

      B2DEBUG(100, "  Created " << m_axialStereoSegmentPairPaths.size()  << " SegmentTripleTracks");

      //reduce to plain tracks
      B2DEBUG(100, "Reducing the AxialStereoPairTracks to CDCTracks");
      tracks.clear();
      m_trackCreator.create(m_axialStereoSegmentPairPaths, tracks);
      B2DEBUG(100, "  Created " << tracks.size()  << " CDCTracks");

    }

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif // TRACKFINDERCDCSEGMENTPAIRAUTOMATONMODULE_H_
