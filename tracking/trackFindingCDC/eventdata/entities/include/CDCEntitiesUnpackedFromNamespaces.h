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

#include <tracking/trackFindingCDC/eventdata/entities/CDCEntities.h>

#ifdef __CINT__
// Because ROOTCINT does not like namespaces inside template parameters
// we have to make each object, for which we want the instantiate a template,
// available outside any namespace.

typedef Belle2::TrackFindingCDC::CDCGenHit CDCGenHit;
typedef Belle2::TrackFindingCDC::CDCWireHit CDCWireHit;
typedef Belle2::TrackFindingCDC::CDCRLWireHit CDCRLWireHit;
typedef Belle2::TrackFindingCDC::CDCRecoHit2D CDCRecoHit2D;
typedef Belle2::TrackFindingCDC::CDCTangent CDCTangent;
typedef Belle2::TrackFindingCDC::CDCFacet CDCFacet;
typedef Belle2::TrackFindingCDC::CDCRecoHit3D CDCRecoHit3D;

#endif // __CINT__

#endif //CDCENTITIESUNPACKEDFROMNAMESPACES_H
