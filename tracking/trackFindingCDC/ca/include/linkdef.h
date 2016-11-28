
#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

#pragma link C++ class Belle2::TrackFindingCDC::AutomatonCell+;

#pragma extra_include <tracking/trackFindingCDC/ca/WeightedRelation.h>;

/// Some dictionaries for weighted relations that will be needed
#pragma extra_include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>;
#pragma extra_include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>;
#pragma extra_include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>;
#pragma extra_include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>;
#pragma extra_include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>;
#pragma extra_include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>;

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<Belle2::TrackFindingCDC::CDCWireHit>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<Belle2::TrackFindingCDC::CDCWireHit> >+;

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCFacet>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCFacet> >+;

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegment2D>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegment2D> >+;

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentPair>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentPair> >+;

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentTriple>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentTriple> >+;

#pragma link C++ class Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCTrack>+;
#pragma link C++ class std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCTrack> >+;

#endif

