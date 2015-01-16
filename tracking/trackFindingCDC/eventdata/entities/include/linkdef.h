#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>
//#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHit-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCWireHit-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHit-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoHit2D-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHitPair-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoTangent-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHitTriple-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoFacet-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoHit3D-;

#endif

#endif
