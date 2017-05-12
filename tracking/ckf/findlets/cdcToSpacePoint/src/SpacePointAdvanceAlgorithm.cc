/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/findlets/cdcToSpacePoint/CDCToSpacePointTreeSearchFindlet.h>

#include <svd/reconstruction/SVDRecoHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool SpacePointAdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const SpacePoint* spacePoint)
{
  // We always use the first cluster here to create the plane. Should not make much difference?
  SVDRecoHit recoHit(spacePoint->getRelated<SVDCluster>());

  // The mSoP plays no role here (it is unused in the function)
  const genfit::SharedPlanePtr& plane = recoHit.constructPlane(measuredStateOnPlane);

  try {
    measuredStateOnPlane.extrapolateToPlane(plane);
  } catch (genfit::Exception e) {
    B2WARNING(e.what());
    return false;
  }

  return true;
}

Weight SpacePointAdvanceAlgorithm::operator()(CKFCDCToVXDStateObject& currentState)
{
  B2ASSERT("Encountered invalid state", not currentState.isFitted() and not currentState.isAdvanced());

  const SpacePoint* spacePoint = currentState.getHit();

  if (not spacePoint) {
    // If we do not have a space point, we do not need to do anything here.
    currentState.setAdvanced();
    return 1;
  }

  // This is the mSoP we will edit.
  genfit::MeasuredStateOnPlane measuredStateOnPlane = currentState.getMeasuredStateOnPlaneFromParent();

  if (not extrapolate(measuredStateOnPlane, spacePoint)) {
    return std::nan("");
  }

  currentState.setMeasuredStateOnPlane(measuredStateOnPlane);
  currentState.setAdvanced();
  return 1;
}