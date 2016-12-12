#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::TrackFindingCDC::CDCAxialSegmentPair+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegmentPair+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegmentTriple+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCTrack+;

/// Some dictionaries for vectors that will be needed
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCAxialSegmentPair>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCSegmentPair>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCSegmentTriple>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCTrack>+;

// Some dictionaries for WeightedRelations that will be needed
#pragma extra_include <tracking/trackFindingCDC/ca/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentPair>+; // implicit
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentPair> >+; // implicit

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentTriple>+; // implicit
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentTriple> >+; // implicit

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCTrack>+; // implicit
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCTrack> >+; // implicit

// Define IO types as we want to put them in the DataStore
#pragma extra_include <tracking/trackFindingCDC/rootification/StoreWrapper.h>;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCSegmentPair> >+; // implicit
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentPair> > >+; // implicit

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCAxialSegmentPair> >+; // implicit

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCSegmentTriple> >+; // implicit
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentTriple> > >+; // implicit

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCTrack> >+; // implicit
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCTrack> > >+; // implicit
