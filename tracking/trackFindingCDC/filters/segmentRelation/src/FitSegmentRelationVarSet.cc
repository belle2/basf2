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
#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

FitSegmentRelationVarSet::FitSegmentRelationVarSet()
  : Super()
{
}

bool FitSegmentRelationVarSet::extract(const Relation<const CDCRecoSegment2D>* ptrSegmentRelation)
{
  bool extracted = Super::extract(ptrSegmentRelation);
  if (not extracted or not ptrSegmentRelation) return false;

  const CDCRecoSegment2D* fromSegment = ptrSegmentRelation->first;
  const CDCRecoSegment2D* toSegment   = ptrSegmentRelation->second;

  const CDCTrajectory2D& fromFit = fromSegment->getTrajectory2D();
  const CDCTrajectory2D& toFit   = toSegment->getTrajectory2D();

  const CDCRiemannFitter& riemannFitter = CDCRiemannFitter::getFitter();

  CDCTrajectory2D commonTrajectory2D = riemannFitter.fit(*fromSegment, *toSegment);

  var<named("chi2_common")>() = commonTrajectory2D.getChi2();
  return true;
}
