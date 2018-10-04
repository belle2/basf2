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

namespace Belle2 {
  class CDCCKFStateFilter : public TrackFindingCDC::Findlet<const CDCCKFState, CDCCKFState> {
  public:
    CDCCKFStateFilter()
    {
      addProcessingSignalListener(&m_extrapolator);
    }

    void apply(const CDCCKFPath& path, std::vector<CDCCKFState>& nextStates) override
    {
      const CDCCKFState& lastState = path.back();
      const TrackFindingCDC::CDCTrajectory3D& trajectory = lastState.getTrajectory();

      for (CDCCKFState& nextState : nextStates) {
        // Erase all previous weight information
        nextState.setWeight(0);

        // Do a reconstruction based on the helix extrapolation from the last hit
        if (not roughHitSelection(nextState, trajectory)) {
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
        if (not roughHitSelection(nextState, thisTrajectory)) {
          nextState.setWeight(NAN);
          continue;
        }

        // TODO: set a weight somehow with an MVA
        nextState.setWeight(1 / std::abs(nextState.getHitDistance()));
      }

      TrackFindingCDC::erase_remove_if(nextStates,
                                       TrackFindingCDC::Composition<TrackFindingCDC::IsNaN, TrackFindingCDC::GetWeight>());
      std::sort(nextStates.begin(), nextStates.end(), TrackFindingCDC::GreaterWeight());

      TrackFindingCDC::only_best_N(nextStates, m_maximalHitCandidates);
    }

  private:
    size_t m_maximalHitCandidates = 5;
    KalmanStepper<1> m_updater;
    Advancer m_extrapolator;

    void reconstruct(CDCCKFState& state, const TrackFindingCDC::CDCTrajectory3D& trajectory) const
    {
      // TODO: actually we do not need to do any trajectory creation here. We could save some computing time!
      const TrackFindingCDC::CDCTrajectory2D& trajectory2D = trajectory.getTrajectory2D();
      const TrackFindingCDC::CDCTrajectorySZ& trajectorySZ = trajectory.getTrajectorySZ();

      const TrackFindingCDC::CDCWireHit* wireHit = state.getWireHit();

      // First step: get the position on the drift circle closest to the trajectory
      if (wireHit->isAxial()) {
        // Mostly a copy from CDCRLWireHit
        const TrackFindingCDC::Vector2D& refPos2D = wireHit->getRefPos2D();
        const TrackFindingCDC::Vector2D recoPos2D = trajectory2D.getClosest(refPos2D);

        // Fix the displacement to lie on the drift circle.
        const double driftLength = wireHit->getRefDriftLength();
        TrackFindingCDC::Vector2D disp2D = recoPos2D - refPos2D;
        // Fix the displacement to lie on the drift circle.
        disp2D.normalizeTo(driftLength);
        const TrackFindingCDC::Vector2D recoPos2DOnCircle = refPos2D + disp2D;

        const double distanceToHit = trajectory2D.getDist2D(recoPos2DOnCircle);
        const double arcLength = trajectory2D.calcArcLength2D(recoPos2D);

        state.setArcLength(arcLength);
        state.setHitDistance(distanceToHit);
      } else {
        // TODO
      }
    }

    bool extrapolateAndUpdate(CDCCKFState& state, const genfit::MeasuredStateOnPlane& lastMSoP) const
    {
      genfit::MeasuredStateOnPlane mSoP = lastMSoP;

      const TrackFindingCDC::CDCWireHit* wireHit = state.getWireHit();
      CDCRecoHit recoHit(wireHit->getHit(), nullptr);

      const auto& plane = recoHit.constructPlane(lastMSoP);
      if (std::isnan(m_extrapolator.extrapolateToPlane(mSoP, plane))) {
        return false;
      }

      const auto& measurements = recoHit.constructMeasurementsOnPlane(mSoP);
      B2ASSERT("Should be exactly two measurements", measurements.size() == 2);

      const auto rightLeft = static_cast<TrackFindingCDC::ERightLeft>(TrackFindingCDC::sign(state.getHitDistance()));
      if (rightLeft == TrackFindingCDC::ERightLeft::c_Right) {
        m_updater.kalmanStep(mSoP, *(measurements[1]));
      } else {
        m_updater.kalmanStep(mSoP, *(measurements[0]));
      }

      delete measurements[0];
      delete measurements[1];

      state.setTrackState(mSoP);
      return true;
    }

    bool roughHitSelection(CDCCKFState& state, const TrackFindingCDC::CDCTrajectory3D& trajectory) const
    {
      reconstruct(state, trajectory);

      const double& arcLength = state.getArcLength();
      // TODO: magic number
      if (arcLength <= 0 or arcLength > 20) {
        return false;
      }

      const double& hitDistance = state.getHitDistance();
      // TODO: magic number
      if (std::abs(hitDistance) > 2) {
        return false;
      }

      return true;
    };
  };
}
