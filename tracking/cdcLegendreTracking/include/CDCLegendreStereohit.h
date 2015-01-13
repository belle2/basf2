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


#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

#include "TVector3.h"
#include "TVector2.h"

#include <list>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include "TMath.h"

namespace Belle2 {
  namespace TrackFinderCDCLegendre {

    class TrackHit;
    class TrackCandidate;

    class StereoHit {
    public:

      StereoHit(): m_aplha(0), m_posX(0), m_posY(0), m_InnerOuter(0), m_displacement(0), m_theta(0), m_hit(0) {};

      StereoHit(double alpha, double posX, double posY, int InnerOuter, TrackHit* hit, double displacement):
        m_aplha(alpha), m_posX(posX), m_posY(posY), m_InnerOuter(InnerOuter), m_displacement(displacement), m_theta(0), m_hit(hit) {};


      /** Set alpha of hit */
      void setAlpha(double alpha) { m_aplha = alpha; };

      /** Set alpha of hit */
      void setPos(std::pair<double, double> pos) { m_posX = pos.first; m_posY = pos.second; };

      /** Set alpha of hit */
      void setInnerOuter(int InnerOuter) { m_InnerOuter = InnerOuter; };

      /** Set displacement of the hit on XY plane */
      void setDisplacement(int InnerOuter) { m_InnerOuter = InnerOuter; };

      void setPolarAngle(double theta) { m_theta = theta; };

      inline double getAlpha() const { return m_aplha; };

      inline std::pair<double, double> getPos() const { return std::make_pair(m_posX, m_posY); };

      inline int getInnerOuter() const { return m_InnerOuter; };

      inline TrackHit* getTrackHit() const { return m_hit; };

      inline double getDisplacement() const { return m_displacement; };

      inline double getPolarAngle() const { return m_theta; };

    private:

      double m_aplha;         // position on the track expressed in rads
      double m_posX;          // X position on the track
      double m_posY;          // Y position on the track
      int m_InnerOuter;       // inner or outer hit (L/R); -1 - inner, +1 - outer
      double m_displacement;  // position on the track expressed in rads
      double m_theta;         // polar angle

      TrackHit* m_hit;        // Holds pointer to TrackHit object;

    };
  }
}
