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

    /// only if matching MCTrack with 50% minimal purity was found
    if (mcTrackID != INVALID_ITRACK) {
      if (mapMCTrackIDToCDCTracks.find(mcTrackID) == mapMCTrackIDToCDCTracks.end()) {
        // mcTrackID not yet in map, so add element
        mapMCTrackIDToCDCTracks.emplace(mcTrackID, std::vector<CDCTrack*> {ptrCDCTrack});
      } else { // mcTrackID already in map, so add Track to vector of tracks
        mapMCTrackIDToCDCTracks[mcTrackID].push_back(ptrCDCTrack);
      }
    }
  }
  return mapMCTrackIDToCDCTracks;
}

/// Functor definition of comparison function for findNonCurlerCloneTrack
bool CompareCurlerTracks::operator()(const CDCTrack* ptrTrack1,  const CDCTrack* ptrTrack2) const
{
  // Maybe add reference of instance in functor ctor?
  const CDCMCTrackLookUp& cdcMCTrackLookUp = CDCMCTrackLookUp::getInstance();

  Index firstNLoopsTrack1 = cdcMCTrackLookUp.getFirstNLoops(ptrTrack1);
  Index firstNLoopsTrack2 = cdcMCTrackLookUp.getFirstNLoops(ptrTrack2);
  bool isTrack1Better;

  // Look for track with smallest NLoops of first hit.
  // If it is equal, use track with the larger amount of hits.
  if (firstNLoopsTrack1 == firstNLoopsTrack2) {
    isTrack1Better = ptrTrack1->size() > ptrTrack2->size();
  } else {
    isTrack1Better = (firstNLoopsTrack1 < firstNLoopsTrack2);
  }
  return isTrack1Better;
}

CDCTrack* CDCMCCurlerCloneLookUp::findNonCurlerCloneTrack(std::vector<CDCTrack*> matchedTrackPtrs)
{
  CompareCurlerTracks compareCurlerTracks;
  CDCTrack* ptrNonCurlerCloneTrack =
    *(std::min_element(matchedTrackPtrs.begin(), matchedTrackPtrs.end(), compareCurlerTracks));
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
        m_cdcTrackIsCurlerCloneMap[ptrCDCTrack] = true; // default that all are clones
      }
      const CDCTrack* ptrNonCurlerCloneTrack = findNonCurlerCloneTrack(matchedTrackPtrs);
      m_cdcTrackIsCurlerCloneMap[ptrNonCurlerCloneTrack] = false; // not clone
    }
  }
}

bool CDCMCCurlerCloneLookUp::isTrackCurlerClone(const CDCTrack& cdcTrack)
{
  const CDCTrack* ptrCDCTrack = &cdcTrack;

  const CDCMCTrackLookUp& cdcMCTrackLookUp = CDCMCTrackLookUp::getInstance();
  if (cdcMCTrackLookUp.getMCTrackId(ptrCDCTrack) == INVALID_ITRACK) {
    return false; // track is not matched
  } else {
    if (m_cdcTrackIsCurlerCloneMap.find(ptrCDCTrack) == m_cdcTrackIsCurlerCloneMap.end()) {
      B2FATAL("No entry for this CDC track in m_cdcTrackIsCurlerCloneMap");
    }
    return m_cdcTrackIsCurlerCloneMap[ptrCDCTrack];
  }
}
