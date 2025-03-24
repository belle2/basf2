#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma extra_include <tracking/trackingUtilities/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHitPair+; // checksum=0xe3059323, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCTangent+; // checksum=0x805ed0cf, version=-1

#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHitTriple+; // checksum=0xee3a349a, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCFacet+; // checksum=0x205d9864, version=-1

// Some dictionaries for vectors and pairs that will be needed
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCTangent>+; // checksum=0x7e849460, version=6
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCFacet>+; // checksum=0x2a80738e, version=6
#pragma link C++ class std::pair<const Belle2::TrackFindingCDC::CDCFacet*,double>+; // checksum=0x1e93ebd6, version=-1
#pragma link C++ class std::pair<std::pair<const Belle2::TrackFindingCDC::CDCFacet*,double>,const Belle2::TrackFindingCDC::CDCFacet*>+; // checksum=0xd0b61c8a, version=-1

// Some dictionaries for WeightedRelations that will be needed
#pragma extra_include <tracking/trackingUtilities/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackFindingCDC::CDCFacet>+; // checksum=0xf098c6bc, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackFindingCDC::CDCFacet> >+; // checksum=0x6f468866, implicit, version=6

// Define IO types as we want to put them in the DataStore
#pragma extra_include <tracking/trackingUtilities/rootification/StoreWrapper.h>;

#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCFacet> >+; // checksum=0xcd93ecd9, implicit, version=1
#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackFindingCDC::CDCFacet> > >+; // checksum=0x51ba98e1, implicit, version=1
