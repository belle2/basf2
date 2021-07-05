/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/BestMatchedTruthVarSet.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCCloneLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void BestMatchedTruthVarSet::initialize()
{
  CDCMCManager::getInstance().requireTruthInformation();
  Super::initialize();
}

void BestMatchedTruthVarSet::beginEvent()
{
  CDCMCManager::getInstance().fill();
  Super::beginEvent();
}

bool BestMatchedTruthVarSet::extract(const CDCTrack* ptrCDCTrack)
{
  if (not ptrCDCTrack) return false;

  /// Find the MC track with the highest number of hits in the segment
  const CDCMCTrackLookUp& mcTrackLookUp = CDCMCTrackLookUp::getInstance();
  const CDCMCHitLookUp& hitLookUp = CDCMCHitLookUp::getInstance();
  CDCMCCloneLookUp& cloneLookUp = CDCMCCloneLookUp::getInstance();

  ITrackType trackMCMatch = mcTrackLookUp.getMCTrackId(ptrCDCTrack);
  bool trackIsFake = false;

  unsigned int numberOfCorrectHits = 0;

  /// Flag if CDCTrack is matched in CDCMCTrackLookUp, which uses m_minimalMatchPurity = 0.5
  bool trackHasMinimalMatchPurity = (trackMCMatch != INVALID_ITRACK);

  if (not trackHasMinimalMatchPurity) {
    trackIsFake = true;
  } else {
    /// count number of correct hits
    auto hitIsCorrect = [&hitLookUp, &trackMCMatch](const CDCRecoHit3D & recoHit) {
      return hitLookUp.getMCTrackId(recoHit.getWireHit().getHit()) == trackMCMatch;
    };
    numberOfCorrectHits = std::count_if(begin(*ptrCDCTrack), end(*ptrCDCTrack), hitIsCorrect);

    const double purity = (double)numberOfCorrectHits / ptrCDCTrack->size();
    if (purity < 0.8) {
      trackIsFake = true;
    } else {
      trackIsFake = false;
    }
  }

  bool trackIsClone = cloneLookUp.isTrackClone(*ptrCDCTrack);
  bool matchedNotClone = (not trackIsClone) and (not trackIsFake);
  var<named("truth")>() = matchedNotClone;
  var<named("truth_track_is_fake")>() = trackIsFake;
  var<named("truth_track_is_matched")>() = not trackIsFake;
  var<named("truth_matched_hits")>() = numberOfCorrectHits;
  var<named("truth_track_is_clone")>() = trackIsClone;
  var<named("truth_first_nloops")>() = mcTrackLookUp.getFirstNLoops(ptrCDCTrack);
  var<named("truth_event_id")>() = m_eventMetaData->getEvent();
  var<named("truth_MCTrackID")>() = trackMCMatch;
  return true;
}
