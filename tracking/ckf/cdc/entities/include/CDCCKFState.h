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
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

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

  private:
    boost::optional<const RecoTrack*> m_seed;
    boost::optional<const TrackFindingCDC::CDCWireHit*> m_cdcWireHit;
    boost::optional<genfit::MeasuredStateOnPlane> m_trackState;
  };
}
