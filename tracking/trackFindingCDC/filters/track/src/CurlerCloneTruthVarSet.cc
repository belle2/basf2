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

bool CurlerCloneTruthVarSet::extract(const CDCTrack* track)
{
  if (not track) return false;

  // Find the MC track with the highest number of hits in the segment
  const CDCMCTrackLookUp& mcTrackLookup = CDCMCTrackLookUp::getInstance();
  const CDCMCHitLookUp& hitLookup = CDCMCHitLookUp::getInstance();
  CDCMCCurlerCloneLookUp& curlerCloneLookUp = CDCMCCurlerCloneLookUp::getInstance();

  ITrackType trackMCMatch = mcTrackLookup.getMCTrackId(track);
  bool trackIsFake = false;

  if (trackMCMatch == INVALID_ITRACK) {
    trackIsFake = true;
  } else {

    unsigned int numberOfCorrectHits = 0;
    unsigned int sumHitLoopNumbers = 0;
    for (const CDCRecoHit3D& recoHit : *track) {
      sumHitLoopNumbers += hitLookup.getNLoops(recoHit.getWireHit().getHit());
      if (hitLookup.getMCTrackId(recoHit.getWireHit().getHit()) == trackMCMatch) {
        numberOfCorrectHits++;
      }
    }
    if ((double)numberOfCorrectHits / track->size() < 0.8) {
      trackIsFake = true;
    } else {
      trackIsFake = false;
    }
  }
  bool trackIsClone = curlerCloneLookUp.isTrackCurlerClone(*track);

  var<named("weight")>() = trackIsFake;
  var<named("track_is_curler_clone_truth")>() = trackIsClone;
  var<named("truth")>() = not trackIsClone;
  return true;
}
