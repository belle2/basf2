/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPairRelation/TruthSegmentPairRelationVarSet.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackStore.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool TruthSegmentPairRelationVarSet::extract(const Relation<const CDCSegmentPair>* ptrSegmentPairRelation)
{
  if (not ptrSegmentPairRelation) return false;

  const Relation<const CDCSegmentPair>& segmentPairRelation = *ptrSegmentPairRelation;

  const CDCSegmentPair* ptrFromSegmentPair = segmentPairRelation.getFrom();
  const CDCSegmentPair* ptrToSegmentPair = segmentPairRelation.getTo();

  const CDCSegmentPair& fromSegmentPair = *ptrFromSegmentPair;
  const CDCSegmentPair& toSegmentPair = *ptrToSegmentPair;

  const CDCSegment2D& startSegment = *fromSegmentPair.getFromSegment();
  const CDCSegment2D& middleSegment = *fromSegmentPair.getToSegment();
  const CDCSegment2D& endSegment = *toSegmentPair.getToSegment();

  const CDCMCSegment2DLookUp& mcSegmentLookUp = CDCMCSegment2DLookUp::getInstance();
  const CDCMCTrackStore& mcTrackStore = CDCMCTrackStore::getInstance();
  const std::map<ITrackType, CDCMCTrackStore::CDCHitVector>& mcTracks =
    mcTrackStore.getMCTracksByMCParticleIdx();

  CDCTrajectory3D trueTrajectory = mcSegmentLookUp.getTrajectory3D(&middleSegment);
  var<named("truth_curv")>() = trueTrajectory.getCurvatureXY();
  var<named("truth_tanl")>() = trueTrajectory.getTanLambda();
  var<named("truth_z")>() = trueTrajectory.getSupport().z();

  ITrackType startTrackId = mcSegmentLookUp.getMCTrackId(&startSegment);
  double startMCTrackSize = 0;
  if (mcTracks.count(startTrackId)) {
    startMCTrackSize = mcTracks.find(startTrackId)->second.size();
  }

  ITrackType middleTrackId = mcSegmentLookUp.getMCTrackId(&middleSegment);
  double middleMCTrackSize = 0;
  if (mcTracks.count(middleTrackId)) {
    middleMCTrackSize = mcTracks.find(middleTrackId)->second.size();
  }

  ITrackType endTrackId = mcSegmentLookUp.getMCTrackId(&endSegment);
  double endMCTrackSize = 0;
  if (mcTracks.count(endTrackId)) {
    endMCTrackSize = mcTracks.find(endTrackId)->second.size();
  }

  double trackFraction =
    startSegment.size() / startMCTrackSize +
    middleSegment.size() / middleMCTrackSize +
    endSegment.size() / endMCTrackSize;

  var<named("truth_track_fraction")>() = trackFraction;
  var<named("__weight__")>() = std::isfinite(trackFraction) ? trackFraction : 0;
  return true;
}
