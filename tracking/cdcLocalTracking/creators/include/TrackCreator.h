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
#include <tracking/cdcLocalTracking/eventdata/tracks/CDCAxialStereoSegmentPair.h>
#include <tracking/cdcLocalTracking/eventdata/tracks/CDCTrack.h>

#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>


namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing the reduction form segment triple sequences to reconstructed tracks
    class TrackCreator {

    public:

      /// Empty constructor.
      TrackCreator() {;}

      /// Empty destructor.
      ~TrackCreator() {;}



      /// Type of a path of segment triples.
      typedef std::vector<const CDCSegmentTriple*> CDCSegmentTripleTrack;

      /// Type of a path of axial stereo segment pairs.
      typedef std::vector<const CDCAxialStereoSegmentPair*> CDCAxialStereoSegmentPairTrack;



      /// Creates many CDCTracks from a path of segment triples by averaging the common parts and shifting the transverse travel distance of each part to match the end of the former triple.
      void create(const std::vector<CDCSegmentTripleTrack>& segmentTripleTracks,
                  std::vector<CDCTrack>& tracks) const;



      /// Creates many CDCTracks from a path of axial stereo segment pairs by averaging the common parts and shifting the transverse travel distance of each part to match the end of the former triple.
      void create(const std::vector<CDCAxialStereoSegmentPairTrack>& axialStereoSegmentPairTracks,
                  std::vector<CDCTrack>& tracks) const;



      /// Copies the hit content of the segment triple track to the CDCTrack.
      void create(const CDCSegmentTripleTrack& segmentTripleTrack,
                  CDCTrack& track) const;


      /// Copies the hit content of the axial stereo segment pair track to the CDCTrack.
      void create(const CDCAxialStereoSegmentPairTrack& axialStereoSegmentPairTrack,
                  CDCTrack& track) const;



      /// Copies the hit content of the segment to the CDCTrack
      bool create(const CDCRecoSegment2D& segment,
                  CDCTrack& track) const;



    private:

      ///Reconstruct the start segment of a triple and append it to the track
      void appendStartRecoHits3D(const CDCSegmentTriple& triple,
                                 FloatType perpSOffset,
                                 CDCTrack& recohits3D) const;

      /// Reconstruct the middle segment of a triple and append it to the track
      void appendMiddleRecoHits3D(const CDCSegmentTriple& triple,
                                  FloatType perpSOffset,
                                  CDCTrack& recohits3D) const;

      /// Reconstruct the end segment of a triple and append it to the track
      void appendEndRecoHits3D(const CDCSegmentTriple& triple,
                               FloatType perpSOffset,
                               CDCTrack& recohits3D) const;



      ///Reconstruct the start segment of a pair and append it to the track
      void appendStartRecoHits3D(const CDCAxialStereoSegmentPair& pair,
                                 FloatType perpSOffset,
                                 CDCTrack& recohits3D) const;

      ///Reconstruct the last segment of a pair and append it to the track
      void appendEndRecoHits3D(const CDCAxialStereoSegmentPair& pair,
                               FloatType perpSOffset,
                               CDCTrack& recohits3D) const;



      /// Reconstruct a segment with the two fits and append it to the track
      void appendRecoHits3D(const CDCRecoSegment2D& segment,
                            const CDCTrajectory2D& trajectory2D,
                            const CDCTrajectorySZ& trajectorySZ,
                            FloatType perpSOffset,
                            CDCTrack& recohits3D) const;



      /// Average overlapping segments of two triples and append it to the track
      /** @return the travel distance offset  for the following segment triple. */
      FloatType appendAverageStartEnd(const CDCSegmentTriple& triple,
                                      const CDCSegmentTriple& followingTriple,
                                      FloatType perpSOffset,
                                      CDCTrack& recohits3D) const;



      /// Average overlapping segments of two pairs and append it to the track
      /** @return the travel distance offset for the following segment pairs. */
      FloatType appendAverageStartEnd(const CDCAxialStereoSegmentPair& pair,
                                      const CDCAxialStereoSegmentPair& followingPair,
                                      FloatType perpSOffset,
                                      CDCTrack& recohits3D) const;

      /** Append the three dimensional reconstructed hits from the given segments averaged over two possible trajectories.
       *  In case of overlapping segments in segment triple or segment pairs segment usually participate in two trajectory fits.
       *  In principle we end up with to reconstructed positions from each of the fits which we have to average out. Also there
       *  is a mismatch of the start point of the travel distance scale since the reference points of the two trajectories differ.
       *  Hence we shift the travel distance scale of following trajectory such that both match on the first hit of the given segment.
       *  For the other hits the average of the travel distance is assumed.
       *  Furthermore an carryover offset of the travel distance can be introduced which might arise from earlier travel distance shifts
       *  on other previous parts of the whole track.
       *  Return value is the travel distance by which the following trajectories have to be shifted to match the overall track travel distance scale
       *  on the last hit of the given segment.*/
      FloatType appendAverage(const CDCRecoSegment2D& segment,
                              const CDCTrajectory2D& trajectory2D,
                              const CDCTrajectorySZ& trajectorySZ,
                              FloatType perpSOffset,
                              const CDCTrajectory2D& followingTrajectory2D,
                              const CDCTrajectorySZ& followingTrajectorySZ,
                              CDCTrack& recohits3D) const;


    }; // end class TrackCreator
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //TRACKCREATOR_H_
