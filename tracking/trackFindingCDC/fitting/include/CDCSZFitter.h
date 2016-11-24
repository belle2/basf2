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

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCStereoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCAxialSegment2D.h>

#include <tracking/trackFindingCDC/fitting/CDCSZObservations.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCObservations2D;

    /// Class implementing the z coordinate over travel distance line fit.
    class CDCSZFitter {

    public:
      /// Getter for a standard sz line fitter instance.
      static const CDCSZFitter& getFitter();

      /// Returns the fitted sz trajectory of the track with the z-information of all stereo hits of the number
      /// of stereo hits is big enough. Else return the basic assumption.
      CDCTrajectorySZ fitWithStereoHits(const CDCTrack& track) const
      {
        const bool onlyStereo = true;
        CDCSZObservations observationsSZ(EFitVariance::c_Proper, onlyStereo);
        observationsSZ.appendRange(track);
        if (observationsSZ.size() > 3) {
          CDCTrajectorySZ szTrajectory;
          update(szTrajectory, observationsSZ);
          return szTrajectory;
        } else {
          return CDCTrajectorySZ::basicAssumption();
        }
      }

      /// Returns a fitted trajectory
      CDCTrajectorySZ fit(const CDCStereoSegment2D& stereoSegment,
                          const CDCTrajectory2D& axialTrajectory2D) const
      {
        CDCTrajectorySZ trajectorySZ;
        update(trajectorySZ, stereoSegment, axialTrajectory2D);
        return trajectorySZ;
      }

      /// Fits a linear sz trajectory to the z and s coordinates in the stereo segment.
      CDCTrajectorySZ fit(const CDCSegment3D& segment3D) const
      {
        CDCSZObservations observationsSZ;
        observationsSZ.appendRange(segment3D);
        return fit(std::move(observationsSZ));
      }

      /// Fits a linear sz trajectory to the s and z coordinates given in the observations.
      CDCTrajectorySZ fit(CDCSZObservations observationsSZ) const
      {
        CDCTrajectorySZ trajectorySZ;
        update(trajectorySZ, observationsSZ);
        return trajectorySZ;
      }

      /// Legacy - Fits a linear sz trajectory to the x and y coordinates interpreted as sz space
      CDCTrajectorySZ fit(const CDCObservations2D& observations2D) const;

      /**
       *  Fit a linear sz trajectory to the reconstructed stereo segment.
       *  It uses the normal fitting algorithm but does so multiple times:
       *  In every iteration, one hit is excluded from the observation set and the rest is fitted. In the end, the mean over the
       *  fitting parameters is built and returned.
       *
       *  TODO:
       *    - Use the median.
       *    - Use RANSAC instead of Theil-Sen.
       *    - Think about the parameters better.
       */
      CDCTrajectorySZ fitUsingSimplifiedTheilSen(const CDCSegment3D& segment3D) const;

      /// Updates the trajectory of the axial stereo segment pair inplace
      void update(const CDCSegmentPair& segmentPair) const;

      /// Update the given sz trajectory reconstructing the stereo segment with a near by axial segment
      void update(CDCTrajectorySZ& trajectorySZ,
                  const CDCStereoSegment2D& stereoSegment,
                  const CDCTrajectory2D& axialTrajectory2D) const;

      /// Update the trajectory with a fit to the observations.
      void update(CDCTrajectorySZ& trajectorySZ, CDCSZObservations& observationsSZ) const;
    };
  }
}
