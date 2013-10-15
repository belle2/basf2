/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOSEGMENT3D_H
#define CDCRECOSEGMENT3D_H


#include <tracking/cdcLocalTracking/eventdata/collections/CDCRecoHit3DVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    //for now a typedef is enough
    //may get additional methods if necessary
    /// A segment consisting of three dimensional reconsturcted hits
    typedef CDCRecoHit3DVector CDCRecoSegment3D;

  }
}


#endif //CDCRECOSEGMENT3D_H



