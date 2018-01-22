#include <tracking/trackFindingCDC/mclookup/CDCMCCurlerCloneLookUp.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

// #include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

using namespace Belle2;
using namespace TrackFindingCDC;


CDCMCCurlerCloneLookUp& CDCMCCurlerCloneLookUp::getInstance()
{
  static CDCMCCurlerCloneLookUp cloneInfo;
  return cloneInfo;
}

std::map<const ITrackType, std::vector<CDCTrack*>> CDCMCCurlerCloneLookUp::getMapMCIDToCDCTracks(
                                                  std::vector<CDCTrack>& cdcTracks)
{
  const CDCMCTrackLookUp& cdcMCTrackLookUp = CDCMCTrackLookUp::getInstance();

  /// Map of MCTrackIds to vectors of matching CDCTrack pointers
  /// TODO: is purity threshold for "matched" definition okay?
  std::map<const ITrackType, std::vector<CDCTrack*>> mapMCTrackIDToCDCTracks;

  for (CDCTrack& cdcTrack : cdcTracks) {
    CDCTrack* ptrCDCTrack = &cdcTrack;
    ITrackType mcTrackID = cdcMCTrackLookUp.getMCTrackId(ptrCDCTrack);

    if (mapMCTrackIDToCDCTracks.find(mcTrackID) == mapMCTrackIDToCDCTracks.end()) {
      // mcTrackID not yet in map, so add element
      mapMCTrackIDToCDCTracks.emplace(mcTrackID, std::vector<CDCTrack*> {ptrCDCTrack});
    } else { // mcTrackID already in map, so add Track to vector of tracks
      mapMCTrackIDToCDCTracks[mcTrackID].push_back(ptrCDCTrack);
    }
  }
  return mapMCTrackIDToCDCTracks;
}

CDCTrack* CDCMCCurlerCloneLookUp::findNonCurlerCloneCDCTrack(std::vector<CDCTrack*> matchedTrackPtrs)
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

void CDCMCCurlerCloneLookUp::clear()
{
  m_cdcTrackIsCurlerCloneMap.clear();
}

/// Fill LookUp Table which stores information, which tracks are clones from curlers
void CDCMCCurlerCloneLookUp::fill(std::vector<CDCTrack>& cdcTracks)
{
  /// per default, set all tracks to "not clone"
  for (const CDCTrack& cdcTrack : cdcTracks) {
    const CDCTrack* ptrCDCTrack = &cdcTrack;
    m_cdcTrackIsCurlerCloneMap[ptrCDCTrack] = false; // not clone
  }

  /// get lookup table of MC track IDs to vectors of CDC track pointers
  std::map<const ITrackType, std::vector<CDCTrack*>> mapMCIDToCDCTracks = getMapMCIDToCDCTracks(cdcTracks);

  for (auto& mcIDToCDCTracks : mapMCIDToCDCTracks) {
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

bool CDCMCCurlerCloneLookUp::isTrackCurlerClone(const CDCTrack& cdcTrack)
{
  const CDCTrack* ptrCDCTrack = &cdcTrack;

  if (m_cdcTrackIsCurlerCloneMap.find(ptrCDCTrack) == m_cdcTrackIsCurlerCloneMap.end()) {
    B2FATAL("No entry for this CDC track in m_cdcTrackIsCurlerCloneMap");
  }
  if (m_cdcTrackIsCurlerCloneMap.size() == 0) {
    B2FATAL("m_cdcTrackIsCurlerCloneMap is empty.");
  }
  return m_cdcTrackIsCurlerCloneMap[ptrCDCTrack];
}
