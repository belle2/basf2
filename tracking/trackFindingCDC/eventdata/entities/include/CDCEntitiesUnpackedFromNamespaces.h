/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCENTITIESUNPACKEDFROMNAMESPACES_H
#define CDCENTITIESUNPACKEDFROMNAMESPACES_H


#include "CDCEntities.h"
/* #include <tracking/cdcLocalTracking/eventdata/entities/CDCGenHit.h> */
/* #include <tracking/cdcLocalTracking/eventdata/entities/CDCWireHit.h> */
/* #include <tracking/cdcLocalTracking/eventdata/entities/CDCRLWireHit.h> */
/* #include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit2D.h> */
/* #include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoTangent.h> */
/* #include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h> */
/* #include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit3D.h> */



#ifdef __CINT__
//#ifdef CDCLOCALTRACKING_ROOTIFY_CDCVECTOR
// Because ROOTCINT does not like namespaces inside template parameters
// we have to make each object, which we want the instantiate the template
// available outside any namespace. Therefore we also have to include each of
// them as well.

typedef Belle2::CDCLocalTracking::CDCGenHit CDCGenHit;
typedef Belle2::CDCLocalTracking::CDCWireHit CDCWireHit;
typedef Belle2::CDCLocalTracking::CDCRLWireHit CDCRLWireHit;
typedef Belle2::CDCLocalTracking::CDCRecoHit2D CDCRecoHit2D;
typedef Belle2::CDCLocalTracking::CDCRecoTangent CDCRecoTangent;
typedef Belle2::CDCLocalTracking::CDCRecoFacet CDCRecoFacet;
typedef Belle2::CDCLocalTracking::CDCRecoHit3D CDCRecoHit3D;

//#endif // CDCLOCALTRACKING_ROOTIFY_CDCVECTOR
#endif // __CINT__

#endif //CDCENTITIESUNPACKEDFROMNAMESPACES_H
