#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TrackFindingCDC::RootificationBase+;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<Belle2::TrackFindingCDC::CDCWireHit>+;
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<Belle2::TrackFindingCDC::SortableVector<Belle2::TrackFindingCDC::CDCWireHit>>+;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCWireHit>>+;
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>>+;

#endif
