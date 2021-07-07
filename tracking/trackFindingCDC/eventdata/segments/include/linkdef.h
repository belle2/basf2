#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma extra_include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>;

#pragma link C++ class Belle2::TrackFindingCDC::CDCWireHitSegment+; // checksum=0xf983eaa4, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCWireHitCluster+; // checksum=0xb0d6efd4, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCRLWireHitSegment+; // checksum=0x37814af1, version=-1

#pragma link C++ class Belle2::TrackFindingCDC::CDCTangentSegment+; // checksum=0xdc2ee87f, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCFacetSegment+; // checksum=0x6aadde33, version=-1

#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment2D+; // checksum=0x33b507a, version=-1

#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment3D+; // checksum=0x58d48d8d, version=-1

#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<Belle2::TrackFindingCDC::CDCWireHit>+; // checksum=0x2c23b551, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<const Belle2::TrackFindingCDC::CDCWireHit *>+; // checksum=0xf77f3101, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<Belle2::TrackFindingCDC::CDCRLWireHit>+; // checksum=0xc51a8461, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<Belle2::TrackFindingCDC::CDCRecoHit2D>+; // checksum=0x399c5239, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<Belle2::TrackFindingCDC::CDCTangent>+; // checksum=0xa1d46fad, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<Belle2::TrackFindingCDC::CDCFacet>+; // checksum=0x5b641185, version=-1
#pragma link C++ class Belle2::TrackFindingCDC::CDCSegment<Belle2::TrackFindingCDC::CDCRecoHit3D>+; // checksum=0x871014ad, version=-1

// Some dictionaries for vectors and pairs that will be needed
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCWireHitCluster>+; // checksum=0xd83e8a6f, version=6
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCWireHitSegment>+; // checksum=0xd83eeec2, version=6
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCRLWireHitSegment>+; // checksum=0x6e6d0220, version=6
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCTangentSegment>+; // checksum=0xd6a1f785, version=6
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCSegment2D>+; // checksum=0x72a926e8, version=6
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::CDCSegment3D>+; // checksum=0x72a926f1, version=6
#pragma link C++ class std::pair<const Belle2::TrackFindingCDC::CDCSegment2D*,const Belle2::TrackFindingCDC::CDCSegment2D*>+; // checksum=0xb2b0c8ac, version=-1
#pragma link C++ class std::pair<const Belle2::TrackFindingCDC::CDCSegment2D*,double>+; // checksum=0xb5a776ce, version=-1
#pragma link C++ class std::pair<std::pair<const Belle2::TrackFindingCDC::CDCSegment2D*,double>,const Belle2::TrackFindingCDC::CDCSegment2D*>+; // checksum=0x776e50a6, version=-1

// Some dictionaries for WeightedRelations that will be needed
#pragma extra_include <tracking/trackFindingCDC/utilities/Relation.h>;
#pragma extra_include <tracking/trackFindingCDC/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackFindingCDC::Relation<const Belle2::TrackFindingCDC::CDCSegment2D>+; // checksum=0x8c4555e2, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::Relation<const Belle2::TrackFindingCDC::CDCSegment2D> >+; // checksum=0xe5c13901, implicit, version=6

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegment2D>+; // checksum=0x88751cac, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegment2D> >+; // checksum=0xe50b62da, implicit, version=6

// Define IO types as we want to put them in the DataStore
#pragma extra_include <tracking/trackFindingCDC/rootification/StoreWrapper.h>;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCWireHitCluster> >+; // checksum=0x2382cd6d, implicit, version=1

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCSegment2D> >+; // checksum=0x98bfbfd1, implicit, version=1
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegment2D> > >+; // checksum=0x8909f6f1, implicit, version=1
