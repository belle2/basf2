#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TrackFindingCDC::CDCAxialSegmentPair+; // checksum=0x13923a15, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegmentPair+; // checksum=0x28533850, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegmentTriple+; // checksum=0x8ef36f8a, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCTrack+; // checksum=0xc5454a6e, version=-1

/// Some dictionaries for vectors and pairs that will be needed
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCAxialSegmentPair>+; // checksum=0x6c87abbb, version=6
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCSegmentPair>+; // checksum=0x7f26dcc, version=6
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCSegmentTriple>+; // checksum=0x4785fa88, version=6
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCTrack>+; // checksum=0x2a8085d6, version=6
#pragma link C++ class std::pair<const Belle2::TrackFindingCDC::CDCSegmentPair*,double>+; // checksum=0x5b8abcf6, version=-1
#pragma link C++ class std::pair<std::pair<const Belle2::TrackFindingCDC::CDCSegmentPair*,double>,const Belle2::TrackFindingCDC::CDCSegmentPair*>+; // checksum=0x64d5b89e, version=-1
#pragma link C++ class std::pair<const Belle2::TrackFindingCDC::CDCSegmentTriple*,double>+; // checksum=0x2951577e, version=-1
#pragma link C++ class std::pair<std::pair<const Belle2::TrackFindingCDC::CDCSegmentTriple*,double>,const Belle2::TrackFindingCDC::CDCSegmentTriple*>+; // checksum=0x783b87a6, version=-1
#pragma link C++ class std::pair<const Belle2::TrackFindingCDC::CDCTrack*,double>+; // checksum=0xf1a06876, version=-1
#pragma link C++ class std::pair<std::pair<const Belle2::TrackFindingCDC::CDCTrack*,double>,const Belle2::TrackFindingCDC::CDCTrack*>+; // checksum=0x9a92a13a, version=-1

// Some dictionaries for WeightedRelations that will be needed
#pragma extra_include <tracking/trackFindingCDC/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentPair>+; // checksum=0x859010b4, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentPair> >+; // checksum=0x1aea98f2, implicit, version=6

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentTriple>+; // checksum=0x663600fc, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentTriple> >+; // checksum=0xccd40efa, implicit, version=6

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCTrack>+; // checksum=0xd439af3c, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCTrack> >+; // checksum=0xb8efa276, implicit, version=6

// Define IO types as we want to put them in the DataStore
#pragma extra_include <tracking/trackFindingCDC/rootification/StoreWrapper.h>;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCSegmentPair> >+; // checksum=0xdb8b0039, implicit, version=1
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentPair> > >+; // checksum=0x3944e5a1, implicit, version=1

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCAxialSegmentPair> >+; // checksum=0x11c9ff25, implicit, version=1

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCSegmentTriple> >+; // checksum=0x8abb5901, implicit, version=1
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentTriple> > >+; // checksum=0x6620ef91, implicit, version=1

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCTrack> >+; // checksum=0x8464cf79, implicit, version=1
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCTrack> > >+; // checksum=0x94258fa1, implicit, version=1
