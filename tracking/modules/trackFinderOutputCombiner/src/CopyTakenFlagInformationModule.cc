#include <tracking/modules/trackFinderOutputCombiner/CopyTakenFlagInformationModule.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace std;

REG_MODULE(CopyTakenFlagInformation)

void CopyTakenFlagInformationModule::event()
{
  StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> > storedRecoSegments(m_param_segmentsStoreObjName);
  if (not storedRecoSegments) {
    B2WARNING("The segments have not been created.");
    return;
  }

  std::vector<CDCRecoSegment2D>& segments = *storedRecoSegments;

  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  // List of all wire hits in the segments
  std::vector<const CDCWireHit*> wireHitsInSegments;
  wireHitsInSegments.resize(segments.size() * 20); // Assuming ~ 20 hits per segments
  for (const CDCRecoSegment2D& segment : segments) {
    for (const CDCRecoHit2D& recoHit2D : segment) {
      wireHitsInSegments.push_back(&(recoHit2D.getWireHit()));
    }
  }

  for (const CDCWireHit& wireHit : wireHitTopology.getWireHits()) {
    if (std::find(wireHitsInSegments.begin(), wireHitsInSegments.end(), &wireHit) == wireHitsInSegments.end()) {
      wireHit.getAutomatonCell().setTakenFlag();
    } else {
      wireHit.getAutomatonCell().unsetTakenFlag();
    }
  }
}
