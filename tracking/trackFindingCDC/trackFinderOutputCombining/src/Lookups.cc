#include <tracking/trackFindingCDC/trackFinderOutputCombining/Lookups.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void SegmentLookUp::fillWith(std::vector<CDCRecoSegment2D>& segments)
{
  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  // Calculate a lookup SuperLayerID -> Segments
  m_lookup.clear();
  m_lookup.resize(wireTopology.N_SUPERLAYERS);

  // Calculate a lookup cdcHit-> Segment (we use cdcHits here, not cdcWireHits)
  m_hitSegmentLookUp.clear();

  for (CDCRecoSegment2D& segment : segments) {
    if (segment.getAutomatonCell().hasTakenFlag())
      continue;

    ILayerType superlayerID = segment.getISuperLayer();
    SegmentInformation* newSegmentInformation = new SegmentInformation(&segment);
    m_lookup[superlayerID].push_back(newSegmentInformation);

    for (const CDCRecoHit2D& recoHit : segment) {
      const CDCHit* cdcHit = recoHit.getWireHit().getHit();
      m_hitSegmentLookUp.insert(std::make_pair(cdcHit, newSegmentInformation));
    }

    B2DEBUG(200, "Added new segment to segment lookup: " << segment.getTrajectory2D())
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

    std::vector<double>& perpSList = trackInformation->getPerpSList();
    perpSList.reserve(trackCand.size());
    for (const CDCRecoHit3D& recoHit : trackCand) {
      perpSList.push_back(recoHit.getPerpS());
    }
    trackInformation->calcPerpS();
    m_lookup.push_back(trackInformation);

    for (const CDCRecoHit3D& recoHit : trackCand) {
      const CDCHit* cdcHit = recoHit.getWireHit().getHit();
      m_hitTrackLookUp.insert(std::make_pair(cdcHit, trackInformation));
    }

    B2DEBUG(200, "Added new track to track lookup: " << trackCand.getStartTrajectory3D().getTrajectory2D())
  }
}
