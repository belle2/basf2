#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>
//#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

#pragma link C++ class Belle2::TrackFindingCDC::CDCGenHit-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCDerivedGenHit-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCDerivedGenHit::Impl-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCWireHit-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHit-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRLTaggedWireHit-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoHit2D-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHitPair-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCTangent-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHitTriple-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCFacet-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoHit3D-;

/// Some dictionaries for vectors that will be needed
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCWireHit>-;
#pragma link C++ class std::vector<const Belle2::TrackFindingCDC::CDCWireHit*>-;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCRLWireHit>-;
#pragma link C++ class std::vector<const Belle2::TrackFindingCDC::CDCRLWireHit*>-;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCRecoHit2D>-;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCTangent>-;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCFacet>-;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCRecoHit3D>-;

#endif

#endif
