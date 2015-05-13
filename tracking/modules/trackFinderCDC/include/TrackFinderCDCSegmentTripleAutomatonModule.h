/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRACKFINDERCDCSEGMENTTRIPLEAUTOMATONMODULE_H_
#define TRACKFINDERCDCSEGMENTTRIPLEAUTOMATONMODULE_H_

#include <tracking/trackFindingCDC/creators/TrackCreator.h>

#include <tracking/trackFindingCDC/creators/SegmentTripleCreator.h>
#include <tracking/trackFindingCDC/algorithms/MultipassCellularPathFinder.h>

#include <tracking/trackFindingCDC/filters/axial_axial/SimpleAxialAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segment_triple/SimpleSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/segment_triple_segment_triple/SimpleSegmentTripleNeighborChooser.h>
#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>

#include <tracking/trackFindingCDC/algorithms/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCFromSegmentsModule.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Forward declaration of the module implementing the track generation by cellular automaton on segment triples using specific filter instances.
    template < class AxialAxialSegmentPairFilter = BaseAxialAxialSegmentPairFilter,
               class SegmentTripleFilter = BaseSegmentTripleFilter,
               class SegmentTripleNeighborChooser = BaseSegmentTripleNeighborChooser >
    class TrackFinderCDCSegmentTripleAutomatonImplModule;
  }

  /// Module specialisation using the default Monte Carlo free filters. To be used in production.
  typedef TrackFindingCDC::TrackFinderCDCSegmentTripleAutomatonImplModule <
  TrackFindingCDC::SimpleAxialAxialSegmentPairFilter,
                  TrackFindingCDC::SimpleSegmentTripleFilter,
                  TrackFindingCDC::SimpleSegmentTripleNeighborChooser
                  > TrackFinderCDCSegmentTripleAutomatonModule;

  namespace TrackFindingCDC {
    template<class AxialAxialSegmentPairFilter, class SegmentTripleFilter, class SegmentTripleNeighborChooser>
    class TrackFinderCDCSegmentTripleAutomatonImplModule : public TrackFinderCDCFromSegmentsModule {

    public:
      /// Default constructor initialising the filters with the default settings
      TrackFinderCDCSegmentTripleAutomatonImplModule(ETrackOrientation trackOrientation = c_None) :
        TrackFinderCDCBaseModule(trackOrientation),
        TrackFinderCDCFromSegmentsModule(trackOrientation),
        m_ptrAxialAxialSegmentPairFilter(new AxialAxialSegmentPairFilter()),
        m_ptrSegmentTripleFilter(new SegmentTripleFilter()),
        m_ptrSegmentTripleNeighborChooser(new SegmentTripleNeighborChooser()),
        m_param_writeSegmentTriples(false),
        m_param_segmentTriplesStoreObjName("CDCSegmentTripleVector")
      {

        setDescription("Generates tracks from segments using a cellular automaton build from segment triples.");

        addParam("WriteSegmentTriples",
                 m_param_writeSegmentTriples,
                 "Switch if segment triples shall be written to the DataStore",
                 false);

        addParam("SegmentTriplesStoreObjName",
                 m_param_segmentTriplesStoreObjName,
                 "Name of the output StoreObjPtr of the axial stereo segment piars generated within this module.",
                 std::string("CDCSegmentTripleVector"));

      }

      /// Destructor deleting the filters.
      ~TrackFinderCDCSegmentTripleAutomatonImplModule()
      {
        if (m_ptrAxialAxialSegmentPairFilter) delete m_ptrAxialAxialSegmentPairFilter;
        m_ptrAxialAxialSegmentPairFilter = nullptr;

        if (m_ptrSegmentTripleFilter) delete m_ptrSegmentTripleFilter;
        m_ptrSegmentTripleFilter = nullptr;

        if (m_ptrSegmentTripleNeighborChooser) delete m_ptrSegmentTripleNeighborChooser;
        m_ptrSegmentTripleNeighborChooser = nullptr;
      }

      ///  Initialize the Module before event processing
      virtual void initialize() override
      {
        TrackFinderCDCFromSegmentsModule::initialize();

        if (m_param_writeSegmentTriples) {
          StoreWrappedObjPtr< std::vector<CDCSegmentTriple> >::registerTransient(m_param_segmentTriplesStoreObjName);
        }

        if (m_ptrAxialAxialSegmentPairFilter) {
          m_ptrAxialAxialSegmentPairFilter->initialize();
        }

        if (m_ptrSegmentTripleFilter) {
          m_ptrSegmentTripleFilter->initialize();
        }

        if (m_ptrSegmentTripleNeighborChooser) {
          m_ptrSegmentTripleNeighborChooser->initialize();
        }

      }

      /// Generates the tracks from the given segments into the output argument.
      virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                            std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override final;

      virtual void terminate() override
      {
        if (m_ptrAxialAxialSegmentPairFilter) {
          m_ptrAxialAxialSegmentPairFilter->terminate();
        }

        if (m_ptrSegmentTripleFilter) {
          m_ptrSegmentTripleFilter->terminate();
        }

        if (m_ptrSegmentTripleNeighborChooser) {
          m_ptrSegmentTripleNeighborChooser->terminate();
        }

        TrackFinderCDCFromSegmentsModule::terminate();
      }

    public:
      /// Getter for the current axial axial segment filter. The module keeps ownership of the pointer.
      AxialAxialSegmentPairFilter* getAxialAxialSegmentPairFilter()
      {
        return m_ptrAxialAxialSegmentPairFilter;
      }

      /// Setter for the segment triple filter. The module takes ownership of the pointer.
      void getAxialAxialSegmentPairFilter(AxialAxialSegmentPairFilter* ptrAxialAxialSegmentPairFilter)
      {
        if (m_ptrAxialAxialSegmentPairFilter) delete m_ptrAxialAxialSegmentPairFilter;
        m_ptrAxialAxialSegmentPairFilter = ptrAxialAxialSegmentPairFilter;
      }

      /// Getter for the current segment triple filter. The module keeps ownership of the pointer.
      SegmentTripleFilter* getSegmentTripleFilter()
      {
        return m_ptrSegmentTripleFilter;
      }

      /// Setter for the segment triple filter. The module takes ownership of the pointer.
      void setSegmentTripleFilter(SegmentTripleFilter* ptrSegmentTripleFilter)
      {
        if (m_ptrSegmentTripleFilter) delete m_ptrSegmentTripleFilter;
        m_ptrSegmentTripleFilter = ptrSegmentTripleFilter;
      }

      /// Getter for the current segment triple neighbor chooser. The module keeps ownership of the pointer.
      SegmentTripleNeighborChooser* getSegmentTripleNeighborChooser()
      {
        return m_ptrSegmentTripleNeighborChooser;
      }

      /// Setter for the current segment triple neighbor chooser. The module keeps ownership of the pointer.
      void setSegmentTripleNeighborChooser(SegmentTripleNeighborChooser* ptrSegmentTripleNeighborChooser)
      {
        if (m_ptrSegmentTripleNeighborChooser) delete m_ptrSegmentTripleNeighborChooser;
        m_ptrSegmentTripleNeighborChooser = ptrSegmentTripleNeighborChooser;
      }

    private:
      /// Reference to the filter to be used for the axial axial segment pair generation.
      AxialAxialSegmentPairFilter* m_ptrAxialAxialSegmentPairFilter;

      /// Reference to the filter to be used for the segment triple generation.
      SegmentTripleFilter* m_ptrSegmentTripleFilter;

      /// Reference to the chooser to be used to construct the segment triple network.
      SegmentTripleNeighborChooser* m_ptrSegmentTripleNeighborChooser;

    private:
      /// Parameter: Switch if segment triples shall be written to the DataStore
      bool m_param_writeSegmentTriples;

      /// Parameter: Name of the output StoreObjPtr of the segment triples generated within this module.
      std::string m_param_segmentTriplesStoreObjName;

    private:
      // object pools
      /// Memory for the segment triples
      std::set<CDCSegmentTriple> m_segmentTriples;

      /// Memory for the segment triples neighborhood
      WeightedNeighborhood<const CDCSegmentTriple> m_segmentTripleNeighborhood;

      /// Memory for the segment triple paths generated from the graph.
      std::vector< std::vector<const CDCSegmentTriple*> > m_segmentTriplePaths;


      //object creators
      /// Instance of the segment triple creator
      SegmentTripleCreator m_segmentTripleCreator;

      //cellular automat
      /// Instance of the cellular automaton.
      MultipassCellularPathFinder<CDCSegmentTriple> m_cellularPathFinder;

      /// Instance of the track creator from paths.
      TrackCreator m_trackCreator;

    }; // end class TrackFinderCDCSegmentTripleAutomatonImplModule


    template < class AxialAxialSegmentPairFilter,
               class SegmentTripleFilter,
               class SegmentTripleNeighborChooser >
    void TrackFinderCDCSegmentTripleAutomatonImplModule <
    AxialAxialSegmentPairFilter,
    SegmentTripleFilter,
    SegmentTripleNeighborChooser
    >::generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
    {
      // Attain segment triple vector on the DataStore if needed.
      std::vector<CDCSegmentTriple>* ptrSegmentTriples = nullptr;
      if (m_param_writeSegmentTriples) {
        StoreWrappedObjPtr< std::vector<CDCSegmentTriple> > storedSegmentTriples(m_param_segmentTriplesStoreObjName);
        storedSegmentTriples.create();
        std::vector<CDCSegmentTriple>& segmentTriples = *storedSegmentTriples;
        ptrSegmentTriples = &segmentTriples;
      }


      // Create the segment triples
      B2DEBUG(100, "Combining CDCReco2DSegments to CDCSegmentTriples");
      m_segmentTriples.clear();
      m_segmentTripleCreator.create(*m_ptrAxialAxialSegmentPairFilter,
                                    *m_ptrSegmentTripleFilter,
                                    segments,
                                    m_segmentTriples);
      B2DEBUG(100, "  Created " << m_segmentTriples.size()  << " CDCSegmentTriples");

      // Create the segment triple neighorhood
      B2DEBUG(100, "Creating the CDCSegmentTriple neighborhood");
      m_segmentTripleNeighborhood.clear();
      m_segmentTripleNeighborhood.createUsing(*m_ptrSegmentTripleNeighborChooser, m_segmentTriples);
      B2DEBUG(100, "  Created " << m_segmentTripleNeighborhood.size()  << " SegmentTripleNeighborhoods");

      if (m_param_writeSegmentTriples) {
        std::vector<CDCSegmentTriple>& segmentTriples = *ptrSegmentTriples;
        for (const CDCSegmentTriple& segmentTriple : m_segmentTriples) {
          segmentTriples.push_back(segmentTriple);
        }
      }

      // Multiple passes if growMany is active and one track is created at a time
      m_segmentTriplePaths.clear();
      m_cellularPathFinder.apply(m_segmentTriples, m_segmentTripleNeighborhood, m_segmentTriplePaths);
      B2DEBUG(100, "  Created " << m_segmentTriplePaths.size()  << " SegmentTripleTracks");

      // Reduce to plain tracks
      B2DEBUG(100, "Reducing the SegmentTripleTracks to CDCTracks");
      m_trackCreator.create(m_segmentTriplePaths, tracks);
      B2DEBUG(100, "  Created " << tracks.size()  << " CDCTracks");
    }

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif // TRACKFINDERCDCSEGMENTTRIPLEAUTOMATONMODULE_H_






