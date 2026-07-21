#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

// Some dictionaries for pairs that will be needed
#pragma link C++ class std::pair<int*,int*>+; // checksum=0x3efb1de, version=-1
#pragma link C++ class std::pair<int*,double>+; // checksum=0x77d90b4a, version=-1
#pragma link C++ class std::pair<std::pair<int*,double>,int*>+; // checksum=0x70ffb7b0, version=-1

// Sentinel to trigger the dictionary generation of the StoreWrapper.
// Other instances can be defined in other linkdef.h files.
#pragma link C++ class Belle2::TrackingUtilities::Relation<int, int>+; // checksum=0x9641adef, version=-1
#pragma link C++ class Belle2::TrackingUtilities::WeightedRelation<int, int>+; // checksum=0x7dfe1a7f, version=-1
