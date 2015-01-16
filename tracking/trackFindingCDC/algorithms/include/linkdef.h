#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

#ifdef TRACKFINDINGCDC_USE_ROOT_BASE

#pragma link C++ class Belle2::TrackFindingCDC::AutomatonCell+;

//for testing
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<int>+;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<float>+;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCGenHit>+;

//to use
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCWire>+;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<const CDCWire*>+;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCWireHit>+;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<const CDCWireHit*>+;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCRLWireHit>+;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<const CDCRLWireHit*>+;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCRecoHit2D>+;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCRecoTangent>+;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCRecoFacet>+;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCRecoHit3D>+;

#else

#pragma link C++ class Belle2::TrackFindingCDC::AutomatonCell-;

//for testing
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<int>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<float>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCGenHit>-;

//to use
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCWire>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<const CDCWire*>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCWireHit>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<const CDCWireHit*>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCRLWireHit>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<const CDCRLWireHit*>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCRecoHit2D>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCRecoTangent>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCRecoFacet>-;
#pragma link C++ class Belle2::TrackFindingCDC::SortableVector<CDCRecoHit3D>-;

#endif

#endif

#endif
