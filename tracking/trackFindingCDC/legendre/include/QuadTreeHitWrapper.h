/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Nils Braun                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#ifndef __CINT__
#include <tuple>
#endif

#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>

namespace Belle2 {
  class CDCHit;

  namespace TrackFindingCDC {

    class CDCWireHit;

    /** CDC Hit Class used for pattern recognition in the Legendre plain. */
    class QuadTreeHitWrapper {
    public:

      /** For root: */
      QuadTreeHitWrapper() : m_cdcWireHit(nullptr), m_conformalPosition(),
        m_conformalDriftLength() { }

      /** Constructor to create a TrackHit from a CDCWireHit object.
       * Some member variables of CDCHit are copied and other to CDCTrackHit specific variables are initialized
       * (e.g. the position of the hit wire in normal space and in the conformal plane).
       */
      explicit QuadTreeHitWrapper(const CDCWireHit* wireHit)
      {
        initializeFromWireHit(wireHit);
      }

      /** Returns the Hit position (X coordinate) in the conformal plane.*/
      inline double getConformalX() const { return m_conformalPosition.x(); }

      /** Returns the Hit position (Y coordinate) in the conformal plane.*/
      inline double getConformalY() const { return m_conformalPosition.y(); }

      /** Returns the drift time in the conformal plane (with r << x,y).*/
      inline double getConformalDriftLength() const { return m_conformalDriftLength; }

      const CDCWireHit* getCDCWireHit() const { return m_cdcWireHit; }

      void setUsedFlag(bool flag) { m_cdcWireHit->getAutomatonCell().setTakenFlag(flag); };

      void setMaskedFlag(bool flag) { m_cdcWireHit->getAutomatonCell().setMaskedFlag(flag); };

      bool getUsedFlag() const {return m_cdcWireHit->getAutomatonCell().hasTakenFlag();};

      bool getMaskedFlag() const {return m_cdcWireHit->getAutomatonCell().hasMaskedFlag();};

      /** Check hit drift lenght; if it's greater than cell size return false */
      bool checkHitDriftLength();

#ifndef __CINT__
      /** Calculate conformal coordinates with respect to choosen point by transforming the wire coordinates. Returns (x',y',driftLength) */
      std::tuple<double, double, double> performConformalTransformWithRespectToPoint(double x0, double y0);
#endif


    private:
      /** Returns the phi angle of the center wire position.
        * From the Hit position (= center of the wire) the angle is calculated so that it goes from 0 to 2*pi.
        * With the cases:
        *   x > 0, y > 0: phi in 0, pi/2
        *   x < 0, y > 0: phi in pi/2, pi
        *   x < 0, y < 0: phi in pi, 3/2 pi
        *   x > 0, y < 0: phi in 3/2 pi, 2 pi
        */

      /** Set all parameters from the given wire hit */
      void initializeFromWireHit(const CDCWireHit* wireHit);

      const CDCWireHit* m_cdcWireHit;

      /** Assigns values for conformal coordinates by transforming the wire coordinates. */
      void performConformalTransformation();

      Vector2D m_conformalPosition;      /**< Position in the conformal space of the drift center (not the wire!) */
      double m_conformalDriftLength;     /**< Drift time of the hit in the conformal plane (under the assumption that r << x,y*/

    }; //end class CDCTrackHit
  } //end namespace TrackFindingCDC
} //end namespace Belle2


