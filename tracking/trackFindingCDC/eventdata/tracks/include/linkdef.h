
#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

#pragma link C++ class Belle2::TrackFindingCDC::CDCAxialSegmentPair+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegmentPair+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegmentTriple+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCTrack+;

/// Some dictionaries for vectors that will be needed
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCAxialSegmentPair>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCSegmentPair>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCSegmentTriple>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCTrack>+;

#endif

