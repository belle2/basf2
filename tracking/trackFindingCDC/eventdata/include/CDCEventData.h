/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCEVENTDATA_H
#define CDCEVENTDATA_H

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectories.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCEntities.h>
#include <tracking/trackFindingCDC/eventdata/collections/CDCCollections.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegments.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTracks.h>



namespace Belle2 {
  namespace CDCLocalTracking {


    //some additional transient types for exchange between creator objects
    typedef CDCRecoFacetVector CDCRecoFacetCollection;

  }
}


#endif //CDCEVENTDATA_H

