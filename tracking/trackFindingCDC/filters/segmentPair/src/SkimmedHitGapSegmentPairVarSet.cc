/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/SkimmedHitGapSegmentPairVarSet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SkimmedHitGapSegmentPairVarSet::SkimmedHitGapSegmentPairVarSet()
  : Super()
{
}

bool SkimmedHitGapSegmentPairVarSet::accept(const CDCSegmentPair*)
{
  if (fabs(var<named("delta_hit_pos_phi")>()) > 1) return false;
  if (fabs(var<named("delta_hit_alpha")>()) > 1.5) return false;
  if (var<named("delta_hit_distance")>() < 0) return false;
  if (var<named("hit_distance")>() > 45) return false;
  if (var<named("hit_long_distance")>() < 5) return false;
  if (var<named("hit_forward")>() < -5) return false;
  return true;
}
