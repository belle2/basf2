/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Michael Eliachevitch                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/CurlerCloneTruthVarSet.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCCurlerCloneLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void CurlerCloneTruthVarSet::initialize()
{
  CDCMCManager::getInstance().requireTruthInformation();
  Super::initialize();
}

void CurlerCloneTruthVarSet::beginEvent()
{
  CDCMCManager::getInstance().fill();
  Super::beginEvent();
}

bool CurlerCloneTruthVarSet::extract(const CDCTrack* ptrCDCTrack)
{
  if (not ptrCDCTrack) return false;

  /// Find the MC track with the highest number of hits in the segment
  const CDCMCTrackLookUp& mcTrackLookUp = CDCMCTrackLookUp::getInstance();
  const CDCMCHitLookUp& hitLookUp = CDCMCHitLookUp::getInstance();
  CDCMCCurlerCloneLookUp& curlerCloneLookUp = CDCMCCurlerCloneLookUp::getInstance();

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

  bool trackIsClone = curlerCloneLookUp.isTrackCurlerClone(*ptrCDCTrack);

  bool matchedNotClone = (not trackIsClone) and (not trackIsFake);

  var<named("weight")>() = 1.0; // trackHasMinimalMatchPurity;
  var<named("truth_track_is_fake")>() = trackIsFake;
  var<named("truth_track_is_matched")>() = not trackIsFake;
  var<named("truth_matched_hits")>() = numberOfCorrectHits;
  var<named("truth_track_is_curler_clone")>() = trackIsClone;
  var<named("truth")>() = matchedNotClone; //not trackIsClone;
  var<named("truth_first_nloops")>() = mcTrackLookUp.getFirstNLoops(ptrCDCTrack);
  var<named("truth_event_id")>() = m_eventMetaData->getEvent();
  var<named("truth_MCTrackID")>() = trackMCMatch;
  return true;
}
