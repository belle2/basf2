/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCAXIALSTEREOFUSION_H
#define CDCAXIALSTEREOFUSION_H

#include <tracking/trackFindingCDC/rootification/SwitchableRootificationBase.h>
#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectories.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialStereoSegmentPair.h>

#include "TMatrixD.h"
#include "TVectorD.h"

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCAxialStereoFusion : public SwitchableRootificationBase {

    public:
      static TMatrixD calcAmbiguity(const CDCRecoSegment2D& recoSegment2D,
                                    const CDCTrajectory2D& trajectory2D);

      static TMatrixD calcAmbiguity(const CDCRecoSegment3D& recoSegment3D,
                                    const CDCTrajectory2D& trajectory2D);



      static CDCTrajectory3D fuseTrajectories(const CDCRecoSegment2D& startSegment,
                                              const CDCRecoSegment2D& endSegment);

      static void fuseTrajectories(const CDCAxialStereoSegmentPair& axialStereoSegmentPair);

      static CDCTrajectory3D reconstructFuseTrajectories(const CDCRecoSegment2D& startSegment,
                                                         const CDCRecoSegment2D& endSegment,
                                                         bool priorityOnSZ = true);

      static void reconstructFuseTrajectories(const CDCAxialStereoSegmentPair& axialStereoSegmentPair,
                                              bool priorityOnSZ = true);

    public:
      /** ROOT Macro to make CDCAxialStereoFusion a ROOT class.*/
      TRACKFINDINGCDC_SwitchableClassDef(CDCAxialStereoFusion, 1);

    }; // end class
  } // end namespace TrackFindingCDC
} // namespace Belle2
#endif // CDCAXIALSTEREOFUSION_H
