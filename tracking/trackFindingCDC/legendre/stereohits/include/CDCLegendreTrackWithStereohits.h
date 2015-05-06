/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/stereohits/CDCLegendreStereohit.h>


namespace genfit {
  class TrackCand;
}

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCTrack;
    class TrackHit;


    class TrackCandidateWithStereoHits : public TrackCandidate {
    public:

      TrackCandidateWithStereoHits(double theta, double r, int charge, const std::vector<TrackHit*>& trackHitList,
                                   genfit::TrackCand* trackCand):
        TrackCandidate(theta, r, charge, trackHitList), m_trackCand(trackCand), m_cdcTrack(nullptr), m_polarAngle(0) {};

      TrackCandidateWithStereoHits(CDCTrack* oldCDCTrack);

      TrackCandidateWithStereoHits(double theta, double r, int charge, const std::vector<TrackHit*>& trackHitList,
                                   CDCTrack* trackCand):
        TrackCandidate(theta, r, charge, trackHitList), m_trackCand(nullptr), m_cdcTrack(trackCand), m_polarAngle(0) {};

      genfit::TrackCand* getGfTrackCand() const {return m_trackCand;}
      CDCTrack* getCDCTrackCand() const {return m_cdcTrack;}

      std::vector<std::pair<StereoHit, StereoHit>> getStereohits() const {return m_stereoHits;}

      double getPolarAngle() const {return m_polarAngle;};

      void setPolarAngle(double polarAngle) {m_polarAngle = polarAngle;};

      void addStereohit(std::pair<StereoHit, StereoHit> stereopair) {m_stereoHits.push_back(stereopair);}

    private:

//      double m_theta;         // polar angle
//      double m_aplha;         // position on the track expressed in rads
//      double m_displacement;  // position on the track expressed in rads
//      double m_posX;          // X position on the track
//      double m_posY;          // Y position on the track
//      int m_InnerOuter;       // inner or outer hit (L/R); -1 - inner, +1 - outer

//      std::vector<StereoHit*> m_stereoHits;       // Holds pointers of stereohits objects;
//      TrackHit* m_hit;        // Holds pointer to TrackHit object;

      genfit::TrackCand* m_trackCand;
      CDCTrack* m_cdcTrack;
      std::vector<std::pair<StereoHit, StereoHit>> m_stereoHits;
      double m_polarAngle;


    };
  }
}
