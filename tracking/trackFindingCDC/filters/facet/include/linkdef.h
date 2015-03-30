#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

//#pragma link C++ class Belle2::TrackFindingCDC::BaseFacetFilter-;

#pragma link C++ class Belle2::TrackFindingCDC::MCFacetFilter-;

#pragma link C++ class Belle2::TrackFindingCDC::AllFacetFilter-;
#pragma link C++ class Belle2::TrackFindingCDC::FitlessFacetFilter-;
#pragma link C++ class Belle2::TrackFindingCDC::SimpleFacetFilter-;
#pragma link C++ class Belle2::TrackFindingCDC::RealisticFacetFilter-;

#endif

#endif
