#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma extra_include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>;

#pragma link C++ class Belle2::TrackingUtilities::CDCWireHitSegment+; // checksum=0x3711d49, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCWireHitCluster+; // checksum=0x939ed350, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCRLWireHitSegment+; // checksum=0xe6a48c12, version=-1

#pragma link C++ class Belle2::TrackingUtilities::CDCTangentSegment+; // checksum=0xc1f1d970, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCFacetSegment+; // checksum=0xb8eacc, version=-1

#pragma link C++ class Belle2::TrackingUtilities::CDCSegment2D+; // checksum=0x11c93300, version=-1

#pragma link C++ class Belle2::TrackingUtilities::CDCSegment3D+; // checksum=0x8eea33c, version=-1

#pragma link C++ class Belle2::TrackingUtilities::CDCSegment<Belle2::TrackingUtilities::CDCWireHit>+; // checksum=0xe20d381, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCSegment<const Belle2::TrackingUtilities::CDCWireHit *>+; // checksum=0xbee6eae3, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCSegment<Belle2::TrackingUtilities::CDCRLWireHit>+; // checksum=0xd6ff9c69, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCSegment<Belle2::TrackingUtilities::CDCRecoHit2D>+; // checksum=0x89888151, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCSegment<Belle2::TrackingUtilities::CDCTangent>+; // checksum=0x753fe15, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCSegment<Belle2::TrackingUtilities::CDCFacet>+; // checksum=0xefa7aa95, version=-1
#pragma link C++ class Belle2::TrackingUtilities::CDCSegment<Belle2::TrackingUtilities::CDCRecoHit3D>+; // checksum=0xd8f0c07d, version=-1

// Some dictionaries for vectors and pairs that will be needed
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCWireHitCluster>+; // checksum=0x9b8fcba, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCWireHitSegment>+; // checksum=0x9b9610d, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCRLWireHitSegment>+; // checksum=0x2bbb06c3, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCTangentSegment>+; // checksum=0x81c69d0, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCSegment2D>+; // checksum=0xc5098631, version=6
#pragma link C++ class std::vector<Belle2::TrackingUtilities::CDCSegment3D>+; // checksum=0xc509863a, version=6
#pragma link C++ class std::pair<const Belle2::TrackingUtilities::CDCSegment2D*,const Belle2::TrackingUtilities::CDCSegment2D*>+; // checksum=0xb15c28fa, version=-1
#pragma link C++ class std::pair<const Belle2::TrackingUtilities::CDCSegment2D*,double>+; // checksum=0x570f7ce2, version=-1
#pragma link C++ class std::pair<std::pair<const Belle2::TrackingUtilities::CDCSegment2D*,double>,const Belle2::TrackingUtilities::CDCSegment2D*>+; // checksum=0x2794358c, version=-1

// Some dictionaries for WeightedRelations that will be needed
#pragma extra_include <tracking/trackingUtilities/utilities/Relation.h>;
#pragma extra_include <tracking/trackingUtilities/utilities/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackingUtilities::Relation<const Belle2::TrackingUtilities::CDCSegment2D>+; // checksum=0xa6ea66c7, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackingUtilities::Relation<const Belle2::TrackingUtilities::CDCSegment2D> >+; // checksum=0x1a94e16c, implicit, version=6

#pragma link C++ class Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCSegment2D>+; // checksum=0x16b31787, implicit, version=-1
#pragma link C++ class std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCSegment2D> >+; // checksum=0x8ec68eb5, implicit, version=6

// Define IO types as we want to put them in the DataStore
#pragma extra_include <tracking/trackingUtilities/rootification/StoreWrapper.h>;

#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::CDCWireHitCluster> >+; // checksum=0xa2e5d154, implicit, version=2

#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::CDCSegment2D> >+; // checksum=0xf7f81a2e, implicit, version=2
#pragma link C++ class Belle2::TrackingUtilities::StoreWrapper<std::vector<Belle2::TrackingUtilities::WeightedRelation<const Belle2::TrackingUtilities::CDCSegment2D> > >+; // checksum=0x518eb9e, implicit, version=2
