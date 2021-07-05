/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/numerics/WeightComperator.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>
#include <tracking/ckf/cdc/filters/states/CDCStateFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  /// A stack of pre-, helix-extrapolation- , Kalman-extrapolation- and Kalman-update-filters.
  class CDCCKFStateFilter : public TrackFindingCDC::Findlet<const CDCCKFState, CDCCKFState> {
  public:
    /// Add all sub findlets
    CDCCKFStateFilter()
    {
      addProcessingSignalListener(&m_preFilter);
      addProcessingSignalListener(&m_basicFilter);
      addProcessingSignalListener(&m_extrapolationFilter);
      addProcessingSignalListener(&m_finalSelection);
    }


    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalHitCandidates"),
                                    m_maximalHitCandidates, "Maximal hit candidates to test",
                                    m_maximalHitCandidates);
      m_preFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "pre"));
      m_basicFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "basic"));
      m_extrapolationFilter.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "extrapolation"));
      m_finalSelection.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "final"));
    }

    /// Apply the findlet and do the state selection
    void apply(const CDCCKFPath& path, std::vector<CDCCKFState>& nextStates) override
    {
      const CDCCKFState& lastState = path.back();
      const TrackFindingCDC::CDCTrajectory3D& trajectory = lastState.getTrajectory();

      TrackFindingCDC::Weight weight;

      B2DEBUG(29, "On layer: " << (lastState.isSeed() ? -1 : lastState.getWireHit()->getWire().getICLayer()));

      for (CDCCKFState& nextState : nextStates) {
        B2DEBUG(29, "Checking layer: " << nextState.getWireHit()->getWire().getICLayer());

        weight = m_preFilter({&path, &nextState});
        nextState.setWeight(weight);
        if (std::isnan(weight)) {
          B2DEBUG(29, "Fails PreFilter");
          continue;
        }

        // Do a reconstruction based on the helix extrapolation from the last hit
        reconstruct(nextState, trajectory, lastState.getArcLength());

        weight = m_basicFilter({&path, &nextState});
        nextState.setWeight(weight);
        if (std::isnan(weight)) {
          B2DEBUG(29, "Fails BasicFilter");
          continue;
        }

        // Extrapolate and update
        weight = m_extrapolationFilter({&path, &nextState});
        nextState.setWeight(weight);
        if (std::isnan(weight)) {
          B2DEBUG(29, "Fails ExtrapolationFilter");
          continue;
        }

        // Do a final hit selection based on the new state
        const TrackFindingCDC::CDCTrajectory3D& thisTrajectory = nextState.getTrajectory();
        reconstruct(nextState, thisTrajectory, nextState.getArcLength());

        weight = m_finalSelection({&path, &nextState});
        nextState.setWeight(weight);
        if (std::isnan(weight)) {
          B2DEBUG(29, "Fails FinalFilter");
          continue;
        }
      }

      B2DEBUG(29, "Starting with " << nextStates.size() << " possible hits");

      TrackFindingCDC::erase_remove_if(nextStates,
                                       TrackFindingCDC::Composition<TrackFindingCDC::IsNaN, TrackFindingCDC::GetWeight>());

      B2DEBUG(29, "Now have " << nextStates.size());

      std::sort(nextStates.begin(), nextStates.end(), TrackFindingCDC::GreaterWeight());

      TrackFindingCDC::only_best_N(nextStates, m_maximalHitCandidates);
    }

  private:
    /// Parameter: max number of candidates
    size_t m_maximalHitCandidates = 4;
    /// Pre Filter
    TrackFindingCDC::ChooseableFilter<CDCStateFilterFactory> m_preFilter;
    /// Basic Filter (uses helix extrapolation)
    TrackFindingCDC::ChooseableFilter<CDCStateFilterFactory> m_basicFilter;
    /// Extrapolation Filter  (after Kalman extrapolation)
    TrackFindingCDC::ChooseableFilter<CDCStateFilterFactory> m_extrapolationFilter;
    /// Final Selection Filter (after Kalman update)
    TrackFindingCDC::ChooseableFilter<CDCStateFilterFactory> m_finalSelection;

    /// Helper function to reconstruct the arc length and the hit distance of a state according to the trajectory
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
      const double z = trajectorySZ.mapSToZ(arcLength);
      const double distanceToHit = trajectory2D.getDist2D(recoPos2D);

      state.setArcLength(lastArcLength + arcLength);
      state.setHitDistance(distanceToHit);
      state.setReconstructedZ(z);
    }
  };
}
