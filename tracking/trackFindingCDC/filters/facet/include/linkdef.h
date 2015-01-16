#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

#ifdef TRACKFINDINGCDC_USE_ROOT_BASE

#pragma link C++ class Belle2::TrackFindingCDC::MCFacetFilter+;

#pragma link C++ class Belle2::TrackFindingCDC::AllFacetFilter+;
#pragma link C++ class Belle2::TrackFindingCDC::SimpleFacetFilter+;

#else

#pragma link C++ class Belle2::TrackFindingCDC::MCFacetFilter-;

#pragma link C++ class Belle2::TrackFindingCDC::AllFacetFilter-;
#pragma link C++ class Belle2::TrackFindingCDC::SimpleFacetFilter-;

#endif

#endif

#endif
