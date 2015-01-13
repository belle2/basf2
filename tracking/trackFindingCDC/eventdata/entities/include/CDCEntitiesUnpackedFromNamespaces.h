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
/* #include <tracking/trackFindingCDC/eventdata/entities/CDCGenHit.h> */
/* #include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h> */
/* #include <tracking/trackFindingCDC/eventdata/entities/CDCRLWireHit.h> */
/* #include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit2D.h> */
/* #include <tracking/trackFindingCDC/eventdata/entities/CDCRecoTangent.h> */
/* #include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h> */
/* #include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h> */



#ifdef __CINT__
//#ifdef TRACKFINDINGCDC_ROOTIFY_CDCVECTOR
// Because ROOTCINT does not like namespaces inside template parameters
// we have to make each object, which we want the instantiate the template
// available outside any namespace. Therefore we also have to include each of
// them as well.

typedef Belle2::TrackFindingCDC::CDCGenHit CDCGenHit;
typedef Belle2::TrackFindingCDC::CDCWireHit CDCWireHit;
typedef Belle2::TrackFindingCDC::CDCRLWireHit CDCRLWireHit;
typedef Belle2::TrackFindingCDC::CDCRecoHit2D CDCRecoHit2D;
typedef Belle2::TrackFindingCDC::CDCRecoTangent CDCRecoTangent;
typedef Belle2::TrackFindingCDC::CDCRecoFacet CDCRecoFacet;
typedef Belle2::TrackFindingCDC::CDCRecoHit3D CDCRecoHit3D;

//#endif // TRACKFINDINGCDC_ROOTIFY_CDCVECTOR
#endif // __CINT__

#endif //CDCENTITIESUNPACKEDFROMNAMESPACES_H
