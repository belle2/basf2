/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

namespace Belle2 {

  namespace TrackingUtilities {
    class CDCTrack;
    class CDCTrajectory2D;
    class CDCTrajectorySZ;
    class CDCSegmentPair;
    class CDCSegment3D;
    class CDCSegment2D;
  }
  namespace TrackFindingCDC {
    class CDCSZObservations;
    class CDCObservations2D;

    /// Class implementing the z coordinate over travel distance line fit.
    class CDCSZFitter {

    public:
      /// Getter for a standard sz line fitter instance.
      static const CDCSZFitter& getFitter();

      /// Returns the fitted sz trajectory of the track with the z-information of all stereo hits of the number
      /// of stereo hits is big enough. Else return the basic assumption.
      static TrackingUtilities::CDCTrajectorySZ fitWithStereoHits(const TrackingUtilities::CDCTrack& track);

      /// Returns a fitted trajectory
      static TrackingUtilities::CDCTrajectorySZ fit(const TrackingUtilities::CDCSegment2D& stereoSegment,
                                                    const TrackingUtilities::CDCTrajectory2D& axialTrajectory2D);

      /// Fits a linear sz trajectory to the z and s coordinates in the stereo segment.
      static TrackingUtilities::CDCTrajectorySZ fit(const TrackingUtilities::CDCSegment3D& segment3D);

      /// Fits a linear sz trajectory to the s and z coordinates given in the observations.
      static TrackingUtilities::CDCTrajectorySZ fit(CDCSZObservations observationsSZ);

      /// Legacy - Fits a linear sz trajectory to the x and y coordinates interpreted as sz space
      static TrackingUtilities::CDCTrajectorySZ fit(const CDCObservations2D& observations2D);

      /// Updates the trajectory of the axial stereo segment pair inplace
      static void update(const TrackingUtilities::CDCSegmentPair& segmentPair);

      /// Update the given sz trajectory reconstructing the stereo segment with a near by axial segment
      static void update(TrackingUtilities::CDCTrajectorySZ& trajectorySZ,
                         const TrackingUtilities::CDCSegment2D& stereoSegment,
                         const TrackingUtilities::CDCTrajectory2D& axialTrajectory2D);

      /// Update the trajectory with a fit to the observations.
      static void update(TrackingUtilities::CDCTrajectorySZ& trajectorySZ, CDCSZObservations& observationsSZ);
    };
  }
}
