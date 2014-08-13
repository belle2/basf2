#ifdef __CINT__

#include <tracking/cdcLocalTracking/mockroot/ToggleMockRoot.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef CDCLOCALTRACKING_USE_ROOT

#pragma link C++ class Belle2::CDCLocalTracking::CDCWireHitCluster+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCWireHitSegment+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRLWireHitSegment+;

#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoTangentSegment+;

#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoSegment2D+;

#endif

#endif
