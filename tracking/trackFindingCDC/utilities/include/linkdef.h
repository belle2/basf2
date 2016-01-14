#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

/// Some dictionaries for weighted relations that will be needed
#pragma extra_include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>;

#pragma link C++ class Belle2::TrackFindingCDC::VectorRange<Belle2::TrackFindingCDC::CDCWireHit>-;

#endif
