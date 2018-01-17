#include <tracking/trackFindingCDC/mclookup/CDCMCTrackCurlerCloneLookUp.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <framework/logging/Logger.h>

#include <algorithm>

// #include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

using namespace Belle2;
using namespace TrackFindingCDC;


CDCMCTrackCurlerCloneLookUp& CDCMCTrackCurlerCloneLookUp::getInstance()
{
  static CDCMCTrackCurlerCloneLookUp cloneInfo;
  return cloneInfo;
}

void CDCMCTrackCurlerCloneLookUp::fillMCIDToCDCTracksMap(std::vector<CDCTrack>& cdcTracks)
{
  const CDCMCTrackLookUp& cdcMCTrackLookUp = CDCMCTrackLookUp::getInstance();
  for (CDCTrack& cdcTrack : cdcTracks) {
    CDCTrack* ptrCDCTrack = &cdcTrack;
    ITrackType mcTrackID = cdcMCTrackLookUp.getMCTrackId(ptrCDCTrack);

    if (m_mcTrackIDToCDCTracksMap.find(mcTrackID) == m_mcTrackIDToCDCTracksMap.end()) {
      // mcTrackID not yet in map, so add element
      m_mcTrackIDToCDCTracksMap.emplace(mcTrackID, std::vector<CDCTrack*> {ptrCDCTrack});
    } else { // mcTrackID already in map, so add Track to vector of tracks
      m_mcTrackIDToCDCTracksMap[mcTrackID].push_back(ptrCDCTrack);
    }
  }
}

CDCTrack* CDCMCTrackCurlerCloneLookUp::findNonCurlerCloneCDCTrack(std::vector<CDCTrack*> matchedTrackPtrs)
{
  const CDCMCTrackLookUp& cdcMCTrackLookUp = CDCMCTrackLookUp::getInstance();

  /// TODO: this is so ugly, maybe replace by a loops-based approach
  CDCTrack* ptrNonCurlerCloneTrack =
    *(std::min_element(matchedTrackPtrs.begin(), matchedTrackPtrs.end(),
  [&cdcMCTrackLookUp](CDCTrack * ptrTrack1, CDCTrack * ptrTrack2) {
    return cdcMCTrackLookUp.getFirstNLoops(ptrTrack1)
           < cdcMCTrackLookUp.getFirstNLoops(ptrTrack2);
  }));
  return ptrNonCurlerCloneTrack;
}

/// Fill LookUp Table which stores information, which tracks are clones from curlers
void CDCMCTrackCurlerCloneLookUp::fill(std::vector<CDCTrack>& cdcTracks)
{
  /// per default, all tracks are not clones
  for (const CDCTrack& cdcTrack : cdcTracks) {
    const CDCTrack* ptrCDCTrack = &cdcTrack;
    m_cdcTrackIsCurlerCloneMap[ptrCDCTrack] = false; // not clone
  }

  /// fill lookup table of MC track IDs to vectors of CDC track pointers
  fillMCIDToCDCTracksMap(cdcTracks);

  for (auto& mcIDToCDCTracks : m_mcTrackIDToCDCTracksMap) {
    /// Vector of track pointers which are mapped to this mcTrackID
    std::vector<CDCTrack*>& matchedTrackPtrs = mcIDToCDCTracks.second;

    if (matchedTrackPtrs.size() == 1) { // only one matching track
      m_cdcTrackIsCurlerCloneMap[matchedTrackPtrs.at(0)] = false; // not clone

    } else { // multiple matching tracks
      for (const CDCTrack* ptrCDCTrack : matchedTrackPtrs) {
        m_cdcTrackIsCurlerCloneMap[ptrCDCTrack] = true; // is clone
      }
      const CDCTrack* ptrNonCurlerCloneTrack = findNonCurlerCloneCDCTrack(matchedTrackPtrs);
      m_cdcTrackIsCurlerCloneMap[ptrNonCurlerCloneTrack] = false; // not clone
    }
  }
}

bool CDCMCTrackCurlerCloneLookUp::isTrackCurlerClone(const CDCTrack& cdcTrack)
{
  const CDCTrack* ptrCDCTrack = &cdcTrack;

  if (m_cdcTrackIsCurlerCloneMap.find(ptrCDCTrack) == m_cdcTrackIsCurlerCloneMap.end()) {
    B2FATAL("No entry for this CDC track in m_cdcTrackIsCurlerCloneMap");
  }

  return m_cdcTrackIsCurlerCloneMap[ptrCDCTrack];
}
