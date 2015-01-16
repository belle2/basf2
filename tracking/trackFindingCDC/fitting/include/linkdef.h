#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

#pragma link C++ class Belle2::TrackFindingCDC::CDCObservations2D-;

#pragma link C++ class Belle2::TrackFindingCDC::RiemannsMethod-;
#pragma link C++ class Belle2::TrackFindingCDC::ExtendedRiemannsMethod-;
#pragma link C++ class Belle2::TrackFindingCDC::KarimakisMethod-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCFitter2D<RiemannsMethod>-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCFitter2D<ExtendedRiemannsMethod>-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCFitter2D<KarimakisMethod>-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRiemannFitter-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCKarimakiFitter-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCAxialStereoFusion-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCSZFitter-;

#endif

#endif
