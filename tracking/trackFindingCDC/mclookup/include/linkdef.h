
#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TrackFindingCDC::CDCMCHitLookUp-;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

#pragma link C++ class Belle2::TrackFindingCDC::CDCMCHitCollectionLookUp<Belle2::TrackFindingCDC::CDCRecoSegment2D>-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCMCSegmentLookUp-;

#pragma link C++ class Belle2::TrackFindingCDC::CDCMCHitCollectionLookUp<Belle2::TrackFindingCDC::CDCTrack>-;
#pragma link C++ class Belle2::TrackFindingCDC::CDCMCTrackLookUp-;

#endif

