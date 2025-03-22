#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TrackingUtilities::CDCTrack+; // checksum=0x35a5e7f6, version=-1

/// Some dictionaries for vectors and pairs that will be needed
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCTrack>+; // checksum=0x2a8085d6, version=6
#pragma link C++ class std::pair<const Belle2::TrackingUtilities::CDCTrack*,double>+; // checksum=0xf1a06876, version=-1
#pragma link C++ class std::pair<std::pair<const Belle2::TrackingUtilities::CDCTrack*,double>,const Belle2::TrackingUtilities::CDCTrack*>+; // checksum=0x9a92a13a, version=-1

// Some dictionaries for WeightedRelations that will be needed
#pragma extra_include <tracking/trackingUtilities/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCTrack>+; // checksum=0xd439af3c, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCTrack> >+; // checksum=0xb8efa276, implicit, version=6

// Define IO types as we want to put them in the DataStore
#pragma extra_include <tracking/trackingUtilities/rootification/StoreWrapper.h>;

#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::CDCTrack> >+; // checksum=0x8464cf79, implicit, version=1
#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCTrack> > >+; // checksum=0x94258fa1, implicit, version=1
