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

#include <Eigen/Dense>

namespace Eigen {
  using Vector5d = Matrix<double, 5, 1>;
  using RowVector5d = Matrix<double, 1, 5>;
  using Vector1d = Matrix<double, 1, 1>;
  using Matrix5d = Matrix<double, 5, 5>;
}

using namespace Belle2;
using namespace TrackFindingCDC;

void CDCToSpacePointTreeSearchFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter("useCaching", m_param_useCaching, "Use caching or not", m_param_useCaching);
}

SortedVectorRange<const SpacePoint*> CDCToSpacePointTreeSearchFindlet::getMatchingHits(Super::StateIterator currentState)
{
  const unsigned int currentNumber = currentState->getNumber();

  if (currentNumber == CKFCDCToVXDStateObject::N or currentState->isOnOverlapLayer()) {
    // next layer is not an overlap one, so we can just return all hits of this layer.
    const unsigned int nextLayer = currentState->extractGeometryLayer() - 1;
    return m_cachedHitMap[nextLayer];
  } else {
    // next layer is an overlap one, so lets return all hits from the same layer, that are on a
    // ladder which is one below the last added hit.
    const SpacePoint* lastAddedSpacePoint = currentState->getSpacePoint();
    if (not lastAddedSpacePoint) {
      // No hit was added on the layer, so no overlap can occur.
      return SortedVectorRange<const SpacePoint*>();
    }

    const unsigned int ladderNumber = lastAddedSpacePoint->getVxdID().getLadderNumber();
    const unsigned int currentLayer = currentState->extractGeometryLayer();
    const unsigned int maximumLadderNumber = m_maximumLadderNumbers[currentLayer];

    // the reason for this strange formula is the numbering scheme in the VXD.
    // we first substract 1 from the ladder number to have a ladder counting from 0 to N - 1,
    // then we subtract one to get to the next (overlapping) ladder and % N, to also cope for the
    // highest number. Then we add 1 again, to go from the counting from 0 .. N-1 to 1 .. N.
    const unsigned int overlappingLadder = ((ladderNumber - 1) - 1) % maximumLadderNumber + 1;

    B2DEBUG(100, "Overlap check on " << ladderNumber << " using from " << overlappingLadder);

    const auto& onNextLadderCheck = [overlappingLadder](const SpacePoint * spacePoint) {

      return spacePoint->getVxdID().getLadderNumber() == overlappingLadder;
    };

    const auto& hitsOfCurrentLayer = m_cachedHitMap[currentLayer];
    const auto first = std::find_if(hitsOfCurrentLayer.begin(), hitsOfCurrentLayer.end(), onNextLadderCheck);
    const auto last = std::find_if_not(first, hitsOfCurrentLayer.end(), onNextLadderCheck);

    B2DEBUG(100, "Overlap " << currentLayer << " " << lastAddedSpacePoint->getVxdID() <<  " " << std::distance(first, last));
    for (auto it = first; it != last; it++) {
      B2DEBUG(100, (*it)->getVxdID());
    }
    return SortedVectorRange<const SpacePoint*>(first, last);
  }

}

bool CDCToSpacePointTreeSearchFindlet::fit(Super::StateIterator currentState)
{
  B2ASSERT("Encountered invalid state", not currentState->isFitted() and currentState->isAdvanced());

  const SpacePoint* spacePoint = currentState->getSpacePoint();

  if (not spacePoint) {
    // If we do not have a space point, we do not need to do anything here.
    currentState->setFitted();
    return true;
  }

  genfit::MeasuredStateOnPlane& measuredStateOnPlane = currentState->getMeasuredStateOnPlane();
  double& chi2 = currentState->getChi2();

  // We will change the state x_k, the covariance C_k and the chi2
  Eigen::Vector5d x_k_old(measuredStateOnPlane.getState().GetMatrixArray());
  Eigen::Matrix5d C_k_old(measuredStateOnPlane.getCov().GetMatrixArray());

  // Loop over the two clusters and extract the change for x_k and C_k.
  for (const SVDCluster& relatedCluster : spacePoint->getRelationsTo<SVDCluster>()) {
    SVDRecoHit clusterMeasurement(&relatedCluster);
    // Important: measuredStateOnPlane must already be extrapolated to the correct plane.
    // Only the plane and the rep are accessed (but the rep has no meaningful members).
    const std::vector<genfit::MeasurementOnPlane*> measurementsOnPlane = clusterMeasurement.constructMeasurementsOnPlane(
          measuredStateOnPlane);

    B2ASSERT("There should be exactly one measurement on plane", measurementsOnPlane.size() == 1);
    const genfit::MeasurementOnPlane& measurementOnPlane = *(measurementsOnPlane.front());

    Eigen::Vector1d m_k(measurementOnPlane.getState().GetMatrixArray());
    Eigen::RowVector5d H_k(measurementOnPlane.getHMatrix()->getMatrix().GetMatrixArray());
    Eigen::Vector5d H_k_t = H_k.transpose();
    Eigen::Vector1d V_k(measurementOnPlane.getCov().GetMatrixArray());

    const Eigen::Vector5d& K_k = C_k_old * H_k_t * (V_k + H_k * C_k_old * H_k_t).inverse();

    B2DEBUG(200, "C_k_old " << C_k_old);
    B2DEBUG(200, "H_k " << H_k);
    B2DEBUG(200, "m_k " << m_k);
    B2DEBUG(200, "V_k " << V_k);
    B2DEBUG(200, "x_k_old " << x_k_old);
    B2DEBUG(200, "K_k " << K_k);

    C_k_old -= K_k * H_k * C_k_old;
    x_k_old += K_k * (m_k - H_k * x_k_old);

    Eigen::Vector1d residual = m_k - H_k * x_k_old;

    chi2 += (residual.transpose() * (V_k - H_k * C_k_old * H_k_t).inverse() * residual).value();

    B2DEBUG(200, "C_k_old " << C_k_old);
    B2DEBUG(200, "x_k_old " << x_k_old);
    B2DEBUG(100, "chi2 " << chi2);
  }

  measuredStateOnPlane.setState(TVectorD(5, x_k_old.data()));
  measuredStateOnPlane.setCov(TMatrixDSym(5, C_k_old.data()));

  currentState->setFitted();
  return true;
}

bool CDCToSpacePointTreeSearchFindlet::advance(Super::StateIterator currentState)
{
  B2ASSERT("Encountered invalid state", not currentState->isFitted() and not currentState->isAdvanced());

  const SpacePoint* spacePoint = currentState->getSpacePoint();

  if (not spacePoint) {
    // If we do not have a space point, we do not need to do anything here.
    currentState->setAdvanced();
    return true;
  }

  // We always use the first cluster here to create the plane. Should not make much difference?
  SVDRecoHit recoHit(spacePoint->getRelated<SVDCluster>());

  // This is the mSoP we will edit.
  genfit::MeasuredStateOnPlane& measuredStateOnPlane = currentState->getMeasuredStateOnPlane();

  // This mSoP may help us for extrapolation
  genfit::MeasuredStateOnPlane& parentsCachedMSoP = currentState->getParentsCachedMeasuredStateOnPlane();

  // Possibility 1: extrapolate onto a "common ground", e.g. the average radius and store this as a cache.
  // In possibility 2 (below), we start with the former
  if (m_param_useCaching and m_param_useCachingOne) {
    // Test if we have already calculated an extrapolated state in former calculations. The parent
    // knows about this.
    if (not currentState->parentHasCache()) {
      // If not, use the parents mSoP (which is copied into the cache before) as a starting point and extrapolate this.
      // parentsCachedMSoP.extrapolateToCone();
      B2FATAL("Extrapolation not implemented in the moment");
      currentState->setParentHasCache();
    }
  }

  // only use the cache if it is there.
  // Otherwise, just stay with the mSoP, which is equal to the parents (fitted) state.
  if (m_param_useCaching and currentState->parentHasCache()) {
    measuredStateOnPlane = parentsCachedMSoP;
  }

  // The mSoP plays no role here (it is unused in the function)
  const genfit::SharedPlanePtr& plane = recoHit.constructPlane(measuredStateOnPlane);

  try {
    measuredStateOnPlane.extrapolateToPlane(plane);
  } catch (genfit::Exception e) {
    B2WARNING(e.what());
    return false;
  }


  // Possibility 2: extrapolate this state as normal and store this as a cache (the other then maybe have to
  //                extrapolate back a bit, but the calculation is much easier)
  if (m_param_useCaching and not m_param_useCachingOne) {
    // TODO: Always or only store the first one?
    parentsCachedMSoP = measuredStateOnPlane;
    currentState->setParentHasCache();
  }

  currentState->setAdvanced();
  return true;
}

void CDCToSpacePointTreeSearchFindlet::initializeEventCache(std::vector<RecoTrack*>& seedsVector,
                                                            std::vector<const SpacePoint*>& filteredHitVector)
{
  m_cachedHitMap.clear();

  const auto& hitSorterByID = [](const SpacePoint * lhs, const SpacePoint * rhs) {
    return lhs->getVxdID() < rhs->getVxdID();
  };

  std::sort(filteredHitVector.begin(), filteredHitVector.end(), hitSorterByID);

  for (unsigned int layerID = 3; layerID <= 6; ++layerID) {
    const auto& onGivenLayerCheck = [layerID](const SpacePoint * spacePoint) {
      return spacePoint->getVxdID().getLayerNumber() == layerID;
    };

    const auto first = std::find_if(filteredHitVector.begin(), filteredHitVector.end(), onGivenLayerCheck);
    const auto last = std::find_if_not(first, filteredHitVector.end(), onGivenLayerCheck);

    m_cachedHitMap.emplace(layerID, SortedVectorRange<const SpacePoint*>(first, last));

    B2DEBUG(100, "Storing in " << layerID << " " << std::distance(first, last));
    for (auto it = first; it != last; it++) {
      B2DEBUG(100, (*it)->getVxdID());
    }
  }

  // Include the PXD layers as empty
  m_cachedHitMap.emplace(2, SortedVectorRange<const SpacePoint*>());
  m_cachedHitMap.emplace(1, SortedVectorRange<const SpacePoint*>());
}