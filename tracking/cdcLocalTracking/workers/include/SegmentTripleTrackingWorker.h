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

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>
#include<framework/datastore/StoreArray.h>

#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>
#include <tracking/cdcLocalTracking/algorithms/NeighborhoodBuilder.h>
#include <tracking/cdcLocalTracking/neighbor_chooser/SimpleSegmentTripleNeighborChooser.h>

#include <tracking/cdcLocalTracking/algorithms/Clusterizer.h>
#include <tracking/cdcLocalTracking/algorithms/CellularAutomaton.h>
#include <tracking/cdcLocalTracking/algorithms/CellularPathFollower.h>

#include <tracking/cdcLocalTracking/creators/SegmentTripleCreator.h>
#include <tracking/cdcLocalTracking/creators/TrackCreator.h> //no decisions to optimize
#include <tracking/cdcLocalTracking/creators/SingleSegmentTrackCreator.h>
#include <tracking/cdcLocalTracking/creators/TrackOrientator.h>
#include <tracking/cdcLocalTracking/creators/GFTrackCandCreator.h>

#include <tracking/cdcLocalTracking/mclookup/CDCMCLookUp.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Worker for building reconstructed tracks form reconstructed segments using segment triples
    template<class SegmentTripleFilter, class SegmentTripleNeighborChooser>
    class SegmentTripleTrackingWorker {

    public:

      /** Constructor. */
      SegmentTripleTrackingWorker() {;}


      /** Destructor.*/
      ~SegmentTripleTrackingWorker() {;}

      void init() {

#ifdef CDCLOCALTRACKING_USE_ROOT
        StoreArray < CDCSegmentTriple >::registerTransient("CDCSegmentTriples");
        StoreArray < CDCTrack >::registerTransient("CDCTracks");
#endif

      }

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
        m_segmentTriple_neighborhoodBuilder.create(m_segmentTriples, m_segmentTripleNeighborhood);
        B2DEBUG(100, "  Created " << m_segmentTripleNeighborhood.size()  << " SegmentTripleNeighborhoods");
        if (growMany) {

          //apply the cellular automation
          B2DEBUG(100, "Apply cellular automat for CDCSegmentTriples");
          const CDCSegmentTriple* highestCell
            =  m_segmentTripleAutomaton.applyTo(m_segmentTriples, m_segmentTripleNeighborhood);
          B2DEBUG(100, "  MaximalState " << highestCell->getAutomatonCell().getCellState());

          //create the tracks by following the highest states in the segment triples
          B2DEBUG(100, "Follow the longest paths");
          m_segmentTripleTracks.clear();
          m_tipleSegmentCellularFollower.followAll(m_segmentTriples,
                                                   m_segmentTripleNeighborhood,
                                                   m_segmentTripleTracks);

          B2DEBUG(100, "  Created " << m_segmentTripleTracks.size()  << " SegmentTripleTracks");

          //reduce to plain tracks
          B2DEBUG(100, "Reducing the SegmentTripleTracks to CDCTracks");
          m_tracks.clear();
          m_trackCreator.create(m_segmentTripleTracks, m_tracks);
          B2DEBUG(100, "  Created " << m_tracks.size()  << " CDCTracks");

          //TODO: die out analysis

        } else { // not growMany
          //multiple passes if growMany is active and one track is created at a time
          //no best candidate analysis needed

          m_segmentTripleTracks.clear();
          bool created = 0;
          B2DEBUG(100, "Apply multipass cellular automat for CDCSegmentTriples");
          do {
            //apply the cellular automation
            //B2DEBUG(100,"Apply cellular automat for CDCSegmentTriples");
            const CDCSegmentTriple* highestCell
              =  m_segmentTripleAutomaton.applyTo(m_segmentTriples, m_segmentTripleNeighborhood);
            if (highestCell != nullptr) {
              //B2DEBUG(100,"  MaximalState " << highestCell->getCellState());
            }

            //create the segments by following the highest states in the segment triples
            //B2DEBUG(100,"Follow the longest paths");
            m_segmentTripleTracks.push_back(std::vector<const CDCSegmentTriple*>());
            std::vector<const CDCSegmentTriple*>& segmentTripleTrack = m_segmentTripleTracks.back();
            created = m_tipleSegmentCellularFollower.followSingle(highestCell, m_segmentTripleNeighborhood,
                                                                  segmentTripleTrack, 0);
            //B2DEBUG(100,"  Created SegmentTripleTracks with " <<
            //               segmentTripleTrack.size() << " segment triples");

            //Block the used segments
            for (const CDCSegmentTriple * triple : segmentTripleTrack) {
              triple->setDoNotUse();
            }

            //Block the triples that use already used segments as well
            for (const CDCSegmentTriple & triple : m_segmentTriples) {
              triple.receiveDoNotUse();
            }

          } while (created != 0);

          m_segmentTripleTracks.pop_back();
          B2DEBUG(100, "  Created " << m_segmentTripleTracks.size()  << " SegmentTripleTracks");

          //reduce to plain tracks
          B2DEBUG(100, "Reducing the SegmentTripleTracks to CDCTracks");
          m_tracks.clear();
          m_trackCreator.create(m_segmentTripleTracks, m_tracks);
          B2DEBUG(100, "  Created " << m_tracks.size()  << " CDCTracks");



        } // end if( growMany )

        m_singleSegmentTrackCreator.append(recoSegments, m_tracks);

        m_trackOrientator.markOrientation(m_tracks);

        //create the gftracks
        B2DEBUG(100, "Creating the genfit::TrackCands");
        m_gfTrackCandCreator.create(m_tracks, storedGFTrackCands);
        B2DEBUG(100, "  Created " << storedGFTrackCands.getEntries()  << " genfit::TrackCands");

        // IO for monitoring in python
#ifdef CDCLOCALTRACKING_USE_ROOT
        // IO segment triples
        {
          B2DEBUG(100, "  Creating the StoreArray for the CDCSegmentTriple");
          StoreArray < CDCSegmentTriple > storedSegmentTriples("CDCSegmentTriples");
          storedSegmentTriples.create();

          for (std::set< CDCSegmentTriple >::iterator itSegmentTriple = m_segmentTriples.begin();
               itSegmentTriple != m_segmentTriples.end(); ++itSegmentTriple) {

            storedSegmentTriples.appendNew(*itSegmentTriple);
            //B2DEBUG(100,"  ##### Copying " << itSegmentTriple->getStartISuperLayer() <<
            //                           " " << itSegmentTriple->getMiddleISuperLayer() <<
            //                           " " << itSegmentTriple->getEndISuperLayer() );
          }
          B2DEBUG(100, "  Created " << storedSegmentTriples.getEntries()  << " CDCSegmentTriples");
        }

        // IO tracks
        {

          B2DEBUG(100, "  Creating the StoreArray for the CDCTracks");
          StoreArray < CDCTrack > storedTracks("CDCTracks");
          storedTracks.create();
          B2DEBUG(100, "  Copying the CDCTracks to the StoreArray");
          for (std::vector< CDCTrack >::iterator itTrack = m_tracks.begin();
               itTrack != m_tracks.end(); ++itTrack) {
            storedTracks.appendNew(*itTrack);
          }

        }
#endif

      }


    private:
      //object pools
      std::set<CDCSegmentTriple> m_segmentTriples;
      WeightedNeighborhood<const CDCSegmentTriple> m_segmentTripleNeighborhood;
      std::vector< std::vector<const CDCSegmentTriple*> > m_segmentTripleTracks;
      std::vector<CDCTrack> m_tracks;

      //object creators
      SegmentTripleCreator<SegmentTripleFilter> m_segmentTripleCreator;

      //neighborhood builders
      NeighborhoodBuilder < std::set<CDCSegmentTriple>,
                          SegmentTripleNeighborChooser > m_segmentTriple_neighborhoodBuilder;

      //cellular automat
      CellularAutomaton< std::set<CDCSegmentTriple> > m_segmentTripleAutomaton;

      static const bool growMany = false;

      //CellularPathFollower< std::set<CDCSegmentTriple> > m_tipleSegmentCellularFollower;
      CellularPathFollower<CDCSegmentTriple> m_tipleSegmentCellularFollower;

      TrackCreator m_trackCreator;

      SingleSegmentTrackCreator m_singleSegmentTrackCreator;

      TrackOrientator m_trackOrientator;

      GFTrackCandCreator m_gfTrackCandCreator;

    }; // end class SegmentTripleTrackingWorker
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SEGMENTTRIPLETRACKINGWORKER_H_
