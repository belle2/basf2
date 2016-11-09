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

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/numerics/Angle.h>
#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;

FitSegmentRelationVarSet::FitSegmentRelationVarSet()
  : Super()
{
}

bool FitSegmentRelationVarSet::extract(const Relation<const CDCRecoSegment2D>* ptrSegmentRelation)
{
  if (not ptrSegmentRelation) return false;

  const CDCRecoSegment2D* fromSegment = ptrSegmentRelation->first;
  const CDCRecoSegment2D* toSegment   = ptrSegmentRelation->second;

  const CDCRiemannFitter& riemannFitter = CDCRiemannFitter::getFitter();

  CDCTrajectory2D commonTrajectory2D = riemannFitter.fit(*fromSegment, *toSegment);

  finitevar<named("is_fitted")>() = commonTrajectory2D.isFitted();
  finitevar<named("curv")>() = commonTrajectory2D.getCurvature();

  using namespace NPerigeeParameterIndices;
  finitevar<named("curv_var")>() = commonTrajectory2D.getLocalVariance(c_Curv);

  finitevar<named("chi2")>() = std::fabs(commonTrajectory2D.getChi2());
  finitevar<named("chi2_per_ndf")>() = std::fabs(commonTrajectory2D.getChi2() / commonTrajectory2D.getNDF());
  finitevar<named("ndf")>() = commonTrajectory2D.getNDF();
  finitevar<named("p_value")>() =  commonTrajectory2D.getPValue();
  return true;
}
