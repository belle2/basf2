#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

//for testing
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<int>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<float>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<Belle2::TrackFindingCDC::CDCGenHit>-;

//to use
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<Belle2::TrackFindingCDC::CDCWire>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<const Belle2::TrackFindingCDC::CDCWire*>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<Belle2::TrackFindingCDC::CDCWireHit>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<const Belle2::TrackFindingCDC::CDCWireHit*>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<Belle2::TrackFindingCDC::CDCRLWireHit>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<const Belle2::TrackFindingCDC::CDCRLWireHit*>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<Belle2::TrackFindingCDC::CDCRecoHit2D>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<Belle2::TrackFindingCDC::CDCTangent>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<Belle2::TrackFindingCDC::CDCFacet>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<Belle2::TrackFindingCDC::CDCRecoHit3D>-;

#endif

#endif
