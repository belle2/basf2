/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/modules/cdcToVXDExtrapolator/BaseCDCTrackSpacePointCombinationFilter.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class SimpleCDCToVXDExtrapolationFilter : public BaseCDCTrackSpacePointCombinationFilter {
    public:
      void exposeParameters(ModuleParamList* moduleParamList,
                            const std::string& prefix) final {
        moduleParamList->addParameter(prefixed(prefix, "maximumXYNorm"),
        m_param_maximumXYNorm, "", m_param_maximumXYNorm);
      }

      /// Checks if a pair of axial segments is a good combination
      Weight operator()(const BaseCDCTrackSpacePointCombinationFilter::Object& currentState) final {
        RecoTrack* cdcTrack = currentState.getSeedRecoTrack();
        const SpacePoint* spacePoint = currentState.getSpacePoint();

        if (not spacePoint)
        {
          return true;
        }

        TrackFindingCDC::Vector3D position;
        TrackFindingCDC::Vector3D momentum;

        if (not cdcTrack->wasFitSuccessful())
        {
          return false;
        }

        const auto& firstMeasurement = currentState.getMeasuredStateOnPlane();
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

    private:
      double m_param_maximumXYNorm = 2;
    };
  }
}
