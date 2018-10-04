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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/numerics/WeightComperator.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>

#include <cdc/dataobjects/CDCRecoHit.h>

#include <tracking/ckf/general/utilities/KalmanStepper.h>
#include <tracking/ckf/general/utilities/Advancer.h>
#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  class CDCCKFStateFilter : public TrackFindingCDC::Findlet<const CDCCKFState, CDCCKFState> {
  public:
    CDCCKFStateFilter()
    {
      addProcessingSignalListener(&m_extrapolator);
    }


    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalHitCandidates"),
                                    m_maximalHitCandidates, "Maximal hit candidates to test",
                                    m_maximalHitCandidates);
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalHitDistance"),
                                    m_maximalHitDistance, "Maximal hit distance to allow",
                                    m_maximalHitDistance);

      m_extrapolator.exposeParameters(moduleParamList, prefix);
    }

    void apply(const CDCCKFPath& path, std::vector<CDCCKFState>& nextStates) override
    {
      const CDCCKFState& lastState = path.back();
      const TrackFindingCDC::CDCTrajectory3D& trajectory = lastState.getTrajectory();

      for (CDCCKFState& nextState : nextStates) {
        // Erase all previous weight information
        nextState.setWeight(0);

        // Do a reconstruction based on the helix extrapolation from the last hit
        reconstruct(nextState, trajectory, lastState.getArcLength());
        if (not roughHitSelection(nextState, lastState)) {
          nextState.setWeight(NAN);
          continue;
        }

        // Extrapolate and update
        const genfit::MeasuredStateOnPlane& lastMSoP = lastState.getTrackState();
        if (not extrapolateAndUpdate(nextState, lastMSoP)) {
          nextState.setWeight(NAN);
          continue;
        }

        // Do a final hit selection based on the new state
        const TrackFindingCDC::CDCTrajectory3D& thisTrajectory = nextState.getTrajectory();
        reconstruct(nextState, thisTrajectory, nextState.getArcLength());

        // TODO: set a weight somehow with an MVA
        nextState.setWeight(1 / std::abs(nextState.getHitDistance()));
      }

      B2DEBUG(100, "Starting with " << nextStates.size() << " possible hits");

      TrackFindingCDC::erase_remove_if(nextStates,
                                       TrackFindingCDC::Composition<TrackFindingCDC::IsNaN, TrackFindingCDC::GetWeight>());

      B2DEBUG(100, "Now have " << nextStates.size());

      std::sort(nextStates.begin(), nextStates.end(), TrackFindingCDC::GreaterWeight());

      TrackFindingCDC::only_best_N(nextStates, m_maximalHitCandidates);
    }

  private:
    size_t m_maximalHitCandidates = 4;
    double m_maximalHitDistance = 2;

    KalmanStepper<1> m_updater;
    Advancer m_extrapolator;

    void reconstruct(CDCCKFState& state, const TrackFindingCDC::CDCTrajectory3D& trajectory, const double lastArcLength) const
    {
      // TODO: actually we do not need to do any trajectory creation here. We could save some computing time!
      const TrackFindingCDC::CDCTrajectory2D& trajectory2D = trajectory.getTrajectory2D();
      const TrackFindingCDC::CDCTrajectorySZ& trajectorySZ = trajectory.getTrajectorySZ();

      const TrackFindingCDC::CDCWireHit* wireHit = state.getWireHit();

      TrackFindingCDC::Vector2D recoPos2D;
      if (wireHit->isAxial()) {
        recoPos2D = wireHit->reconstruct2D(trajectory2D);
      } else {
        const TrackFindingCDC::CDCWire& wire = wireHit->getWire();
        const TrackFindingCDC::Vector2D& posOnXYPlane = wireHit->reconstruct2D(trajectory2D);

        const double arcLength = trajectory2D.calcArcLength2D(posOnXYPlane);
        const double z = trajectorySZ.mapSToZ(arcLength);

        const TrackFindingCDC::Vector2D& wirePos2DAtZ = wire.getWirePos2DAtZ(z);

        const TrackFindingCDC::Vector2D& recoPosOnTrajectory = trajectory2D.getClosest(wirePos2DAtZ);
        const double driftLength = wireHit->getRefDriftLength();
        TrackFindingCDC::Vector2D disp2D = recoPosOnTrajectory - wirePos2DAtZ;
        disp2D.normalizeTo(driftLength);
        recoPos2D = wirePos2DAtZ + disp2D;
      }

      const double arcLength = trajectory2D.calcArcLength2D(recoPos2D);
      const double distanceToHit = trajectory2D.getDist2D(recoPos2D);

      state.setArcLength(lastArcLength + arcLength);
      state.setHitDistance(distanceToHit);
    }

    bool extrapolateAndUpdate(CDCCKFState& state, const genfit::MeasuredStateOnPlane& lastMSoP) const
    {
      genfit::MeasuredStateOnPlane mSoP = lastMSoP;

      const TrackFindingCDC::CDCWireHit* wireHit = state.getWireHit();
      CDCRecoHit recoHit(wireHit->getHit(), nullptr);

      try {
        const auto& plane = recoHit.constructPlane(lastMSoP);
        if (std::isnan(m_extrapolator.extrapolateToPlane(mSoP, plane))) {
          return false;
        }

        const auto& measurements = recoHit.constructMeasurementsOnPlane(mSoP);
        B2ASSERT("Should be exactly two measurements", measurements.size() == 2);

        const auto rightLeft = static_cast<TrackFindingCDC::ERightLeft>(TrackFindingCDC::sign(
                                 state.getHitDistance()));
        if (rightLeft == TrackFindingCDC::ERightLeft::c_Right) {
          B2INFO(m_updater.kalmanStep(mSoP, *(measurements[1])));
        } else {
          B2INFO(m_updater.kalmanStep(mSoP, *(measurements[0])));
        }

        delete measurements[0];
        delete measurements[1];

        state.setTrackState(mSoP);
        return true;
      } catch (genfit::Exception) {
        return false;
      }
    }

    bool roughHitSelection(CDCCKFState& state, const CDCCKFState& lastState) const
    {
      const double& arcLength = state.getArcLength() - lastState.getArcLength();
      // TODO: magic number
      if (arcLength <= 0 or arcLength > 20) {
        return false;
      }

      const double& hitDistance = state.getHitDistance();
      // TODO: magic number
      if (std::abs(hitDistance) > m_maximalHitDistance) {
        return false;
      }

      return true;
    };
  };
}
