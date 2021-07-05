/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/mclookup/CDCMCCloneLookUp.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCMCCloneLookUp& CDCMCCloneLookUp::getInstance()
{
  static CDCMCCloneLookUp cloneInfo;
  return cloneInfo;
}

std::map<const ITrackType, std::vector<CDCTrack*>> CDCMCCloneLookUp::getMatchedCDCTracksByMCID(
                                                  std::vector<CDCTrack>& cdcTracks)
{
  const CDCMCTrackLookUp& cdcMCTrackLookUp = CDCMCTrackLookUp::getInstance();

  // Map of MCTrackIds to vectors of matching CDCTrack pointers
  std::map<const ITrackType, std::vector<CDCTrack*>> mapMCTrackIDToCDCTracks;

  for (CDCTrack& cdcTrack : cdcTracks) {
    CDCTrack* ptrCDCTrack = &cdcTrack;

    ITrackType mcTrackID = cdcMCTrackLookUp.getMCTrackId(ptrCDCTrack);

    // only if matching MCTrack with 50% minimal purity was found
    if (mcTrackID != INVALID_ITRACK) {
      mapMCTrackIDToCDCTracks[mcTrackID].push_back(ptrCDCTrack);
    }
  }
  return mapMCTrackIDToCDCTracks;
}

unsigned int CompareCDCTracks::getNumberOfCorrectHits(const CDCTrack* ptrCDCTrack) const
{
  ITrackType mcTrackID = m_CDCMCTrackLookUp.getMCTrackId(ptrCDCTrack);
  auto hitIsCorrect = [this, &mcTrackID](const CDCRecoHit3D & recoHit) {
    return m_CDCMCHitLookUp.getMCTrackId(recoHit.getWireHit().getHit()) == mcTrackID;
  };
  return std::count_if(begin(*ptrCDCTrack), end(*ptrCDCTrack), hitIsCorrect);
}

/// Functor definition of comparison function for findBestMatchedTrack
bool CompareCDCTracks::operator()(const CDCTrack* ptrCDCTrack1, const CDCTrack* ptrCDCTrack2) const
{
  Index firstNLoopsTrack1 = m_CDCMCTrackLookUp.getFirstNLoops(ptrCDCTrack1);
  Index firstNLoopsTrack2 = m_CDCMCTrackLookUp.getFirstNLoops(ptrCDCTrack2);

  // Look for track with smallest NLoops of first hit.
  // If it is equal, use track with the larger number of correct hits.
  if (firstNLoopsTrack1 == firstNLoopsTrack2) {
    const unsigned int nCorrectHitsTrack1 = getNumberOfCorrectHits(ptrCDCTrack1);
    const unsigned int nCorrectHitsTrack2 = getNumberOfCorrectHits(ptrCDCTrack2);

    const bool firstTrackBetter = nCorrectHitsTrack1 > nCorrectHitsTrack2;
    return firstTrackBetter;
  }
  const bool firstTrackBetter = (firstNLoopsTrack1 < firstNLoopsTrack2);
  return firstTrackBetter;
}

CDCTrack* CDCMCCloneLookUp::findBestMatchedTrack(std::vector<CDCTrack*> matchedTrackPtrs)
{
  const CDCMCTrackLookUp& cdcMCTrackLookUp = CDCMCTrackLookUp::getInstance();
  const CDCMCHitLookUp& cdcMCHitLookUp = CDCMCHitLookUp::getInstance();

  const CompareCDCTracks compareCDCTracks(cdcMCTrackLookUp, cdcMCHitLookUp);
  CDCTrack* ptrNonCloneTrack =
    *(std::min_element(begin(matchedTrackPtrs), end(matchedTrackPtrs), compareCDCTracks));

  return ptrNonCloneTrack;
}

void CDCMCCloneLookUp::clear()
{
  m_cdcTrackIsCloneMap.clear();
}

/// Fill LookUp Table which stores information, which tracks are clones from curlers
void CDCMCCloneLookUp::fill(std::vector<CDCTrack>& cdcTracks)
{
  /// per default, set all tracks to "not clone"
  for (const CDCTrack& cdcTrack : cdcTracks) {
    const CDCTrack* ptrCDCTrack = &cdcTrack;
    m_cdcTrackIsCloneMap[ptrCDCTrack] = false; // not clone
  }

  /// get lookup table of MC track IDs to vectors of CDC track pointers
  std::map<const ITrackType, std::vector<CDCTrack*>> matchedCDCTracksByMCID =
                                                    getMatchedCDCTracksByMCID(cdcTracks);

  for (auto& mcIDAndCDCTracks : matchedCDCTracksByMCID) {
    /// Vector of track pointers which are mapped to this mcTrackID
    std::vector<CDCTrack*>& matchedTrackPtrs = mcIDAndCDCTracks.second;

    if (matchedTrackPtrs.size() == 1) { // only one matching track
      m_cdcTrackIsCloneMap[matchedTrackPtrs.at(0)] = false; // not clone

    } else { // multiple matching tracks
      for (const CDCTrack* ptrCDCTrack : matchedTrackPtrs) {
        m_cdcTrackIsCloneMap[ptrCDCTrack] = true; // default that all are clones
      }
      const CDCTrack* ptrNonCloneTrack = findBestMatchedTrack(matchedTrackPtrs);
      m_cdcTrackIsCloneMap[ptrNonCloneTrack] = false; // not clone
    }
  }
}

bool CDCMCCloneLookUp::isTrackClone(const CDCTrack& cdcTrack)
{
  const CDCTrack* ptrCDCTrack = &cdcTrack;

  const CDCMCTrackLookUp& cdcMCTrackLookUp = CDCMCTrackLookUp::getInstance();
  if (cdcMCTrackLookUp.getMCTrackId(ptrCDCTrack) == INVALID_ITRACK) {
    return false; // track is not matched
  } else {
    return m_cdcTrackIsCloneMap.at(ptrCDCTrack);
  }
}
