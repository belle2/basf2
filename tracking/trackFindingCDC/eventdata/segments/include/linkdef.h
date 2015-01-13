#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT

#pragma link C++ class Belle2::TrackFindingCDC::CDCWireHitCluster+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCWireHitSegment+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHitSegment+;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoTangentSegment+;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoSegment2D+;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoSegment3D+;

#endif

#endif
