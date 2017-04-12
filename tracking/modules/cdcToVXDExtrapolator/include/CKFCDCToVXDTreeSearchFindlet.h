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
#include <tracking/modules/cdcToVXDExtrapolator/CDCTrackSpacePointCombinationFilterFactory.h>
#include <tracking/modules/cdcToVXDExtrapolator/CKFCDCToVXDResultObject.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/utilities/SortedVectorRange.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  class CKFCDCToVXDTreeSearchFindlet : public TrackFindingCDC::TreeSearchFindlet<RecoTrack, SpacePoint, CKFCDCToVXDStateObject> {
  public:
    using Super = TrackFindingCDC::TreeSearchFindlet<RecoTrack, SpacePoint, CKFCDCToVXDStateObject>;

    CKFCDCToVXDTreeSearchFindlet() : Super()
    {
      addProcessingSignalListener(&m_hitFilter);
    }

    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      Super::exposeParameters(moduleParamList, prefix);

      m_hitFilter.exposeParameters(moduleParamList, prefix);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumXYNorm"), m_param_maximumXYNorm,
                                    "", m_param_maximumXYNorm);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "checkDistanceWithoutExtrapolation"),
                                    m_param_checkDistanceWithoutExtrapolation,
                                    "", m_param_checkDistanceWithoutExtrapolation);
    }

    void apply(std::vector<RecoTrack*>& seedsVector,
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
      }
    }

  protected:
    TrackFindingCDC::SortedVectorRange<const SpacePoint*> getMatchingHits(Super::StateIterator currentState) final {
      const unsigned int nextLayer = currentState->getLastLayer() - 1;
      return m_cachedHitMap[nextLayer];
    }

    bool useResult(Super::StateIterator currentState) final {
      if (m_param_checkDistanceWithoutExtrapolation and not checkDistanceWithoutExtrapolation(currentState))
      {
        return false;
      }

      currentState->advance();

      TrackFindingCDC::Weight weight = m_hitFilter(*currentState);
      return not std::isnan(weight);
    }

  private:
    /// Cache
    std::map<unsigned int, TrackFindingCDC::SortedVectorRange<const SpacePoint*>> m_cachedHitMap;

    /// Subfindlet: Filter
    TrackFindingCDC::ChooseableFilter<CDCTrackSpacePointCombinationFilterFactory> m_hitFilter;

    double m_param_maximumXYNorm = 1;
    bool m_param_checkDistanceWithoutExtrapolation = true;

    bool checkDistanceWithoutExtrapolation(Super::StateIterator currentState)
    {
      // Simple filtering based on xy distance
      // TODO: Move in own filter
      RecoTrack* cdcTrack = currentState->getSeedRecoTrack();
      const SpacePoint* spacePoint = currentState->getSpacePoint();

      TrackFindingCDC::Vector3D position;
      TrackFindingCDC::Vector3D momentum;

      if (not cdcTrack->wasFitSuccessful()) {
        return false;
      }

      const auto& firstMeasurement = currentState->getMeasuredStateOnPlane();
      position = TrackFindingCDC::Vector3D(firstMeasurement.getPos());
      momentum = TrackFindingCDC::Vector3D(firstMeasurement.getMom());

      const TrackFindingCDC::CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed());

      const auto& hitPosition = TrackFindingCDC::Vector3D(spacePoint->getPosition());

      const double arcLength = trajectory.calcArcLength2D(hitPosition);
      const auto& trackPositionAtHit2D = trajectory.getTrajectory2D().getPos2DAtArcLength2D(arcLength);
      const auto& trackPositionAtHitZ = trajectory.getTrajectorySZ().mapSToZ(arcLength);

      TrackFindingCDC::Vector3D trackPositionAtHit(trackPositionAtHit2D, trackPositionAtHitZ);
      TrackFindingCDC::Vector3D distance = trackPositionAtHit - hitPosition;

      return distance.xy().norm() < m_param_maximumXYNorm;
    }
  };
}