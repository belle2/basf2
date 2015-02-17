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
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackHit.h>

#include "TVector3.h"
#include "TVector2.h"

#include <list>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include "TMath.h"

namespace Belle2 {
  namespace TrackFindingCDC {

    class TrackHit;
    class TrackCandidate;

    class StereoHit {
    public:

      StereoHit(): m_aplha(-999), m_posX(-999), m_posY(-999), m_InnerOuter(0), m_displacement(-999), m_lWire(-999), m_rWire(-999),
        m_sign_final(-999), m_Rcand(-999), m_theta(0), m_Z0(-999), m_hit(0) {};

      StereoHit(double alpha, double posX, double posY, int InnerOuter, TrackHit* hit, double displacement):
        m_aplha(alpha), m_posX(posX), m_posY(posY), m_InnerOuter(InnerOuter), m_displacement(displacement), m_lWire(-999), m_rWire(-999),
        m_sign_final(-999), m_Rcand(-999), m_theta(0), m_Z0(-999), m_hit(hit) {};


      /** Set alpha of hit */
      void setAlpha(double alpha) { m_aplha = alpha; };

      /** Set alpha of hit */
      void setPos(std::pair<double, double> pos) { m_posX = pos.first; m_posY = pos.second; };

      /** Set alpha of hit */
      void setInnerOuter(int InnerOuter) { m_InnerOuter = InnerOuter; };

      /** Set displacement of the hit on XY plane */
      void setDisplacement(double displacement) { m_displacement = displacement; };

      void setLWire(double lWire) { m_lWire = lWire; };

      void setRWire(double rWire) { m_rWire = rWire; };

      void setSign(double sign_final) { m_sign_final = sign_final; };

      void setRcand(double Rcand) { m_Rcand = Rcand; };

      void setPolarAngle(double theta) { m_theta = theta; };

      void setZ0(double Z0) { m_Z0 = Z0; };

      inline double getAlpha() const { return m_aplha; };

      inline std::pair<double, double> getPos() const { return std::make_pair(m_posX, m_posY); };

      inline int getInnerOuter() const { return m_InnerOuter; };

      inline TrackHit* getTrackHit() const { return m_hit; };

      inline double getDisplacement() const { return m_displacement; };

      inline double getLWire() const { return m_lWire; };

      inline double getRWire() const { return m_rWire; };

      inline double getSign() const { return m_sign_final; };

      inline double getRcand() const { return m_Rcand; };

      inline double getPolarAngle() const { return m_theta; };

      inline double getZ0() const { return m_Z0; };

      double computePolarAngle() {
        assert(m_lWire != -999);
        assert(m_rWire != -999);
        assert(m_sign_final != -999);
        assert(m_displacement != -999);
        assert(m_Z0 != -999);
        assert(m_aplha != -999);
        assert(m_Rcand != -999);

        m_theta = atan2(m_lWire * m_sign_final * m_displacement - m_Z0 * m_rWire , m_aplha * m_Rcand * m_rWire); // + 3.1415 / 2.;
        return tan(m_theta);
      }

    private:

      double m_aplha;         // position on the track expressed in rads
      double m_posX;          // X position on the track
      double m_posY;          // Y position on the track
      int m_InnerOuter;       // inner or outer hit (L/R); -1 - inner, +1 - outer
      double m_displacement;  // position on the track expressed in rads
      double m_lWire;         // polar angle
      double m_rWire;         // polar angle
      double m_sign_final;    // polar angle
      double m_Rcand;         // polar angle
      double m_theta;         // polar angle
      double m_Z0;            // polar angle

      TrackHit* m_hit;        // Holds pointer to TrackHit object;

    };
  }
}
