/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SEGMENTTRIPLETRACKINGWORKER_H_
#define SEGMENTTRIPLETRACKINGWORKER_H_

#include <vector>

#include <framework/datastore/StoreArray.h>

#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>

#include <tracking/cdcLocalTracking/algorithms/Clusterizer.h>
#include <tracking/cdcLocalTracking/algorithms/MultipassCellularPathFinder.h>

#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

#include <tracking/cdcLocalTracking/creators/SegmentTripleCreator.h>
#include <tracking/cdcLocalTracking/creators/TrackCreator.h> //no decisions to optimize
#include <tracking/cdcLocalTracking/creators/SingleSegmentTrackCreator.h>
#include <tracking/cdcLocalTracking/creators/TrackOrientator.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// Worker for building reconstructed tracks form reconstructed segments using segment triples
    template<class AxialAxialSegmentPairFilter, class SegmentTripleFilter, class SegmentTripleNeighborChooser>
    class SegmentTripleTrackingWorker {

    public:
      /** Constructor. */
      SegmentTripleTrackingWorker(): m_cellularPathFinder(0.0) {;}

      /** Destructor.*/
      ~SegmentTripleTrackingWorker() {;}

      /// Forwards the initialize methods of the module to the segment creator and the neighborhood builder
      void initialize() {

#ifdef CDCLOCALTRACKING_USE_ROOT
        StoreArray < CDCSegmentTriple >::registerTransient("CDCSegmentTriples");
        StoreArray < CDCTrack >::registerTransient("CDCTracks");
#endif
        m_segmentTripleCreator.initialize();
        m_segmentTripleNeighborChooser.initialize();

      }

      /// Forwards the terminate method of the module to the segment creator and the neighborhood builder
      void terminate() {
        m_segmentTripleCreator.terminate();
        m_segmentTripleNeighborChooser.terminate();
      }


      /// Generates tracks from the given segments
      inline void apply(const std::vector< CDCRecoSegment2D >& recoSegments,
                        StoreArray < genfit::TrackCand >& storedGFTrackCands) {

        //create the segment triples
        B2DEBUG(100, "Combining CDCReco2DSegments to CDCSegmentTriples");
        m_segmentTriples.clear();
        m_segmentTripleCreator.create(recoSegments, m_segmentTriples);
        B2DEBUG(100, "  Created " << m_segmentTriples.size()  << " CDCSegmentTriples");

        //create the segment triple neighorhood
        B2DEBUG(100, "Creating the CDCSegmentTriple neighborhood");
        m_segmentTripleNeighborhood.clear();
        m_segmentTripleNeighborhood.createUsing(m_segmentTripleNeighborChooser, m_segmentTriples);
        B2DEBUG(100, "  Created " << m_segmentTripleNeighborhood.size()  << " SegmentTripleNeighborhoods");

        //multiple passes if growMany is active and one track is created at a time
        //no best candidate analysis needed

        m_segmentTripleTracks.clear();
        m_cellularPathFinder.apply(m_segmentTriples, m_segmentTripleNeighborhood, m_segmentTripleTracks);

        B2DEBUG(100, "  Created " << m_segmentTripleTracks.size()  << " SegmentTripleTracks");

        //reduce to plain tracks
        B2DEBUG(100, "Reducing the SegmentTripleTracks to CDCTracks");
        m_tracks.clear();
        m_trackCreator.create(m_segmentTripleTracks, m_tracks);
        B2DEBUG(100, "  Created " << m_tracks.size()  << " CDCTracks");

        //m_singleSegmentTrackCreator.append(recoSegments, m_tracks);

        m_trackOrientator.markOrientation(m_tracks);

        //create the gftracks
        for (const CDCTrack & track : m_tracks) {
          genfit::TrackCand* ptrTrackCand = storedGFTrackCands.appendNew();
          track.fillInto(*ptrTrackCand);
        }

        copyToDataStoreForDebug();

      }





    private:
      /// Helper function to copy intermediate objects to the data store for analysis from python.
      void copyToDataStoreForDebug() const {

#ifdef CDCLOCALTRACKING_USE_ROOT
        // IO for monitoring in python
        // IO segment triples
        StoreArray < CDCSegmentTriple > storedSegmentTriples("CDCSegmentTriples");
        storedSegmentTriples.create();
        for (const CDCSegmentTriple & segmentTriple :  m_segmentTriples) {
          storedSegmentTriples.appendNew(segmentTriple);
        }

        // IO tracks
        StoreArray < CDCTrack > storedTracks("CDCTracks");
        storedTracks.create();
        for (const CDCTrack & track : m_tracks) {
          storedTracks.appendNew(track);
        }

#endif

      }





    private:
      // object pools

      /// Memory for the segment triples
      std::set<CDCSegmentTriple> m_segmentTriples;

      /// Memory for the segment triples neighborhood
      WeightedNeighborhood<const CDCSegmentTriple> m_segmentTripleNeighborhood;

      /// Memory for the segment triple paths generated from the graph.
      std::vector< std::vector<const CDCSegmentTriple*> > m_segmentTripleTracks;

      /// Memory for the tracks generated from the paths
      std::vector<CDCTrack> m_tracks;

      //object creators
      /// Instance of the segment triple creator
      SegmentTripleCreator<AxialAxialSegmentPairFilter, SegmentTripleFilter> m_segmentTripleCreator;

      /// Instance of the segment triple neighbor chooser
      SegmentTripleNeighborChooser m_segmentTripleNeighborChooser;

      //cellular automat
      /// Instance of the cellular automaton.
      MultipassCellularPathFinder<CDCSegmentTriple> m_cellularPathFinder;

      /// Instance of the track creator from paths.
      TrackCreator m_trackCreator;

      /// Instance of the leftover single segment tracks creator.
      SingleSegmentTrackCreator m_singleSegmentTrackCreator;

      /// Instance of the orientation fixer
      TrackOrientator m_trackOrientator;

    }; // end class SegmentTripleTrackingWorker
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SEGMENTTRIPLETRACKINGWORKER_H_
