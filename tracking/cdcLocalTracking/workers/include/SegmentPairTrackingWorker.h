/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SEGMENTPAIRTRACKINGWORKER_H
#define SEGMENTPAIRTRACKINGWORKER_H

#include <vector>

#include <framework/datastore/StoreArray.h>

#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>
#include <tracking/cdcLocalTracking/algorithms/NeighborhoodBuilder.h>

#include <tracking/cdcLocalTracking/algorithms/Clusterizer.h>
#include <tracking/cdcLocalTracking/algorithms/MultipassCellularPathFinder.h>

#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

#include <tracking/cdcLocalTracking/creators/AxialStereoSegmentPairCreator.h>

#include <tracking/cdcLocalTracking/creators/TrackCreator.h>
#include <tracking/cdcLocalTracking/creators/SingleSegmentTrackCreator.h>
#include <tracking/cdcLocalTracking/creators/TrackOrientator.h>
#include <tracking/cdcLocalTracking/creators/GFTrackCandCreator.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// Worker for building reconstructed tracks form reconstructed segments using segment triples
    template<class AxialStereoSegmentPairFilter, class AxialStereoSegmentPairNeighorChooser>
    class SegmentPairTrackingWorker {

    public:
      /// Constructor.
      SegmentPairTrackingWorker(): m_cellularPathFinder(0.0) {;}



      /// Destructor.
      ~SegmentPairTrackingWorker() {;}



      /// Forwards the initialize methods of the module to the segment creator and the neighborhood builder.
      void initialize() {

#ifdef CDCLOCALTRACKING_USE_ROOT
        StoreArray < CDCAxialStereoSegmentPair >::registerTransient("CDCAxialStereoSegmentPairs");
        StoreArray < CDCTrack >::registerTransient("CDCTracks");
#endif

        m_axialStereoSegmentPairCreator.initialize();
        m_axialStereoSegmentPairNeighborhoodBuilder.initialize();

      }



      /// Forwards the terminate method of the module to the segment creator and the neighborhood builder.
      void terminate() {
        m_axialStereoSegmentPairCreator.terminate();
        m_axialStereoSegmentPairNeighborhoodBuilder.terminate();
      }



      /// Generates tracks from the given segments.
      inline void apply(const std::vector< CDCRecoSegment2D >& segments,
                        StoreArray < genfit::TrackCand >& storedGFTrackCands) {

        //create the segment pairs
        B2DEBUG(100, "Combining CDCReco2DSegments to CDCAxialStereoSegmentPairs");
        m_axialStereoSegmentPairs.clear();
        m_axialStereoSegmentPairCreator.create(segments, m_axialStereoSegmentPairs);
        B2DEBUG(100, "  Created " << m_axialStereoSegmentPairs.size()  << " CDCAxialStereoSegmentPair");

        //create the segment pair neighborhood
        B2DEBUG(100, "Creating the CDCAxialStereoSegmentPair neighborhood");
        m_axialStereoSegmentPairNeighborhood.clear();
        m_axialStereoSegmentPairNeighborhoodBuilder.create(m_axialStereoSegmentPairs, m_axialStereoSegmentPairNeighborhood);
        B2DEBUG(100, "  Created " << m_axialStereoSegmentPairNeighborhood.size()  << " AxialStereoPairNeighborhoods");

        //multiple passes if growMany is active and one track is created at a time
        //no best candidate analysis needed
        m_axialStereoSegmentPairTracks.clear();
        m_cellularPathFinder.apply(m_axialStereoSegmentPairs, m_axialStereoSegmentPairNeighborhood, m_axialStereoSegmentPairTracks);

        B2DEBUG(100, "  Created " << m_axialStereoSegmentPairTracks.size()  << " SegmentTripleTracks");

        //reduce to plain tracks
        B2DEBUG(100, "Reducing the AxialStereoPairTracks to CDCTracks");
        m_tracks.clear();
        m_trackCreator.create(m_axialStereoSegmentPairTracks, m_tracks);
        B2DEBUG(100, "  Created " << m_tracks.size()  << " CDCTracks");

        //m_singleSegmentTrackCreator.append(recoSegments, m_tracks);

        m_trackOrientator.markOrientation(m_tracks);

        //create the gftracks
        B2DEBUG(100, "Creating the genfit::TrackCands");
        m_gfTrackCandCreator.create(m_tracks, storedGFTrackCands);
        B2DEBUG(100, "  Created " << storedGFTrackCands.getEntries()  << " genfit::TrackCands");

        copyToDataStoreForDebug();


      }





    private:
      /// Helper function to copy intermediate objects to the data store for analysis from python.
      void copyToDataStoreForDebug() const {

#ifdef CDCLOCALTRACKING_USE_ROOT

        // IO for monitoring in python
        // IO segment triples
        B2DEBUG(100, "  Creating the StoreArray for the CDCAxialStereoSegmentPair");
        StoreArray < CDCAxialStereoSegmentPair > storedAxialStereoSegmentPairs("CDCAxialStereoSegmentPairs");
        storedAxialStereoSegmentPairs.create();
        for (const CDCAxialStereoSegmentPair & axialStereoSegmentPair :  m_axialStereoSegmentPairs) {
          storedAxialStereoSegmentPairs.appendNew(axialStereoSegmentPair);
        }
        B2DEBUG(100, "  Created " << storedAxialStereoSegmentPairs.getEntries()  << " CDCAxialStereoSegmentPairs");

        // IO tracks
        B2DEBUG(100, "  Creating the StoreArray for the CDCTracks");
        StoreArray < CDCTrack > storedTracks("CDCTracks");
        storedTracks.create();
        B2DEBUG(100, "  Copying the CDCTracks to the StoreArray");
        for (const CDCTrack & track : m_tracks) {
          storedTracks.appendNew(track);
        }

#endif

      }





    private:
      //object pools
      /// Memory for the axial stereo segment pairs.
      std::vector<CDCAxialStereoSegmentPair> m_axialStereoSegmentPairs;

      /// Memory for the axial stereo segment pair neighborhood.
      WeightedNeighborhood<const CDCAxialStereoSegmentPair> m_axialStereoSegmentPairNeighborhood;

      /// Memory for the segment triple paths generated from the graph.
      std::vector< std::vector<const CDCAxialStereoSegmentPair*> > m_axialStereoSegmentPairTracks;

      /// Memory for the tracks generated from the paths.
      std::vector<CDCTrack> m_tracks;



      //object creators
      /// Instance of the axial stereo segment pair creator.
      AxialStereoSegmentPairCreator<AxialStereoSegmentPairFilter> m_axialStereoSegmentPairCreator;

      //neighborhood builders
      /// Instance of the axial stereo pair neighborhoods builder
      NeighborhoodBuilder <CDCAxialStereoSegmentPair, AxialStereoSegmentPairNeighorChooser > m_axialStereoSegmentPairNeighborhoodBuilder;



      //cellular automat
      /// Instance of the cellular automaton.
      MultipassCellularPathFinder<CDCAxialStereoSegmentPair> m_cellularPathFinder;

      /// Instance of the track creator from paths.
      TrackCreator m_trackCreator;

      /// Instance of the leftover single segment tracks creator.
      SingleSegmentTrackCreator m_singleSegmentTrackCreator;

      /// Instance of the orientation fixer
      TrackOrientator m_trackOrientator;

      /// Instance of track to the genfit translater
      GFTrackCandCreator m_gfTrackCandCreator;



    }; // end class SegmentPairTrackingWorker

  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif // SEGMENTPAIRTRACKINGWORKER_H
