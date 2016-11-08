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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

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

      void fusePreliminary(const CDCSegmentPair& segmentPair);

      /**
       * Combine the trajectories of the two given segments to a full helix trajectory
       */
      CDCTrajectory3D reconstructFuseTrajectories(const CDCRecoSegment2D& fromSegment2D,
                                                  const CDCRecoSegment2D& toSegment2D);

      CDCTrajectory3D fusePreliminary(const CDCRecoSegment2D& fromSegment2D,
                                      const CDCRecoSegment2D& toSegment2D);

      /// Combine the two segments given a prelimiary reference trajectory to which a creation is applied
      CDCTrajectory3D reconstructFuseTrajectories(const CDCRecoSegment2D& fromSegment2D,
                                                  const CDCRecoSegment2D& toSegment2D,
                                                  const CDCTrajectory3D& preliminaryTrajectory3D);

    public:
      /**
       *  Calculate the ambiguity of the helix parameters relative to the three circle
       *  parameters given the hit content of the segment and their stereo displacement.
       */
      JacobianMatrix<3, 5> calcAmbiguity(const CDCRecoSegment3D& recoSegment3D,
                                         const CDCTrajectory2D& trajectory2D);


    private:
      /// Swtich to reestimate the  drift length.
      bool m_reestimateDriftLength;

    };
  }
}
