#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma extra_include <tracking/trackingUtilities/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackingUtilities::CDCWireHit+; // checksum=0xa11ee4bb, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCRLWireHit+; // checksum=0xd30b83b9, version=-1

#pragma link C++ class Belle2::TrackingUtilities::CDCRecoHit2D+; // checksum=0x9b12c4d, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCRecoHit3D+; // checksum=0x1c170e75, version=-1

#pragma link C++ class Belle2::TrackingUtilities::CDCRLWireHitPair+; // checksum=0xe371b5e6, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCTangent+; // checksum=0x85c0a307, version=-1

#pragma link C++ class Belle2::TrackingUtilities::CDCRLWireHitTriple+; // checksum=0xdc5a19e1, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCFacet+; // checksum=0x29d6de27, version=-1

// Some dictionaries for vectors and pairs that will be needed
#pragma link C++ class std::vector<const Belle2::TrackingUtilities::CDCWireHit*>+; // checksum=0x85372405, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCWireHit>+; // checksum=0xc08f79f8, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCRLWireHit>+; // checksum=0xc5062bda, version=6
#pragma link C++ class std::vector<const Belle2::TrackingUtilities::CDCRLWireHit*>+; // checksum=0xaee0ea83, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCRecoHit2D>+; // checksum=0xc508ffb8, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCTangent>+; // checksum=0xc08f49a1, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCFacet>+; // checksum=0xc00fdd07, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCRecoHit3D>+; // checksum=0xc508ffc1, version=6
#pragma link C++ class std::pair<Belle2::TrackingUtilities::CDCWireHit*,double>+; // checksum=0xf4e0290a, version=-1
#pragma link C++ class std::pair<std::pair<Belle2::TrackingUtilities::CDCWireHit*,double>,Belle2::TrackingUtilities::CDCWireHit*>+; // checksum=0x993fbab0, version=-1
#pragma link C++ class std::pair<const Belle2::TrackingUtilities::CDCFacet*,double>+; // checksum=0xb680c44a, version=-1
#pragma link C++ class std::pair<std::pair<const Belle2::TrackingUtilities::CDCFacet*,double>,const Belle2::TrackingUtilities::CDCFacet*>+; // checksum=0x59a3e050, version=-1


// Some dictionaries for WeightedRelations that will be needed
#pragma extra_include <tracking/trackingUtilities/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackingUtilities::WeightedRelation<Belle2::TrackingUtilities::CDCWireHit>+; // checksum=0x85952c87, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackingUtilities::WeightedRelation<Belle2::TrackingUtilities::CDCWireHit> >+; // checksum=0x7c6cc4b1, implicit, version=6

#pragma link C++ class Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCFacet>+; // checksum=0xdcd5efd7, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCFacet> >+; // checksum=0x5a3f161, implicit, version=6

// Define IO types as we want to put them in the DataStore
#pragma extra_include <tracking/trackingUtilities/rootification/StoreWrapper.h>;

#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::CDCWireHit> >+; // checksum=0x104f720a, implicit, version=2
#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::WeightedRelation<Belle2::TrackingUtilities::CDCWireHit> > >+; // checksum=0xfec6f44e, implicit, version=2

#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::CDCFacet> >+; // checksum=0x623b8f6, implicit, version=2
#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCFacet> > >+; // checksum=0x6a386d8e, implicit, version=2
