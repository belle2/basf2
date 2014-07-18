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

#include "TMatrixD.h"
#include "TVectorD.h"


namespace Belle2 {
  namespace CDCLocalTracking {

    TMatrixD calcAmbiguity(const CDCRecoSegment2D& segment,
                           const CDCTrajectory2D& trajectory2D);

    CDCTrajectory3D fuseTrajectories(const CDCRecoSegment2D& startSegment,
                                     const CDCRecoSegment2D& endSegment);

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCAXIALSTEREOFUSION_H
