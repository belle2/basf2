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
  class CDCCKFState {
  public:
    CDCCKFState(const RecoTrack* seed, const genfit::MeasuredStateOnPlane& trackState) :
      m_seed(seed), m_trackState(trackState) {}

    CDCCKFState(const TrackFindingCDC::CDCWireHit* wireHit) : m_cdcWireHit(wireHit) {}

    const TrackFindingCDC::CDCWireHit* getWireHit() const
    {
      B2ASSERT("State does not represent a wire hit", static_cast<bool>(m_cdcWireHit));
      return *m_cdcWireHit;
    }

    const RecoTrack* getSeed() const
    {
      B2ASSERT("State does not represent a seed", static_cast<bool>(m_seed));
      return *m_seed;
    }


    bool isSeed() const
    {
      return static_cast<bool>(m_seed);
    }

    const genfit::MeasuredStateOnPlane& getTrackState() const
    {
      B2ASSERT("State does not have a track state (yet)", static_cast<bool>(m_trackState));
      return *m_trackState;
    }

    void setTrackState(const genfit::MeasuredStateOnPlane& trackState)
    {
      m_trackState = trackState;
    }

    void setRLinfo(const  TrackFindingCDC::ERightLeft rl)
    {
      m_rl = rl;
    }

    TrackFindingCDC::ERightLeft getRLinfo() const
    {
      B2ASSERT("LR info is not present yet", m_rl != TrackFindingCDC::ERightLeft::c_Unknown);
      return m_rl;
    }

    void setArcLength(double arcLength)
    {
      m_arcLength = arcLength;
    }

    double getArcLength() const
    {
      return m_arcLength;
    }

    void setHitDistance(double hitDistance)
    {
      m_hitDistance = hitDistance;
    }

    double getHitDistance() const
    {
      return m_hitDistance;
    }

    void setWeight(double weight)
    {
      m_weight = weight;
    }

    double getWeight() const
    {
      return m_weight;
    }

    void setChi2(double chi2)
    {
      m_chi2 = chi2;
    }

    double getChi2() const
    {
      return m_chi2;
    }

    TrackFindingCDC::CDCTrajectory3D getTrajectory() const
    {
      const auto& trackState = getTrackState();
      const TrackFindingCDC::Vector3D trackPosition(trackState.getPos());
      const TrackFindingCDC::Vector3D trackMomentum(trackState.getMom());
      return TrackFindingCDC::CDCTrajectory3D(trackPosition, trackState.getTime(),
                                              trackMomentum, trackState.getCharge());
    }

  private:
    boost::optional<const RecoTrack*> m_seed;
    boost::optional<const TrackFindingCDC::CDCWireHit*> m_cdcWireHit;
    boost::optional<genfit::MeasuredStateOnPlane> m_trackState;

    double m_arcLength = 0;
    double m_hitDistance = 0;
    double m_weight = 0;
    double m_chi2 = 0;
    /// Store if the track is on the right or left side of the hit
    TrackFindingCDC::ERightLeft m_rl = TrackFindingCDC::ERightLeft::c_Unknown;
  };


  std::ostream& operator<<(std::ostream& output, const CDCCKFState& state);
}
