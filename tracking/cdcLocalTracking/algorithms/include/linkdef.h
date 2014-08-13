#ifdef __CINT__

#include <tracking/cdcLocalTracking/mockroot/ToggleMockRoot.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef CDCLOCALTRACKING_USE_ROOT

#pragma link C++ class Belle2::CDCLocalTracking::AutomatonCell+;

//for testing
#pragma link C++ class Belle2::CDCLocalTracking::SortableVector<int>+;
#pragma link C++ class Belle2::CDCLocalTracking::SortableVector<float>+;
#pragma link C++ class Belle2::CDCLocalTracking::SortableVector<CDCGenHit>+;

//to use
#pragma link C++ class Belle2::CDCLocalTracking::SortableVector<CDCWire>+;
#pragma link C++ class Belle2::CDCLocalTracking::SortableVector<const CDCWire*>+;
#pragma link C++ class Belle2::CDCLocalTracking::SortableVector<CDCWireHit>+;
#pragma link C++ class Belle2::CDCLocalTracking::SortableVector<const CDCWireHit *>+;
#pragma link C++ class Belle2::CDCLocalTracking::SortableVector<CDCRLWireHit>+;
#pragma link C++ class Belle2::CDCLocalTracking::SortableVector<CDCRecoHit2D>+;
#pragma link C++ class Belle2::CDCLocalTracking::SortableVector<CDCRecoTangent>+;
#pragma link C++ class Belle2::CDCLocalTracking::SortableVector<CDCRecoFacet>+;
#pragma link C++ class Belle2::CDCLocalTracking::SortableVector<CDCRecoHit3D>+;

#endif

#endif
