/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/ca/Path.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Class providing the reduction form segment triple paths to reconstructed tracks
    class TrackCreator {



    public:
      /**
       *  Creates many CDCTracks from a path of segment triples by averaging the common parts and
       *  shifting the transverse travel distance of each part to match the end of the former triple.
       */
      void create(const std::vector<Path<const CDCSegmentTriple> >& segmentTripleTracks,
                  std::vector<CDCTrack>& tracks) const;

      /**
       *  Creates many CDCTracks from a path of axial stereo segment pairs by averaging the common parts and
       *  shifting the transverse travel distance of each part to match the end of the former triple.
       */
      void create(const std::vector<Path<const CDCSegmentPair> >& segmentPairTracks,
                  std::vector<CDCTrack>& tracks) const;

      /// Copies the hit content of the segment triple track to the CDCTrack.
      void create(const Path<const CDCSegmentTriple>& segmentTriplePath,
                  CDCTrack& track) const;

      /// Copies the hit content of the axial stereo segment pair track to the CDCTrack.
      void create(const Path<const CDCSegmentPair>& segmentPairPath,
                  CDCTrack& track) const;

      /// Copies the hit content of the hit vector track to the CDCTrack. Do not use any taken or masked hits.
      void create(std::vector<const CDCWireHit*>& hits, CDCTrack& track) const;

    private:
      /// Reconstruct the start segment of a triple and append it to the track
      void appendStartRecoHits3D(const CDCSegmentTriple& triple,
                                 double perpSOffset,
                                 CDCTrack& recohits3D) const;

      /// Reconstruct the middle segment of a triple and append it to the track
      void appendMiddleRecoHits3D(const CDCSegmentTriple& triple,
                                  double perpSOffset,
                                  CDCTrack& recohits3D) const;

      /// Reconstruct the end segment of a triple and append it to the track
      void appendEndRecoHits3D(const CDCSegmentTriple& triple,
                               double perpSOffset,
                               CDCTrack& recohits3D) const;



      /// Reconstruct the start segment of a pair and append it to the track
      void appendStartRecoHits3D(const CDCSegmentPair& pair,
                                 double perpSOffset,
                                 CDCTrack& recohits3D) const;

      /// Reconstruct the last segment of a pair and append it to the track
      void appendEndRecoHits3D(const CDCSegmentPair& pair,
                               double perpSOffset,
                               CDCTrack& recohits3D) const;



      /// Reconstruct a segment with the two fits and append it to the track
      void appendRecoHits3D(const CDCRecoSegment2D& segment,
                            const CDCTrajectory2D& trajectory2D,
                            const CDCTrajectorySZ& trajectorySZ,
                            double perpSOffset,
                            CDCTrack& recohits3D) const;


      /**
       *  Average overlapping segments of two triples and append it to the track
       *  @return the travel distance offset  for the following segment triple.
       */
      double appendAverageStartEnd(const CDCSegmentTriple& triple,
                                   const CDCSegmentTriple& followingTriple,
                                   double perpSOffset,
                                   CDCTrack& recohits3D) const;

      /**
       *  Average overlapping segments of two pairs and append it to the track
       *  @return the travel distance offset for the following segment pairs.
       */
      double appendAverageStartEnd(const CDCSegmentPair& pair,
                                   const CDCSegmentPair& followingPair,
                                   double perpSOffset,
                                   CDCTrack& recohits3D) const;

      /**
       *  Append the three dimensional reconstructed hits from the given segments averaged over two possible trajectories.
       *  In case of overlapping segments in segment triple or segment pairs segment usually participate in two trajectory fits.
       *  In principle we end up with two reconstructed positions from each of the fits which we have to average out. Also there
       *  is a mismatch of the start point of the travel distance scale since the reference points of the two trajectories differ.
       *  Hence we shift the travel distance scale of the following trajectory such that both match on the first hit of the given segment.
       *  For the other hits the average of the travel distance is assumed.
       *  Furthermore an carry over offset of the travel distance can be introduced which might arise from earlier travel distance shifts
       *  on other previous parts of the whole track.
       *  Return value is the travel distance by which the following trajectories have to be shifted to match the overall track travel distance scale
       *  on the last hit of the given segment.
       */
      double appendAverage(const CDCRecoSegment2D& segment,
                           const CDCTrajectory2D& trajectory2D,
                           const CDCTrajectorySZ& trajectorySZ,
                           double perpSOffset,
                           const CDCTrajectory2D& followingTrajectory2D,
                           const CDCTrajectorySZ& followingTrajectorySZ,
                           CDCTrack& recohits3D) const;

    }; // end class TrackCreator
  } // end namespace TrackFindingCDC
} // end namespace Belle2
