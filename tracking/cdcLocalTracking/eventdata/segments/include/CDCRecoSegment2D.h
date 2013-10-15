/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOSEGMENT2D_H_
#define CDCRECOSEGMENT2D_H_

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/collections/CDCRecoHit2DVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {


    //for now a typedef is enough
    //may get additional methods if necessary
    /// A segment consisting of three dimensional reconsturcted hits
    typedef CDCRecoHit2DVector CDCRecoSegment2D;


  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOSEGMENT2D_H_
