#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma extra_include <tracking/trackFindingCDC/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackFindingCDC::CDCWireHit+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHit+;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoHit2D+;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHitPair+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCTangent+;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHitTriple+;
#pragma link C++ class Belle2::TrackFindingCDC::CDCFacet+;

#pragma link C++ class Belle2::TrackFindingCDC::CDCRecoHit3D+;

// Some dictionaries for vectors that will be needed
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCWireHit>+;
#pragma link C++ class std::vector<const Belle2::TrackFindingCDC::CDCWireHit*>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCRLWireHit>+;
#pragma link C++ class std::vector<const Belle2::TrackFindingCDC::CDCRLWireHit*>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCRecoHit2D>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCTangent>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCFacet>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCRecoHit3D>+;

// Some dictionaries for WeightedRelations that will be needed
#pragma extra_include <tracking/trackFindingCDC/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<Belle2::TrackFindingCDC::CDCWireHit>+; // implicit
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<Belle2::TrackFindingCDC::CDCWireHit> >+; // implicit

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCFacet>+; // implicit
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCFacet> >+; // implicit

// Define IO types as we want to put them in the DataStore
#pragma extra_include <tracking/trackFindingCDC/rootification/StoreWrapper.h>;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCWireHit> >+; // implicit
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<Belle2::TrackFindingCDC::CDCWireHit> > >+; // implicit

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCFacet> >+; // implicit
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCFacet> > >+; // implicit
