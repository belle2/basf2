#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

// Sentinel to trigger the dictionary generation of the StoreWrapper.
// Other instances can be defined in other linkdef.h files.
#pragma link C++ class Belle2::TrackFindingCDC::Relation<int, int>+;
#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<int, int>+;
