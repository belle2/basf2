/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/TruthSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackStore.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/numerics/Angle.h>

#include <cdc/dataobjects/CDCSimHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool TruthSegmentPairVarSet::extract(const CDCSegmentPair* ptrSegmentPair)
{
  if (not ptrSegmentPair) return false;

  const CDCSegmentPair& segmentPair = *ptrSegmentPair;

  const CDCSegment2D* ptrFromSegment = segmentPair.getFromSegment();
  const CDCSegment2D* ptrToSegment = segmentPair.getToSegment();

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();
  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();
  const CDCMCTrackStore& mcTrackStore = CDCMCTrackStore::getInstance();
  const std::map<ITrackType, CDCMCTrackStore::CDCHitVector>& mcTracks =
    mcTrackStore.getMCTracksByMCParticleIdx();

  CDCTrajectory3D trueTrajectory = mcSegmentLookUp.getTrajectory3D(&toSegment);
  var<named("truth_curv")>() = trueTrajectory.getCurvatureXY();
  var<named("truth_tanl")>() = trueTrajectory.getTanLambda();
  var<named("truth_z")>() = trueTrajectory.getSupport().z();

  // Hits
  const CDCRecoHit2D& fromLastHit = fromSegment.back();
  const CDCRecoHit2D& toFirstHit = toSegment.front();

  const CDCSimHit* fromSimHit = mcHitLookUp.getClosestPrimarySimHit(fromLastHit.getWireHit().getHit());
  const CDCSimHit* toSimHit = mcHitLookUp.getClosestPrimarySimHit(toFirstHit.getWireHit().getHit());
  if (fromSimHit and toSimHit) {
    double truthFromAlpha = -fromSimHit->getPosTrack().DeltaPhi(fromSimHit->getMomentum());
    double truthToAlpha = -toSimHit->getPosTrack().DeltaPhi(toSimHit->getMomentum());
    var<named("truth_from_alpha")>() = truthFromAlpha;
    var<named("truth_to_alpha")>() = truthToAlpha;
    var<named("truth_delta_alpha")>() = AngleUtil::normalised(truthToAlpha - truthFromAlpha);
  } else {
    var<named("truth_from_alpha")>() = NAN;
    var<named("truth_to_alpha")>() = NAN;
    var<named("truth_delta_alpha")>() = NAN;
  }

  ITrackType fromTrackId = mcSegmentLookUp.getMCTrackId(&fromSegment);
  double fromMCTrackSize = 0;
  if (mcTracks.count(fromTrackId)) {
    fromMCTrackSize = mcTracks.find(fromTrackId)->second.size();
  }

  ITrackType toTrackId = mcSegmentLookUp.getMCTrackId(&toSegment);
  double toMCTrackSize = 0;
  if (mcTracks.count(toTrackId)) {
    toMCTrackSize = mcTracks.find(toTrackId)->second.size();
  }

  double trackFraction = fromSegment.size() / fromMCTrackSize + toSegment.size() / toMCTrackSize;
  var<named("truth_track_fraction")>() = trackFraction;
  var<named("__weight__")>() = std::isfinite(trackFraction) ? trackFraction : 0;
  return true;
}
