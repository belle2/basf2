/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/minimal/TreeSearchFindlet.h>
#include <tracking/modules/cdcToVXDExtrapolator/filterBased/CDCTrackSpacePointCombinationFilterFactory.h>
#include <tracking/modules/cdcToVXDExtrapolator/filterBased/CKFCDCToVXDResultObject.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <svd/reconstruction/SVDRecoHit.h>

#include <genfit/Tools.h>
#include <Eigen/Dense>

namespace Eigen {
  using Vector5d = Matrix<double, 5, 1>;
  using RowVector5d = Matrix<double, 1, 5>;
  using Vector1d = Matrix<double, 1, 1>;
  using Matrix5d = Matrix<double, 5, 5>;
}

namespace Belle2 {
  class CKFCDCToVXDTreeSearchFindlet : public TrackFindingCDC::TreeSearchFindlet<CKFCDCToVXDStateObject,
    CDCTrackSpacePointCombinationFilterFactory> {
  public:
    using Super = TrackFindingCDC::TreeSearchFindlet<CKFCDCToVXDStateObject,
          CDCTrackSpacePointCombinationFilterFactory>;

  private:
    TrackFindingCDC::SortedVectorRange<const SpacePoint*> getMatchingHits(Super::StateIterator currentState) final {
      const unsigned int nextLayer = currentState->getLastLayer() - 1;
      return m_cachedHitMap[nextLayer];
    }

    void advance(Super::StateIterator currentState) final {
      // TODO: Test this!
      const SpacePoint* spacePoint = currentState->getSpacePoint();
      genfit::MeasuredStateOnPlane& measuredStateOnPlane = currentState->getMeasuredStateOnPlane();
      double& chi2 = currentState->getChi2();

      if (spacePoint)
      {
        std::vector<SVDRecoHit> measurements;
        for (const SVDCluster& relatedCluster : spacePoint->getRelationsTo<SVDCluster>()) {
          measurements.emplace_back(&relatedCluster);
        }

        const SVDRecoHit& recoHit = measurements.front();
        const VxdID& vxdID = recoHit.getSensorID();
        const short id = vxdID.getID();

        // Extrapolate from k-1 -> k. The mSoP is copied from the state before, so it is k-1.
        // By extrapolating, we will will go to k.
        const auto& cachedMSOP = m_cachedMeasuredStates.find(id);
        if (cachedMSOP == m_cachedMeasuredStates.end()) {
          const genfit::SharedPlanePtr& plane = recoHit.constructPlane(measuredStateOnPlane);
          measuredStateOnPlane.extrapolateToPlane(plane);
          m_cachedMeasuredStates[id] = measuredStateOnPlane;
        } else {
          measuredStateOnPlane.setStateCov(cachedMSOP->second.getState(), cachedMSOP->second.getCov());
        }

        // We will change the state x_k, the covariance C_k and the chi2
        Eigen::Vector5d x_k_old(measuredStateOnPlane.getState().GetMatrixArray());
        Eigen::Matrix5d C_k_old(measuredStateOnPlane.getCov().GetMatrixArray());

        Eigen::Vector5d x_k_new = x_k_old;
        Eigen::Matrix5d C_k_new = C_k_old;

        // Loop over the two clusters and extract the change for x_k and C_k.
        for (const auto& clusterMeasurement : measurements) {
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

          C_k_new -= K_k * H_k * C_k_old;
          x_k_new += K_k * (m_k - H_k * x_k_old);

          Eigen::Vector1d residual = m_k - H_k * x_k_new;

          chi2 += residual.transpose() * (V_k - H_k * C_k_new * H_k_t).inverse() * residual;

          B2DEBUG(200, "C_k_new " << C_k_new);
          B2DEBUG(200, "x_k_new " << x_k_new);
          B2DEBUG(200, "chi2 " << chi2);
        }

        measuredStateOnPlane.setState(TVectorD(5, x_k_new.data()));
        measuredStateOnPlane.setCov(TMatrixDSym(5, C_k_new.data()));
      }
    }

    void initializeEventCache(std::vector<RecoTrack*>& seedsVector,
                              std::vector<const SpacePoint*>& filteredHitVector) final {

      m_cachedHitMap.clear();

      const auto& hitSorterByLayer = [](const SpacePoint * lhs, const SpacePoint * rhs)
      {
        return lhs->getVxdID().getLayerNumber() < rhs->getVxdID().getLayerNumber();
      };

      std::sort(filteredHitVector.begin(), filteredHitVector.end(), hitSorterByLayer);

      for (unsigned int layerID = 0; layerID < 8; ++layerID)
      {
        const auto& onGivenLayerCheck = [layerID](const SpacePoint * spacePoint) {
          return spacePoint->getVxdID().getLayerNumber() == layerID;
        };

        const auto first = std::find_if(filteredHitVector.begin(), filteredHitVector.end(), onGivenLayerCheck);
        const auto last = std::find_if_not(first, filteredHitVector.end(), onGivenLayerCheck);

        m_cachedHitMap.emplace(layerID, TrackFindingCDC::SortedVectorRange<const SpacePoint*>(first, last));

        B2INFO("Storing in " << layerID << " " << std::distance(first, last));
      }
    }

    void initializeSeedCache(RecoTrack*) final {
      m_cachedMeasuredStates.clear();
    }


  private:
    /// Cache for sorted hits
    std::map<unsigned int, TrackFindingCDC::SortedVectorRange<const SpacePoint*>> m_cachedHitMap;
    /// Cache for extrapolated states
    std::map<short, genfit::MeasuredStateOnPlane> m_cachedMeasuredStates;
  };
}