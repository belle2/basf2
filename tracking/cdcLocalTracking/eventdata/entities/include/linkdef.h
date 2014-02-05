#ifdef __CINT__

#include <tracking/cdcLocalTracking/mockroot/ToggleMockRoot.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCWireHit.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef CDCLOCALTRACKING_USE_ROOT

#pragma link C++ class Belle2::CDCLocalTracking::CDCGenHit;

#pragma link C++ class Belle2::CDCLocalTracking::CDCWireHit;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRLWireHit;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoHit2D;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoTangent;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoFacet;

#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoHit3D;

#endif

#endif
