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

#include <tuple>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCWireHit;

    /**
     * CDC Hit Class used for pattern recognition in the Legendre tracking algorithm.
     * This class holds a pointer to a CDCWireHit it belongs to and adds information that are only
     * relevant to the Legendre algorithm (e.g. the conformal position or the conformal drift length).
     * These variables are precalculated and cached in this object for faster calculation times in the
     * quad tree. The flags of the wire hit are accessible through the interface also (only for convenience).
     * */
    class ConformalCDCWireHit {
    public:
      /**
       * Constructor to create a ConformalCDCWireHit from a CDCWireHit object.
       * Some member variables specific to the ConformalCDCWireHit are initialized
       * (e.g. the conformal transformed position).
       */
      explicit ConformalCDCWireHit(const CDCWireHit* wireHit);

      /** Returns the Hit position (X coordinate) in the conformal plane.*/
      inline double getConformalX() const { return m_conformalPosition.x(); }

      /** Returns the Hit position (Y coordinate) in the conformal plane.*/
      inline double getConformalY() const { return m_conformalPosition.y(); }

      /** Returns the drift time in the conformal plane (with r << x,y).*/
      inline double getConformalDriftLength() const { return m_conformalDriftLength; }

      /** Returns pointer to the underlying CDCWireHit.*/
      const CDCWireHit* getCDCWireHit() const { return m_cdcWireHit; }

      /** Sets TAKEN flag of the Automaton cell of the CDCWireHit.*/
      void setUsedFlag(bool flag) { m_cdcWireHit->getAutomatonCell().setTakenFlag(flag); };

      /** Sets MASKED flag of the Automaton cell of the CDCWireHit.*/
      void setMaskedFlag(bool flag) { m_cdcWireHit->getAutomatonCell().setMaskedFlag(flag); };

      /** Returns TAKEN flag of the Automaton cell of the CDCWireHit.*/
      bool getUsedFlag() const {return m_cdcWireHit->getAutomatonCell().hasTakenFlag();};

      /** Returns MASKED flag of the Automaton cell of the CDCWireHit.*/
      bool getMaskedFlag() const {return m_cdcWireHit->getAutomatonCell().hasMaskedFlag();};

      /**
       * Check hit drift lenght. if it's greater than the cell size return false (typically this is true for background?).
       * TODO: This is done using the CDCGeometry - better do this using the CDCWireTopology!
       * */
      bool checkHitDriftLength() const;

      /** Calculate conformal coordinates with respect to choosen point by transforming the wire coordinates. Returns (x',y',driftLength) */
      std::tuple<double, double, double> performConformalTransformWithRespectToPoint(double x0, double y0) const;

    private:
      const CDCWireHit* m_cdcWireHit;    /**< Pointer to the wire hit. */

      Vector2D m_conformalPosition;      /**< Position in the conformal space of the drift center (not the wire!) */
      double m_conformalDriftLength;     /**< Drift time of the hit in the conformal plane (under the assumption that r << x,y*/

      /** Assigns values for the conformal coordinates by transforming the wire coordinates of the contained CDCWireHit. */
      void performConformalTransformation();


    }; //end class CDCTrackHit
  } //end namespace TrackFindingCDC
} //end namespace Belle2


