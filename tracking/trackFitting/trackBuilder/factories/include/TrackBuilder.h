/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Markus Prim                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>
#include <TVector3.h>

namespace Belle2 {

  class Track;
  class RecoTrack;
  class TrackFitResult;
  class HitPatternVXD;
  class HitPatternCDC;

  class TrackBuilder {
  public:
    TrackBuilder(
      const std::string& trackColName,
      const std::string& trackFitResultColName,
      const std::string& mcParticleColName,
      const TVector3& beamSpot = TVector3(0., 0., 0.),
      const TVector3& beamAxis = TVector3(0., 0., 1.)
    ) :
      m_trackColName(trackColName),
      m_trackFitResultColName(trackFitResultColName),
      m_mcParticleColName(mcParticleColName),
      m_beamSpot(beamSpot),
      m_beamAxis(beamAxis)
    {};

    bool storeTrackFromRecoTrack(const RecoTrack& recoTrack);

  private:
    std::string m_trackColName;
    std::string m_trackFitResultColName;
    std::string m_mcParticleColName;
    TVector3 m_beamSpot;
    TVector3 m_beamAxis;

    uint32_t getHitPatternVXDInitializer(const RecoTrack& recoTrack) const;
    uint64_t getHitPatternCDCInitializer(const RecoTrack& recoTrack) const;
  };

}
