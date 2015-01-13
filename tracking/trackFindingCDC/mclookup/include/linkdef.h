#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TrackFindingCDC::CDCMCHitLookUp+;

#ifdef TRACKFINDINGCDC_USE_ROOT
#pragma link C++ class Belle2::TrackFindingCDC::CDCMCSegmentLookUp+;
#endif

#endif
