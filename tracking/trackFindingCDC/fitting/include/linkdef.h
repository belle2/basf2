#ifdef __CINT__

#include <tracking/cdcLocalTracking/config/CDCLocalTrackingConfig.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef CDCLOCALTRACKING_USE_ROOT

#pragma link C++ class Belle2::CDCLocalTracking::CDCObservations2D+;

#pragma link C++ class Belle2::CDCLocalTracking::RiemannsMethod+;
#pragma link C++ class Belle2::CDCLocalTracking::ExtendedRiemannsMethod+;
#pragma link C++ class Belle2::CDCLocalTracking::KarimakisMethod+;

#pragma link C++ class Belle2::CDCLocalTracking::CDCFitter2D<RiemannsMethod>+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCFitter2D<ExtendedRiemannsMethod>+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCFitter2D<KarimakisMethod>+;

#pragma link C++ class Belle2::CDCLocalTracking::CDCRiemannFitter+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCKarimakiFitter+;

#pragma link C++ class Belle2::CDCLocalTracking::CDCAxialStereoFusion+;

#pragma link C++ class Belle2::CDCLocalTracking::CDCSZFitter+;

#endif

#endif
