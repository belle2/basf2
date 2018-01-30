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

  // Find the MC track with the highest number of hits in the segment
  const CDCMCTrackLookUp& mcTrackLookUp = CDCMCTrackLookUp::getInstance();
  const CDCMCHitLookUp& hitLookUp = CDCMCHitLookUp::getInstance();
  CDCMCCurlerCloneLookUp& curlerCloneLookUp = CDCMCCurlerCloneLookUp::getInstance();

  ITrackType trackMCMatch = mcTrackLookUp.getMCTrackId(ptrCDCTrack);
  bool trackIsFake = false;

  if (trackMCMatch == INVALID_ITRACK) {
    trackIsFake = true;
  } else {

    unsigned int numberOfCorrectHits = 0;
    unsigned int sumHitLoopNumbers = 0;
    for (const CDCRecoHit3D& recoHit : *ptrCDCTrack) {
      sumHitLoopNumbers += hitLookUp.getNLoops(recoHit.getWireHit().getHit());
      if (hitLookUp.getMCTrackId(recoHit.getWireHit().getHit()) == trackMCMatch) {
        numberOfCorrectHits++;
      }
    }
    if ((double)numberOfCorrectHits / ptrCDCTrack->size() < 0.8) {
      trackIsFake = true;
    } else {
      trackIsFake = false;
    }
  }
  bool trackIsClone = curlerCloneLookUp.isTrackCurlerClone(*ptrCDCTrack);

  var<named("weight")>() = not trackIsFake;
  var<named("track_is_curler_clone_truth")>() = trackIsClone;
  var<named("truth")>() = not trackIsClone;
  var<named("truth_first_nloops")>() = mcTrackLookUp.getFirstNLoops(ptrCDCTrack);
  var<named("truth_event_id")>() = m_eventMetaData->getEvent();
  var<named("truth_MCTrackID")>() = trackMCMatch;
  return true;
}
