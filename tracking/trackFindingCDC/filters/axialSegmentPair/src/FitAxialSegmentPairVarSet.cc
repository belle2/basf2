/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/axialSegmentPair/FitAxialSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool FitAxialSegmentPairVarSet::extract(const CDCAxialSegmentPair* ptrAxialSegmentPair)
{
  if (not ptrAxialSegmentPair) return false;

  const CDCAxialSegmentPair& axialSegmentPair = *ptrAxialSegmentPair;

  const CDCSegment2D* ptrFromSegment = axialSegmentPair.getStartSegment();
  const CDCSegment2D* ptrToSegment = axialSegmentPair.getEndSegment();

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  const CDCRiemannFitter& riemannFitter = CDCRiemannFitter::getFitter();

  CDCTrajectory2D commonTrajectory2D = riemannFitter.fit(fromSegment, toSegment);
  axialSegmentPair.setTrajectory2D(commonTrajectory2D);

  double noRLChi2 = 0;
  for (const CDCRecoHit2D& recoHit2D : fromSegment) {
    double dist = std::fabs(commonTrajectory2D.getDist2D(recoHit2D.getRefPos2D())) - recoHit2D.getRefDriftLength();
    noRLChi2 += dist * dist / recoHit2D.getRefDriftLengthVariance();
  }
  for (const CDCRecoHit2D& recoHit2D : toSegment) {
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
