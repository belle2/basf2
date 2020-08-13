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

#include <genfit/MeasuredStateOnPlane.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>

#include <boost/optional.hpp>

namespace Belle2 {

  /// Define states for CKF algorithm, which can be seed track or CDC wire hit.
  class CDCCKFState {
  public:
    /// constructor from the seed recoTrack and genfit trackState
    CDCCKFState(const RecoTrack* seed, const genfit::MeasuredStateOnPlane& trackState) :
      m_seed(seed), m_trackState(trackState) {}

    /// constructor from the CDC wireHit
    explicit CDCCKFState(const TrackFindingCDC::CDCWireHit* wireHit) : m_cdcWireHit(wireHit) {}

    /// Get CDCWireHit corresponding to the state
    const TrackFindingCDC::CDCWireHit* getWireHit() const
    {
      B2ASSERT("State does not represent a wire hit", static_cast<bool>(m_cdcWireHit));
      return *m_cdcWireHit;
    }

    /// Get RecoTrack seed corresponding to the state
    const RecoTrack* getSeed() const
    {
      B2ASSERT("State does not represent a seed", static_cast<bool>(m_seed));
      return *m_seed;
    }

    /// Match seed to the MC track, return it. Works for seed-states only
    const RecoTrack* getMCRecoTrack(const std::string& mcRecoTrackStoreArrayName = "MCRecoTracks") const
    {
      const RecoTrack* seed = getSeed();
      return seed->getRelated<RecoTrack>(mcRecoTrackStoreArrayName);
    }

    /// Returns true if the state corresponds to the seed track
    bool isSeed() const
    {
      return static_cast<bool>(m_seed);
    }

    /// Get genfit track state (but first check if already present)
    const genfit::MeasuredStateOnPlane& getTrackState() const
    {
      B2ASSERT("State does not have a track state (yet)", static_cast<bool>(m_trackState));
      return *m_trackState;
    }

    /// Store genfit Measured state on plane
    void setTrackState(const genfit::MeasuredStateOnPlane& trackState)
    {
      m_trackState = trackState;
    }

    /// Set right-left info for the hit
    void setRLinfo(const TrackFindingCDC::ERightLeft& rl)
    {
      m_rl = rl;
    }

    /// Return right-left info (check if present first)
    TrackFindingCDC::ERightLeft getRLinfo() const
    {
      B2ASSERT("LR info is not present yet", m_rl != TrackFindingCDC::ERightLeft::c_Unknown);
      return m_rl;
    }

    /// Set the arc-length along the tracjectory to the hit
    void setArcLength(double arcLength)
    {
      m_arcLength = arcLength;
    }

    /// Return the arc-length along the tracjectory to the hit
    double getArcLength() const
    {
      return m_arcLength;
    }

    /// Set hit distance to the trajectory
    void setHitDistance(double hitDistance)
    {
      m_hitDistance = hitDistance;
    }

    /// Return hit distance to the trajectory
    double getHitDistance() const
    {
      return m_hitDistance;
    }

    /// Set state weight
    void setWeight(double weight)
    {
      m_weight = weight;
    }

    /// Get state weight
    double getWeight() const
    {
      return m_weight;
    }

    /// Set set chi2
    void setChi2(double chi2)
    {
      m_chi2 = chi2;
    }

    /// Get state chi2
    double getChi2() const
    {
      return m_chi2;
    }

    /// Set state Z information
    void setReconstructedZ(double reconstructedZ)
    {
      m_reconstructedZ = reconstructedZ;
    }

    /// Get state Z information
    double getReconstructedZ() const
    {
      return m_reconstructedZ;
    }

    /// Helper method to get trajectory from the trackState.
    TrackFindingCDC::CDCTrajectory3D getTrajectory() const
    {
      const auto& trackState = getTrackState();
      const TrackFindingCDC::Vector3D trackPosition(trackState.getPos());
      const TrackFindingCDC::Vector3D trackMomentum(trackState.getMom());
      return TrackFindingCDC::CDCTrajectory3D(trackPosition, trackState.getTime(),
                                              trackMomentum, trackState.getCharge());
    }

  private:
    /// (optional) pointer to the seed track
    boost::optional<const RecoTrack*> m_seed;
    /// (optional) pointer to the wire hit
    boost::optional<const TrackFindingCDC::CDCWireHit*> m_cdcWireHit;

    /// (optional) genfit MeasuredStateOnPlane
    boost::optional<genfit::MeasuredStateOnPlane> m_trackState;

    /// arc length along the trajectory to the hit
    double m_arcLength = 0;

    /// distance from the trajectory to the hit
    double m_hitDistance = 0;

    /// state weight
    double m_weight = 0;

    /// state chi2 (using genfit extrapolation)
    double m_chi2 = 0;

    /// reconstructed Z coordinate
    double m_reconstructedZ = 0;

    /// Store if the track is on the right or left side of the hit
    TrackFindingCDC::ERightLeft m_rl = TrackFindingCDC::ERightLeft::c_Unknown;
  };

  /// print state info
  std::ostream& operator<<(std::ostream& output, const CDCCKFState& state);
}
