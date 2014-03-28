#ifdef __CINT__

#include <tracking/cdcLocalTracking/mockroot/ToggleMockRoot.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef CDCLOCALTRACKING_USE_ROOT

#pragma link C++ class Belle2::CDCLocalTracking::CDCGenHitVector<CDCGenHit>+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCGenHitVector<CDCWireHit>+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCGenHitVector<const CDCWireHit *>+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCGenHitVector<CDCRecoHit2D>+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCGenHitVector<CDCRecoTangent>+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCGenHitVector<CDCRecoFacet>+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCGenHitVector<CDCRecoHit3D>+;

//#pragma link C++ class Belle2::CDCLocalTracking::CDCWireHitPtrSet+;
//#pragma link C++ class Belle2::CDCLocalTracking::CDCWireHitSet+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCWireHitVector+;

//#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoFacetSet+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoFacetVector+;

//#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoTangentSet+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoTangentVector+;

//#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoHit2DSet+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoHit2DVector+;

//#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoHit3DSet+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoHit3DVector+;

#endif

#endif
