#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT

#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<CDCGenHit>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<CDCWireHit>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<const CDCWireHit *>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<CDCRLWireHit>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<const CDCRLWireHit *>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<CDCRecoHit2D>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<CDCRecoTangent>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<CDCRecoFacet>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHitVector<CDCRecoHit3D>+;

#pragma link C++ class Belle2::TrackFindingCDC::CDCWireHitVector+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoFacetVector+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoTangentVector+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoHit2DVector+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoHit3DVector+;

#endif

#endif
