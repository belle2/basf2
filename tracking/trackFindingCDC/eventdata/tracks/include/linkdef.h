#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT

#pragma link C++ class Belle2::CDCLocalTracking::CDCAxialAxialSegmentPair+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCAxialStereoSegmentPair+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCSegmentTriple+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCTrack+;

#endif

#endif
