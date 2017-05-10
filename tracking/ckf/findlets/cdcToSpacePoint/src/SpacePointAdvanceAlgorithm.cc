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

Weight SpacePointAdvanceAlgorithm::operator()(CKFCDCToVXDStateObject& currentState)
{
  B2ASSERT("Encountered invalid state", not currentState.isFitted() and not currentState.isAdvanced());

  const SpacePoint* spacePoint = currentState.getHit();

  if (not spacePoint) {
    // If we do not have a space point, we do not need to do anything here.
    currentState.setAdvanced();
    return 1;
  }

  // We always use the first cluster here to create the plane. Should not make much difference?
  SVDRecoHit recoHit(spacePoint->getRelated<SVDCluster>());

  // This is the mSoP we will edit.
  genfit::MeasuredStateOnPlane measuredStateOnPlane = currentState.getMeasuredStateOnPlaneFromParent();

  // This mSoP may help us for extrapolation
  // TODO
  /*genfit::MeasuredStateOnPlane& parentsCachedMSoP = currentState.getMeasuredStateOnPlane();

  // Possibility 1: extrapolate onto a "common ground", e.g. the average radius and store this as a cache.
  // In possibility 2 (below), we start with the former
  if (m_param_useCaching and m_param_useCachingOne) {
    // Test if we have already calculated an extrapolated state in former calculations. The parent
    // knows about this.
    if (not currentState.parentHasCache()) {
      // If not, use the parents mSoP (which is copied into the cache before) as a starting point and extrapolate this.
      // parentsCachedMSoP.extrapolateToCone();
      B2FATAL("Extrapolation not implemented in the moment");
      currentState.setParentHasCache();
    }
  }

  // only use the cache if it is there.
  // Otherwise, just stay with the mSoP, which is equal to the parents (fitted) state.
  if (m_param_useCaching and currentState.parentHasCache()) {
    measuredStateOnPlane = parentsCachedMSoP;
  }*/

  // The mSoP plays no role here (it is unused in the function)
  const genfit::SharedPlanePtr& plane = recoHit.constructPlane(measuredStateOnPlane);

  try {
    measuredStateOnPlane.extrapolateToPlane(plane);
  } catch (genfit::Exception e) {
    B2WARNING(e.what());
    return std::nan("");
  }

  // Possibility 2: extrapolate this state as normal and store this as a cache (the other then maybe have to
  //                extrapolate back a bit, but the calculation is much easier)
  // TODO
  /*if (m_param_useCaching and not m_param_useCachingOne) {
    // TODO: Always or only store the first one?
    parentsCachedMSoP = measuredStateOnPlane;
    currentState.setParentHasCache();
  }*/

  currentState.setMeasuredStateOnPlane(measuredStateOnPlane);
  currentState.setAdvanced();
  return 1;
}