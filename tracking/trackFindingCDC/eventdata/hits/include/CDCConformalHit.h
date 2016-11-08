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
     *  CDC Hit class used for pattern recognition in the Legendre tracking algorithm.
     *  This class holds a pointer to a CDCWireHit it belongs to and adds information that are only
     *  relevant to the Legendre algorithm (e.g. the conformal position or the conformal drift length).
     *  These variables are precalculated and cached in this object for faster calculation times in the
     *  quad tree. The flags of the wire hit are accessible through the interface also (only for convenience).
     */
    class CDCConformalHit {

    public:
      /**
       *  Constructor to create a CDCConformalHit from a CDCWireHit object.
       *  Some member variables specific to the CDCConformalHit are initialized
       *  (e.g. the conformal transformed position).
       */
      explicit CDCConformalHit(const CDCWireHit* wireHit);

      /** Returns the Hit position (X coordinate) in the conformal plane.*/
      const Vector2D& getConformalPos2D() const
      { return m_conformalPos2D; }

      /** Returns the Hit position (X coordinate) in the conformal plane.*/
      double getConformalX() const
      { return m_conformalPos2D.x(); }

      /** Returns the Hit position (Y coordinate) in the conformal plane.*/
      double getConformalY() const
      { return m_conformalPos2D.y(); }

      /** Returns the drift time in the conformal plane (with r << x,y).*/
      double getConformalDriftLength() const
      { return m_conformalDriftLength; }

      /** Returns pointer to the underlying CDCWireHit.*/
      const CDCWireHit* getWireHit() const { return m_wireHit; }

      /** Sets TAKEN flag of the Automaton cell of the CDCWireHit.*/
      void setUsedFlag(bool flag) { m_wireHit->getAutomatonCell().setTakenFlag(flag); };

      /** Sets MASKED flag of the Automaton cell of the CDCWireHit.*/
      void setMaskedFlag(bool flag) { m_wireHit->getAutomatonCell().setMaskedFlag(flag); };

      /** Returns TAKEN flag of the Automaton cell of the CDCWireHit.*/
      bool getUsedFlag() const {return m_wireHit->getAutomatonCell().hasTakenFlag();};

      /** Returns MASKED flag of the Automaton cell of the CDCWireHit.*/
      bool getMaskedFlag() const {return m_wireHit->getAutomatonCell().hasMaskedFlag();};

      /// Calculate conformal coordinates with respect to choosen point by transforming the wire coordinates. Returns (x',y',driftLength)
      std::tuple<Vector2D, double>
      performConformalTransformWithRespectToPoint(const Vector2D& pos2D) const;

    private:
      /// Pointer to the wire hit.
      const CDCWireHit* m_wireHit;

      /// Position in the conformal space of the drift center (not the wire!)
      Vector2D m_conformalPos2D;

      /// Drift time of the hit in the conformal plane
      double m_conformalDriftLength;

    };
  }
}
