/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCWIREHITCLUSTER_H
#define CDCWIREHITCLUSTER_H


#include <tracking/cdcLocalTracking/eventdata/collections/CDCWireHitPtrSet.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    //for now a typedef is enough
    //may get additional methods if necessary

    // An agregation of CDCWireHits
    typedef CDCWireHitPtrSet CDCWireHitCluster;

  }
}


#endif //CDCWIREHITCLUSTER_H



