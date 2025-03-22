#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma extra_include <tracking/trackingUtilities/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackingUtilities::CDCWireHit+; // checksum=0x567f0541, version=-1

#pragma link C++ class Belle2::TrackingUtilities::CDCRecoHit3D+; // checksum=0xad1a22b2, version=-1

// Some dictionaries for vectors and pairs that will be needed
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCRecoHit3D>+; // checksum=0x72a8a078, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCWireHit>+; // checksum=0x7e84c4b7, version=6
#pragma link C++ class std::vector<const Belle2::TrackingUtilities::CDCWireHit*>+; // checksum=0xbbf2d42a, version=6
#pragma link C++ class std::pair<Belle2::TrackingUtilities::CDCWireHit*,double>+; // checksum=0x6087ff06, version=-1
#pragma link C++ class std::pair<std::pair<Belle2::TrackingUtilities::CDCWireHit*,double>,Belle2::TrackingUtilities::CDCWireHit*>+; // checksum=0x2e8c290a, version=-1

// Some dictionaries for WeightedRelations that will be needed
#pragma extra_include <tracking/trackingUtilities/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackingUtilities::WeightedRelation<Belle2::TrackingUtilities::CDCWireHit>+; // checksum=0xddda14ec, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackingUtilities::WeightedRelation<Belle2::TrackingUtilities::CDCWireHit> >+; // checksum=0x71b4cc26, implicit, version=6

// Define IO types as we want to put them in the DataStore
#pragma extra_include <tracking/trackingUtilities/rootification/StoreWrapper.h>;

#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::CDCWireHit> >+; // checksum=0xc649a20d, implicit, version=1
#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::WeightedRelation<Belle2::TrackingUtilities::CDCWireHit> > >+; // checksum=0x10915441, implicit, version=1
