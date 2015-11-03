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

    /** CDC Hit Class used for pattern recognition in the Legendre plain. */
    class ConformalCDCWireHit {
    public:
      /** Constructor to create a TrackHit from a CDCWireHit object.
       * Some member variables specific to QuadTreeHitWrapper specific variables are initialized
       * (e.g. the position of the hit wire in normal space and in the conformal plane).
       */
      explicit ConformalCDCWireHit(const CDCWireHit* wireHit)
      {
        initializeFromWireHit(wireHit);
      }

      /** Returns the Hit position (X coordinate) in the conformal plane.*/
      inline double getConformalX() const { return m_conformalPosition.x(); }

      /** Returns the Hit position (Y coordinate) in the conformal plane.*/
      inline double getConformalY() const { return m_conformalPosition.y(); }

      /** Returns the drift time in the conformal plane (with r << x,y).*/
      inline double getConformalDriftLength() const { return m_conformalDriftLength; }

      /** Returns pointer to the underlying CDCWireHit.*/
      const CDCWireHit* getCDCWireHit() const { return m_cdcWireHit; }

      /** Sets TAKEN flag of Automaton cell.*/
      void setUsedFlag(bool flag) { m_cdcWireHit->getAutomatonCell().setTakenFlag(flag); };

      /** Sets MASKED flag of Automaton cell.*/
      void setMaskedFlag(bool flag) { m_cdcWireHit->getAutomatonCell().setMaskedFlag(flag); };

      /** Returns TAKEN flag of Automaton cell.*/
      bool getUsedFlag() const {return m_cdcWireHit->getAutomatonCell().hasTakenFlag();};

      /** Returns MASKED flag of Automaton cell.*/
      bool getMaskedFlag() const {return m_cdcWireHit->getAutomatonCell().hasMaskedFlag();};

      /** Check hit drift lenght; if it's greater than cell size return false
       * TODO: This is done using the CDCGeometry - better do this using the CDCWireTopology!
       * */
      bool checkHitDriftLength() const;

      /** Calculate conformal coordinates with respect to choosen point by transforming the wire coordinates. Returns (x',y',driftLength) */
      std::tuple<double, double, double> performConformalTransformWithRespectToPoint(double x0, double y0) const;

      CDCRecoSegment2D& getSegment() {return m_segment;};

      void setSegment(CDCRecoSegment2D& segment) {m_segment = segment; };

    private:
      const CDCWireHit* m_cdcWireHit;    /**< Pointer to the wire hit. */

      Vector2D m_conformalPosition;      /**< Position in the conformal space of the drift center (not the wire!) */
      double m_conformalDriftLength;     /**< Drift time of the hit in the conformal plane (under the assumption that r << x,y*/

      CDCRecoSegment2D m_segment;        // TODO: This feature is never used and should be implemented differently.

      /** Set all parameters from the given wire hit */
      void initializeFromWireHit(const CDCWireHit* wireHit);

      /** Assigns values for conformal coordinates by transforming the wire coordinates. */
      void performConformalTransformation();


    }; //end class CDCTrackHit
  } //end namespace TrackFindingCDC
} //end namespace Belle2


