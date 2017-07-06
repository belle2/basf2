/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/FitSegmentRelationVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool FitSegmentRelationVarSet::extract(const Relation<const CDCSegment2D>* ptrSegmentRelation)
{
  if (not ptrSegmentRelation) return false;

  const CDCSegment2D* fromSegment = ptrSegmentRelation->first;
  const CDCSegment2D* toSegment   = ptrSegmentRelation->second;

  const CDCRiemannFitter& riemannFitter = CDCRiemannFitter::getFitter();

  CDCTrajectory2D commonTrajectory2D = riemannFitter.fit(*fromSegment, *toSegment);

  double noRLChi2 = 0;
  for (const CDCRecoHit2D& recoHit2D : *fromSegment) {
    double dist = std::fabs(commonTrajectory2D.getDist2D(recoHit2D.getRefPos2D())) - recoHit2D.getRefDriftLength();
    noRLChi2 += dist * dist / recoHit2D.getRefDriftLengthVariance();
  }
  for (const CDCRecoHit2D& recoHit2D : *toSegment) {
    double dist = std::fabs(commonTrajectory2D.getDist2D(recoHit2D.getRefPos2D())) - recoHit2D.getRefDriftLength();
    noRLChi2 += dist * dist / recoHit2D.getRefDriftLengthVariance();
  }

  finitevar<named("is_fitted")>() = commonTrajectory2D.isFitted();
  finitevar<named("curv")>() = commonTrajectory2D.getCurvature();

  using namespace NPerigeeParameterIndices;
  finitevar<named("curv_var")>() = commonTrajectory2D.getLocalVariance(c_Curv);

  finitevar<named("chi2")>() = std::fabs(commonTrajectory2D.getChi2());
  finitevar<named("chi2_no_rl")>() = noRLChi2;
  finitevar<named("chi2_per_ndf")>() = std::fabs(commonTrajectory2D.getChi2() / commonTrajectory2D.getNDF());
  finitevar<named("chi2_no_rl_per_ndf")>() = noRLChi2 / commonTrajectory2D.getNDF();
  finitevar<named("ndf")>() = commonTrajectory2D.getNDF();
  finitevar<named("p_value")>() =  commonTrajectory2D.getPValue();
  return true;
}
