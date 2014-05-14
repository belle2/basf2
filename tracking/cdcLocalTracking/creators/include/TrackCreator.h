/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRACKCREATOR_H_
#define TRACKCREATOR_H_

#include <vector>

#include <tracking/cdcLocalTracking/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/cdcLocalTracking/eventdata/tracks/CDCTrack.h>

#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>


namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing the reduction form segment triple sequences to reconstructed tracks
    class TrackCreator {

    public:

      /** Constructor. */
      TrackCreator() {;}

      /** Destructor.*/
      ~TrackCreator() {;}


      typedef Belle2::CDCLocalTracking::CDCSegmentTriple Item;
      typedef std::vector<const Item*> CDCSegmentTripleTrack;


      void create(
        const std::vector<CDCSegmentTripleTrack>& segmentTripleTracks,
        std::vector<CDCTrack>& tracks
      ) const;



      void create(
        const CDCSegmentTripleTrack& segmentTripleTrack,
        CDCTrack& track
      ) const;


      bool create(
        const CDCRecoSegment2D& segment,
        CDCTrack& track
      ) const;



    private:

      ///Reconstruct the start segment of a triple and append it to the track
      void appendStartRecoHits3D(
        const CDCSegmentTriple& triple,
        FloatType perpSOffset,
        CDCRecoSegment3D& recohits3D
      ) const;

      ///Reconstruct the middle segment of a triple and append it to the track
      void appendMiddleRecoHits3D(
        const CDCSegmentTriple& triple,
        FloatType perpSOffset,
        CDCRecoSegment3D& recohits3D
      ) const;

      ///Reconstruct the end segment of a triple and append it to the track
      void appendEndRecoHits3D(
        const CDCSegmentTriple& triple,
        FloatType perpSOffset,
        CDCRecoSegment3D& recohits3D
      ) const;

      ///Reconstruct a segment with the two fits and append it to the track
      void appendRecoHits3D(
        const CDCRecoSegment2D& segment,
        const CDCTrajectory2D& trajectory2D,
        const CDCTrajectorySZ& trajectorySZ,
        FloatType perpSOffset,
        CDCRecoSegment3D& recohits3D
      ) const;

      /// Average overlapping segments of two triples and append it to the track
      /** @return the travel distance offset  for the following triple segments */
      FloatType appendAverageStartEnd(
        const CDCSegmentTriple& triple,
        const CDCSegmentTriple& followingTriple,
        FloatType perpSOffset,
        CDCRecoSegment3D& recohits3D
      ) const;


    }; // end class TrackCreator
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //TRACKCREATOR_H_
