#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<Belle2::TrackFindingCDC::CDCGenHit>-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<Belle2::TrackFindingCDC::CDCWireHit>-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<const Belle2::TrackFindingCDC::CDCWireHit *>-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<Belle2::TrackFindingCDC::CDCRLWireHit>-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<const Belle2::TrackFindingCDC::CDCRLWireHit *>-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<Belle2::TrackFindingCDC::CDCRecoHit2D>-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<Belle2::TrackFindingCDC::CDCTangent>-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<Belle2::TrackFindingCDC::CDCFacet>-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<Belle2::TrackFindingCDC::CDCRecoHit3D>-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCWireHitVector-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCFacetVector-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCTangentVector-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoHit2DVector-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoHit3DVector-;

#endif

#endif
