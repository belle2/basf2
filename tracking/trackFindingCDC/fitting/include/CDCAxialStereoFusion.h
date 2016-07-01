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

    class CDCAxialStereoFusion {

    public:
      static UncertainPerigeeCircle fuse(const UncertainPerigeeCircle& startPerigeeCircle,
                                         const UncertainPerigeeCircle& endPerigeeCircle);

      static UncertainHelix fuse(const UncertainPerigeeCircle& startPerigeeCircle,
                                 const JacobianMatrix<3, 5>& startAmbiguityMatrix,
                                 const UncertainPerigeeCircle& endPerigeeCircle,
                                 const JacobianMatrix<3, 5>& endAmbiguityMatrix);

      static UncertainHelix fuse(const UncertainHelix& startHelix,
                                 const UncertainPerigeeCircle& endPerigeeCircle,
                                 const JacobianMatrix<3, 5>& endAmbiguityMatrix)
      { return fuse(endPerigeeCircle, endAmbiguityMatrix, startHelix); }

      static UncertainHelix fuse(const UncertainPerigeeCircle& startPerigeeCircle,
                                 const JacobianMatrix<3, 5>& startAmbiguityMatrix,
                                 const UncertainHelix& endHelix);

      static UncertainHelix fuse(const UncertainHelix& startHelix,
                                 const UncertainHelix& endHelix);

    public:
      static JacobianMatrix<3, 5> calcAmbiguity(const CDCRecoSegment2D& recoSegment2D,
                                                const CDCTrajectory2D& trajectory2D);

      static JacobianMatrix<3, 5> calcAmbiguity(const CDCRecoSegment3D& recoSegment3D,
                                                const CDCTrajectory2D& trajectory2D);


    public:
      static CDCTrajectory3D fuseTrajectories(const CDCRecoSegment2D& startSegment,
                                              const CDCRecoSegment2D& endSegment);

      static void fuseTrajectories(const CDCSegmentPair& segmentPair);

      static CDCTrajectory3D reconstructFuseTrajectories(const CDCRecoSegment2D& startSegment,
                                                         const CDCRecoSegment2D& endSegment,
                                                         bool priorityOnSZ = true);

      static void reconstructFuseTrajectories(const CDCSegmentPair& segmentPair,
                                              bool priorityOnSZ = true);

    }; // end class
  } // end namespace TrackFindingCDC
} // namespace Belle2
