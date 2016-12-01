
#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

#pragma link C++ class Belle2::TrackFindingCDC::CDCWireHitCluster+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCWireHitSegment+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHitSegment+;

#pragma link C++ class Belle2::TrackFindingCDC::CDCTangentSegment+;

#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment2D+;

#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment3D+;

#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<Belle2::TrackFindingCDC::CDCWireHit>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<const Belle2::TrackFindingCDC::CDCWireHit *>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<Belle2::TrackFindingCDC::CDCRLWireHit>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<Belle2::TrackFindingCDC::CDCRecoHit2D>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<Belle2::TrackFindingCDC::CDCTangent>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<Belle2::TrackFindingCDC::CDCFacet>+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<Belle2::TrackFindingCDC::CDCRecoHit3D>+;

/// Some dictionaries for vectors that will be needed
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCWireHitCluster>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCWireHitSegment>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCRLWireHitSegment>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCTangentSegment>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCSegment2D>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCSegment3D>+;

#endif

