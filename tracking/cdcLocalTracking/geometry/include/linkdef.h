#ifdef __CINT__

#include <tracking/cdcLocalTracking/mockroot/ToggleMockRoot.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef USE_ROOT_IN_CDCLOCALTRACKING

#pragma link C++ class Belle2::CDCLocalTracking::Vector2D;
#pragma link C++ class Belle2::CDCLocalTracking::Vector3D;

#pragma link C++ class Belle2::CDCLocalTracking::Circle2D;

#pragma link C++ class Belle2::CDCLocalTracking::Line2D;
#pragma link C++ class Belle2::CDCLocalTracking::ParameterLine2D;

#pragma link C++ class Belle2::CDCLocalTracking::GeneralizedCircle;
#pragma link C++ class Belle2::CDCLocalTracking::BoundSkewLine;


#endif

#endif
