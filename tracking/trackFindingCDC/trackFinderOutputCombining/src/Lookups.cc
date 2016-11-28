#include <tracking/trackFindingCDC/trackFinderOutputCombining/Lookups.h>
#include <tracking/trackFindingCDC/trackFinderOutputCombining/MatchingInformation.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void SegmentLookUp::fillWith(std::vector<CDCSegment2D>& segments)
{
  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  // Calculate a lookup SuperLayerID -> Segments
  m_lookup.clear();
  m_lookup.resize(wireTopology.c_NSuperLayers);

  // Calculate a lookup cdcHit-> Segment (we use cdcHits here, not cdcWireHits)
  m_hitSegmentLookUp.clear();

  for (CDCSegment2D& segment : segments) {
    if (segment.getAutomatonCell().hasTakenFlag() or
        segment.getAutomatonCell().hasBackgroundFlag() or
        segment.isFullyTaken()) {
      continue;
    }

    ISuperLayer iSuperLayer = segment.getISuperLayer();
    SegmentInformation* newSegmentInformation = new SegmentInformation(&segment);
    m_lookup[iSuperLayer].push_back(newSegmentInformation);

    for (const CDCRecoHit2D& recoHit : segment) {
      const CDCHit* cdcHit = recoHit.getWireHit().getHit();
      m_hitSegmentLookUp.insert(std::make_pair(cdcHit, newSegmentInformation));
    }

    B2DEBUG(200, "Added new segment to segment lookup: " << segment.getTrajectory2D());
  }
}

void SegmentLookUp::clear()
{
  for (std::vector<SegmentInformation*>& segmentList : m_lookup) {
    for (SegmentInformation* segmentInformation : segmentList) {
      delete segmentInformation;
    }
  }
}

SegmentInformation* SegmentLookUp::findSegmentForHit(const CDCRecoHit3D& recoHit)
{
  const CDCHit* cdcHit = recoHit.getWireHit().getHit();
  auto foundElement = m_hitSegmentLookUp.find(cdcHit);
  if (foundElement == m_hitSegmentLookUp.end()) {
    return nullptr;
  } else {
    return foundElement->second;
  }
}




void TrackLookUp::fillWith(std::vector<CDCTrack>& tracks)
{
  // Calculate a lookup for Tracks
  m_lookup.clear();
  m_lookup.reserve(tracks.size());

  // Calculate a lookup cdcHit-> Track (we use cdcHits here, not cdcwirehits)
  m_hitTrackLookUp.clear();

  // Calculate a lookup Track -> TrackInformation
  for (CDCTrack& trackCand : tracks) {
    TrackInformation* trackInformation = new TrackInformation(&trackCand);

    std::vector<double>& perpSList = trackInformation->getArcLength2DList();
    perpSList.reserve(trackCand.size());
    for (const CDCRecoHit3D& recoHit : trackCand) {
      perpSList.push_back(recoHit.getArcLength2D());
    }
    trackInformation->calcArcLength2D();
    m_lookup.push_back(trackInformation);

    for (const CDCRecoHit3D& recoHit : trackCand) {
      const CDCHit* cdcHit = recoHit.getWireHit().getHit();
      m_hitTrackLookUp.insert(std::make_pair(cdcHit, trackInformation));
    }

    B2DEBUG(200, "Added new track to track lookup: " << trackCand.getStartTrajectory3D().getTrajectory2D());
  }
}


void TrackLookUp::clear()
{
  for (TrackInformation* trackInformation : m_lookup) {
    delete trackInformation;
  }
}


TrackInformation* TrackLookUp::findTrackForHit(const CDCRecoHit2D& recoHit)
{
  const CDCHit* cdcHit = recoHit.getWireHit().getHit();
  auto foundElement = m_hitTrackLookUp.find(cdcHit);
  if (foundElement == m_hitTrackLookUp.end()) {
    return nullptr;
  } else {
    return foundElement->second;
  }
}
