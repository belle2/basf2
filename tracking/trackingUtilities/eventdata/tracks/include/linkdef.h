#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TrackingUtilities::CDCTrack+; // checksum=0xf73bc37f, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCAxialSegmentPair+; // checksum=0xf3a091fa, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCSegmentPair+; // checksum=0xfa0288d1, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCSegmentTriple+; // checksum=0x217847d0, version=-1

/// Some dictionaries for vectors and pairs that will be needed
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCTrack>+; // checksum=0xc00fef4f, version=6
#pragma link C++ class std::pair<const Belle2::TrackingUtilities::CDCTrack*,double>+; // checksum=0x7ffab7aa, version=-1
#pragma link C++ class std::pair<std::pair<const Belle2::TrackingUtilities::CDCTrack*,double>,const Belle2::TrackingUtilities::CDCTrack*>+; // checksum=0x1e7b2b80, version=-1

#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCAxialSegmentPair>+; // checksum=0x29d5b05e, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCSegmentPair>+; // checksum=0xed55c75d, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCSegmentTriple>+; // checksum=0x580420a1, version=6
#pragma link C++ class std::pair<const Belle2::TrackingUtilities::CDCSegmentPair*,double>+; // checksum=0x981822e2, version=-1
#pragma link C++ class std::pair<std::pair<const Belle2::TrackingUtilities::CDCSegmentPair*,double>,const Belle2::TrackingUtilities::CDCSegmentPair*>+; // checksum=0xa50ae544, version=-1
#pragma link C++ class std::pair<const Belle2::TrackingUtilities::CDCSegmentTriple*,double>+; // checksum=0xf95f86c2, version=-1
#pragma link C++ class std::pair<std::pair<const Belle2::TrackingUtilities::CDCSegmentTriple*,double>,const Belle2::TrackingUtilities::CDCSegmentTriple*>+; // checksum=0xd982cf8c, version=-1

// Some dictionaries for WeightedRelations that will be needed
#pragma extra_include <tracking/trackingUtilities/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCTrack>+; // checksum=0xb634e957, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCTrack> >+; // checksum=0x9c90b7b1, implicit, version=6

#pragma link C++ class Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCSegmentPair>+; // checksum=0x3117774f, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCSegmentPair> >+; // checksum=0x3f05d715, implicit, version=6

#pragma link C++ class Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCSegmentTriple>+; // checksum=0x65321457, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCSegmentTriple> >+; // checksum=0xc7edb865, implicit, version=6


// Define IO types as we want to put them in the DataStore
#pragma extra_include <tracking/trackingUtilities/rootification/StoreWrapper.h>;

#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::CDCTrack> >+; // checksum=0x737b1e56, implicit, version=2
#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCTrack> > >+; // checksum=0x4972f5ce, implicit, version=2

#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::CDCSegmentPair> >+; // checksum=0x4d650be, implicit, version=2
#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCSegmentPair> > >+; // checksum=0x9f662a9e, implicit, version=2

#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::CDCAxialSegmentPair> >+; // checksum=0x1af71e84, implicit, version=2

#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::CDCSegmentTriple> >+; // checksum=0x9716f22e, implicit, version=2
#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCSegmentTriple> > >+; // checksum=0xee6be85e, implicit, version=2
