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

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/trajectories/CDCTrajectories.h>
#include <tracking/cdcLocalTracking/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/cdcLocalTracking/eventdata/tracks/CDCAxialStereoSegmentPair.h>

#include "TMatrixD.h"
#include "TVectorD.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    class CDCAxialStereoFusion : public UsedTObject {

    public:
      static TMatrixD calcAmbiguity(const CDCRecoSegment2D& recoSegment2D,
                                    const CDCTrajectory2D& trajectory2D);

      static TMatrixD calcAmbiguity(const CDCRecoSegment3D& recoSegment3D,
                                    const CDCTrajectory2D& trajectory2D);



      static CDCTrajectory3D fuseTrajectories(const CDCRecoSegment2D& startSegment,
                                              const CDCRecoSegment2D& endSegment);

      static void fuseTrajectories(const CDCAxialStereoSegmentPair& axialStereoSegmentPair);


      static CDCTrajectory3D reconstructFuseTrajectories(const CDCRecoSegment2D& startSegment,
                                                         const CDCRecoSegment2D& endSegment);

      static void reconstructFuseTrajectories(const CDCAxialStereoSegmentPair& axialStereoSegmentPair);

    public:
      /** ROOT Macro to make CDCAxialStereoFusion a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCAxialStereoFusion, 1);

    }; // end class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCAXIALSTEREOFUSION_H
