/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSZObservations;
    class CDCObservations2D;

    class CDCTrack;
    class CDCSegmentPair;
    class CDCSegment3D;
    class CDCSegment2D;
    class CDCTrajectory2D;
    class CDCTrajectorySZ;

    /// Class implementing the z coordinate over travel distance line fit.
    class CDCSZFitter {

    public:
      /// Getter for a standard sz line fitter instance.
      static const CDCSZFitter& getFitter();

      /// Returns the fitted sz trajectory of the track with the z-information of all stereo hits of the number
      /// of stereo hits is big enough. Else return the basic assumption.
      CDCTrajectorySZ fitWithStereoHits(const CDCTrack& track) const;

      /// Returns a fitted trajectory
      CDCTrajectorySZ fit(const CDCSegment2D& stereoSegment,
                          const CDCTrajectory2D& axialTrajectory2D) const;

      /// Fits a linear sz trajectory to the z and s coordinates in the stereo segment.
      CDCTrajectorySZ fit(const CDCSegment3D& segment3D) const;

      /// Fits a linear sz trajectory to the s and z coordinates given in the observations.
      CDCTrajectorySZ fit(CDCSZObservations observationsSZ) const;

      /// Legacy - Fits a linear sz trajectory to the x and y coordinates interpreted as sz space
      CDCTrajectorySZ fit(const CDCObservations2D& observations2D) const;

      /// Updates the trajectory of the axial stereo segment pair inplace
      void update(const CDCSegmentPair& segmentPair) const;

      /// Update the given sz trajectory reconstructing the stereo segment with a near by axial segment
      void update(CDCTrajectorySZ& trajectorySZ,
                  const CDCSegment2D& stereoSegment,
                  const CDCTrajectory2D& axialTrajectory2D) const;

      /// Update the trajectory with a fit to the observations.
      void update(CDCTrajectorySZ& trajectorySZ, CDCSZObservations& observationsSZ) const;
    };
  }
}
