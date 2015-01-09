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

#include <tracking/cdcLocalTracking/algorithms/Clusterizer.h>
#include <tracking/cdcLocalTracking/algorithms/MultipassCellularPathFinder.h>

#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

#include <tracking/cdcLocalTracking/creators/AxialStereoSegmentPairCreator.h>

#include <tracking/cdcLocalTracking/creators/TrackCreator.h>
#include <tracking/cdcLocalTracking/creators/SingleSegmentTrackCreator.h>
#include <tracking/cdcLocalTracking/creators/TrackOrientator.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Worker for building reconstructed tracks form reconstructed segments using segment triples
    template<class AxialStereoSegmentPairFilter, class AxialStereoSegmentPairNeighorChooser>
    class SegmentPairTrackingWorker {

    public:
      /// Constructor.
      SegmentPairTrackingWorker(bool copyToDataStoreForDebug = false): m_copyToDataStoreForDebug(copyToDataStoreForDebug), m_cellularPathFinder(0.0) {;}


      /// Destructor.
      ~SegmentPairTrackingWorker() {;}



      /// Forwards the initialize methods of the module to the segment creator and the neighborhood builder.
      void initialize() {

#ifdef CDCLOCALTRACKING_USE_ROOT
        StoreArray < CDCAxialStereoSegmentPair >::registerTransient("CDCAxialStereoSegmentPairs");
        StoreArray < CDCTrack >::registerTransient("CDCTracks");
#endif

        m_axialStereoSegmentPairCreator.initialize();
        m_axialStereoSegmentPairNeighborChooser.initialize();

      }



      /// Forwards the terminate method of the module to the segment creator and the neighborhood builder.
      void terminate() {
        m_axialStereoSegmentPairCreator.terminate();
        m_axialStereoSegmentPairNeighborChooser.terminate();
      }



      /// Generates tracks from the given segments.
      inline void apply(const std::vector< CDCRecoSegment2D >& segments,
                        StoreArray < genfit::TrackCand >& storedGFTrackCands) {

        // Regenerate the do not use flag for the cellular automaton
        for (const CDCRecoSegment2D & segment : segments) {
          for (const CDCRecoHit2D & recoHit2D : segment) {
            const CDCWireHit& wireHit = recoHit2D.getWireHit();
            wireHit.getAutomatonCell().unsetDoNotUseFlag();
          }
        }


        //create the segment pairs
        B2DEBUG(100, "Combining CDCReco2DSegments to CDCAxialStereoSegmentPairs");
        m_axialStereoSegmentPairs.clear();
        m_axialStereoSegmentPairCreator.create(segments, m_axialStereoSegmentPairs);
        B2DEBUG(100, "  Created " << m_axialStereoSegmentPairs.size()  << " CDCAxialStereoSegmentPair");

        //create the segment pair neighborhood
        B2DEBUG(100, "Creating the CDCAxialStereoSegmentPair neighborhood");
        m_axialStereoSegmentPairNeighborhood.clear();
        m_axialStereoSegmentPairNeighborhood.createUsing(m_axialStereoSegmentPairNeighborChooser, m_axialStereoSegmentPairs);
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
        for (const CDCTrack & track : m_tracks) {
          genfit::TrackCand* ptrTrackCand = storedGFTrackCands.appendNew();
          track.fillInto(*ptrTrackCand);
        }

        if (m_copyToDataStoreForDebug) {
          copyToDataStoreForDebug();
        }

        // Check that tracks are disjoint
        if (not areTracksDisjoint(m_tracks)) {
          B2WARNING("Tracks are not disjoint.");
        }


      }





    private:
      /// Checks if any two of the given tracks have common hist
      bool areTracksDisjoint(const std::vector<CDCTrack>& tracks) {
        bool result = true;
        // Prepare
        for (const CDCTrack & track : tracks) {
          for (const CDCRecoHit3D & recoHit3D : track) {
            const CDCWireHit& wireHit = recoHit3D.getWireHit();
            wireHit.getAutomatonCell().setCellState(-1.0);
          }
        }

        int iTrack = -1;
        for (const CDCTrack & track : tracks) {
          iTrack++;

          for (const CDCRecoHit3D & recoHit3D : track) {
            const CDCWireHit& wireHit = recoHit3D.getWireHit();
            const AutomatonCell& automatonCell = wireHit.getAutomatonCell();
            CellState cellState = automatonCell.getCellState();
            if (cellState == -1.0) {
              automatonCell.setCellState(iTrack);
            } else {
              result = false;
            }
          }
        }

        return result;

      }

      /// Helper function to copy intermediate objects to the data store for analysis from python.
      void copyToDataStoreForDebug() const {

#ifdef CDCLOCALTRACKING_USE_ROOT
        // IO for monitoring in python
        // IO segment triples
        StoreArray < CDCAxialStereoSegmentPair > storedAxialStereoSegmentPairs("CDCAxialStereoSegmentPairs");
        storedAxialStereoSegmentPairs.create();
        for (const CDCAxialStereoSegmentPair & axialStereoSegmentPair :  m_axialStereoSegmentPairs) {
          storedAxialStereoSegmentPairs.appendNew(axialStereoSegmentPair);
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
      /// Switch to write out the intermediate data objects out to DataStore.
      bool m_copyToDataStoreForDebug;

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

      /// Instance of the axial stereo pair neighbor chooser
      AxialStereoSegmentPairNeighorChooser m_axialStereoSegmentPairNeighborChooser;

      //cellular automat
      /// Instance of the cellular automaton.
      MultipassCellularPathFinder<CDCAxialStereoSegmentPair> m_cellularPathFinder;

      /// Instance of the track creator from paths.
      TrackCreator m_trackCreator;

      /// Instance of the leftover single segment tracks creator.
      SingleSegmentTrackCreator m_singleSegmentTrackCreator;

      /// Instance of the orientation fixer
      TrackOrientator m_trackOrientator;

    }; // end class SegmentPairTrackingWorker

  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif // SEGMENTPAIRTRACKINGWORKER_H
