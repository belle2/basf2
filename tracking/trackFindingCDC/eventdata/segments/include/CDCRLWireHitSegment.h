/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>

#include <tracking/trackFindingCDC/eventdata/collections/CDCGenHitVector.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRLWireHit.h>

namespace genfit {
  class TrackCand;
}

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A segment consisting of two dimensional reconsturcted hits
    class CDCRLWireHitSegment :  public CDCGenHitVector<const Belle2::TrackFindingCDC::CDCRLWireHit*> {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRLWireHitSegment() {;}

      /// Construct from a genfit track candidate.
      explicit CDCRLWireHitSegment(const genfit::TrackCand& trackCand);

      /// Empty deconstructor
      ~CDCRLWireHitSegment() {;}

      /// Defines wire hit segments and superlayers to be coaligned.
      friend bool operator<(const CDCRLWireHitSegment& rlWireHitSegment, const CDCWireSuperLayer& wireSuperLayer)
      { return rlWireHitSegment.getISuperLayer() < wireSuperLayer.getISuperLayer(); }

      /// Defines segments and superlayers to be coaligned.
      friend bool operator<(const CDCWireSuperLayer& wireSuperLayer, const CDCRLWireHitSegment& rlWireHitSegment)
      { return wireSuperLayer.getISuperLayer() < rlWireHitSegment.getISuperLayer(); }

      /// Getter for the vector of wires the hits of this segment are based on in the same order
      std::vector<const Belle2::TrackFindingCDC::CDCWire*> getWireSegment() const
      {
        std::vector<const Belle2::TrackFindingCDC::CDCWire*> wireSegment;
        for (const CDCRLWireHit* ptrRLWireHit : *this) {
          ptrRLWireHit ? wireSegment.push_back(&(ptrRLWireHit->getWire())) : wireSegment.push_back(nullptr);
        }
        return wireSegment;
      }

      /// Getter for the two dimensional trajectory fitted to the segment
      CDCTrajectory2D& getTrajectory2D() const
      { return m_trajectory2D; }

      /// Setter for the two dimensional trajectory fitted to the segment
      void setTrajectory2D(const CDCTrajectory2D& trajectory2D) const
      { m_trajectory2D =  trajectory2D; }

    private:
      /// Memory for the two dimensional trajectory fitted to this segment
      mutable CDCTrajectory2D m_trajectory2D;

    private:

    }; //end class CDCRLWireHitSegment

  } // end namespace TrackFindingCDC
} // end namespace Belle2
