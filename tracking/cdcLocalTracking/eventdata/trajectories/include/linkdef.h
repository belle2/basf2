#ifdef __CINT__

#include <tracking/cdcLocalTracking/mockroot/ToggleMockRoot.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef CDCLOCALTRACKING_USE_ROOT

#pragma link C++ class Belle2::CDCLocalTracking::CDCTrajectory2D+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCTrajectorySZ+;

#pragma link C++ class Belle2::CDCLocalTracking::CDCTrajectory3D+;

#endif

#endif
