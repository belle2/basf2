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
      m_hitFilter.exposeParameters(moduleParamList, prefix);

      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumXYNorm"), m_param_maximumXYNorm,
                                    "", m_param_maximumXYNorm);
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
        B2INFO("Storing in " << layerID << " " << std::distance(first, last));
      }
    }

  protected:
    TrackFindingCDC::SortedVectorRange<const SpacePoint*> getMatchingHits(Super::StateIterator currentState) final {
      const unsigned int nextLayer = currentState->getLastLayer() - 1;
      const auto& hitsOnNextLayer = m_cachedHitMap[nextLayer];
      return hitsOnNextLayer;
      /*matchingHits.reserve(hitsOnNextLayer.size());

      for(const SpacePoint* spacePoint : hitsOnNextLayer) {
        const auto& weight = m_hitFilter(std::make_pair(currentResult, spacePoint));
        if(not std::isnan(weight)) {
          matchingHits.push_back(spacePoint);
        }
      }*/
    }

    bool useResult(Super::StateIterator currentState) final {
      // Simple filtering based on xy distance
      // TODO: Move in own filter
      RecoTrack* cdcTrack = currentState->getSeedRecoTrack();
      const SpacePoint* spacePoint = currentState->getSpacePoint();

      TrackFindingCDC::Vector3D position;
      TrackFindingCDC::Vector3D momentum;

      if (cdcTrack->wasFitSuccessful())
      {
        const auto& firstMeasurement = cdcTrack->getMeasuredStateOnPlaneFromFirstHit();
        position = TrackFindingCDC::Vector3D(firstMeasurement.getPos());
        momentum = TrackFindingCDC::Vector3D(firstMeasurement.getMom());
      } else {
        position = TrackFindingCDC::Vector3D(cdcTrack->getPositionSeed());
        momentum = TrackFindingCDC::Vector3D(cdcTrack->getMomentumSeed());
      }

      const TrackFindingCDC::CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed());

      const auto& hitPosition = TrackFindingCDC::Vector3D(spacePoint->getPosition());

      const double arcLength = trajectory.calcArcLength2D(hitPosition);
      const auto& trackPositionAtHit2D = trajectory.getTrajectory2D().getPos2DAtArcLength2D(arcLength);
      const auto& trackPositionAtHitZ = trajectory.getTrajectorySZ().mapSToZ(arcLength);

      TrackFindingCDC::Vector3D trackPositionAtHit(trackPositionAtHit2D, trackPositionAtHitZ);
      TrackFindingCDC::Vector3D distance = trackPositionAtHit - hitPosition;

      if (distance.xy().norm() > m_param_maximumXYNorm)
      {
        return false;
      }

      TrackFindingCDC::Weight weight = m_hitFilter(*currentState);
      return not std::isnan(weight);
    }

  private:
    /// Cache
    std::map<unsigned int, TrackFindingCDC::SortedVectorRange<const SpacePoint*>> m_cachedHitMap;

    /// Subfindlet: Filter
    TrackFindingCDC::ChooseableFilter<CDCTrackSpacePointCombinationFilterFactory> m_hitFilter;

    double m_param_maximumXYNorm = 1;
  };
}