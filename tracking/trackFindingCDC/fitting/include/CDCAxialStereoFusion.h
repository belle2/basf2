/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/utils/DriftLengthEstimator.h>

#include <tracking/trackFindingCDC/geometry/HelixParameters.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentPair;
    class CDCSegment3D;
    class CDCSegment2D;
    class CDCTrajectory3D;
    class CDCTrajectory2D;

    /**
     * Utility class implementing the Kalmanesk combination of to two dimensional
     * trajectories to one three dimensional trajectory
     */
    class CDCAxialStereoFusion {

    public:
      /// Constructor setting up the options of the fit.
      explicit CDCAxialStereoFusion(bool reestimateDriftLength = true)
        : m_reestimateDriftLength(reestimateDriftLength)
      {
      }

    public:
      /**
       *  Combine the two trajectories of the segments in the pair and assign the
       *  resulting three dimensional trajectory to the segment pair
       */
      void reconstructFuseTrajectories(const CDCSegmentPair& segmentPair);

      /**
       *  Fit the given segment pair using the preliminary helix fit without proper covariance matrix.
       *
       *  Updates the contained trajectory.
       */
      void fusePreliminary(const CDCSegmentPair& segmentPair);

      /**
       * Combine the trajectories of the two given segments to a full helix trajectory
       */
      CDCTrajectory3D reconstructFuseTrajectories(const CDCSegment2D& fromSegment2D,
                                                  const CDCSegment2D& toSegment2D);

      /**
       *  Fit the two given segments together using the preliminary helix fit without proper covariance matrix.
       *
       *  The fit is used as the expansion point for the least square fuse fit with proper covariance.
       */
      CDCTrajectory3D fusePreliminary(const CDCSegment2D& fromSegment2D,
                                      const CDCSegment2D& toSegment2D);

      /// Combine the two segments given a prelimiary reference trajectory to which a creation is applied
      CDCTrajectory3D reconstructFuseTrajectories(const CDCSegment2D& fromSegment2D,
                                                  const CDCSegment2D& toSegment2D,
                                                  const CDCTrajectory3D& preliminaryTrajectory3D);

    public:
      /**
       *  Calculate the ambiguity of the helix parameters relative to the three circle
       *  parameters given the hit content of the segment and their stereo displacement.
       */
      PerigeeHelixAmbiguity calcAmbiguity(const CDCSegment3D& segment3D,
                                          const CDCTrajectory2D& trajectory2D);


    private:
      /// Swtich to reestimate the  drift length.
      bool m_reestimateDriftLength;

      /// Helper object to carry out the drift length estimation
      DriftLengthEstimator m_driftLengthEstimator;
    };
  }
}
