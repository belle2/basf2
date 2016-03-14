#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT

#pragma link C++ class Belle2::CDCLocalTracking::CDCWire+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCWireLayer+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCWireSuperLayer+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCWireTopology+;

#endif

#endif
