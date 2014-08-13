/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCSEGMENTS_H
#define CDCSEGMENTS_H

#include "CDCWireHitCluster.h"
#include "CDCWireHitSegment.h"
#include "CDCRecoSegment2D.h"
#include "CDCRecoSegment3D.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    //doing two typedef just to clearify the intention of use in other objects
    //( hence whether an axial or stereo segment shall be given as a parameter )
    typedef CDCRecoSegment2D CDCAxialRecoSegment2D;
    typedef CDCRecoSegment2D CDCStereoRecoSegment2D;

  }
}


#endif //CDCSEGMENTS_H



