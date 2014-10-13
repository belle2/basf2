/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCWIREHITSEGMENT_H_
#define CDCWIREHITSEGMENT_H_

#include <tracking/cdcLocalTracking/rootification/SwitchableRootificationBase.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/collections/CDCGenHitVector.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// A segment consisting of two dimensional reconsturcted hits
    class CDCWireHitSegment :  public CDCGenHitVector<const Belle2::CDCLocalTracking::CDCWireHit*> {
    public:

      /// Default constructor for ROOT compatibility.
      CDCWireHitSegment() {;}

      /// Empty deconstructor
      ~CDCWireHitSegment() {;}

      /// Defines wire hit segments and superlayers to be coaligned.
      friend bool operator<(const CDCWireHitSegment& wireHitSegment, const CDCWireSuperLayer& wireSuperLayer)
      { return wireHitSegment.getISuperLayer() < wireSuperLayer.getISuperLayer(); }

      /// Defines segments and superlayers to be coaligned.
      friend bool operator<(const CDCWireSuperLayer& wireSuperLayer, const CDCWireHitSegment& wireHitSegment)
      { return wireSuperLayer.getISuperLayer() < wireHitSegment.getISuperLayer(); }

      /// Getter for the vector of wires the hits of this segment are based on in the same order
      std::vector<const Belle2::CDCLocalTracking::CDCWire*> getWireSegment() const {
        std::vector<const Belle2::CDCLocalTracking::CDCWire*> wireSegment;
        for (const CDCWireHit * ptrWireHit : *this) {
          ptrWireHit ? wireSegment.push_back(&(ptrWireHit->getWire())) : wireSegment.push_back(nullptr);
        }
        return wireSegment;
      }

      /// Getter for the two dimensional trajectory fitted to the segment
      CDCTrajectory2D& getTrajectory2D() const
      { return m_trajectory2D; }

      /// Setter for the two dimensional trajectory fitted to the segment
      void setTrajectory2D(const CDCTrajectory2D& trajectory2D) const
      { m_trajectory2D =  trajectory2D; }

      /// Reconstructs a central point on the trajectory
      Vector2D getCenterRecoPos2D() const
      { return getTrajectory2D().getClosest(getCenterOfMass2D()); }

      /// Reconstructs a point on the attached trajectory close to the front of the segment
      Vector2D getFrontRecoPos2D() const
      { return front()->getFrontRecoPos2D(getTrajectory2D()); }

      /// Reconstructs a point on the given trajectory close to the front of the segment
      Vector2D getFrontRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return front()->getFrontRecoPos2D(trajectory2D); }

      /// Reconstructs a point on the attached trajectory close to the back of the segment
      Vector2D getBackRecoPos2D() const
      { return back()->getBackRecoPos2D(getTrajectory2D()); }

      /// Reconstructs a point on the attached trajectory close to the back of the segment
      Vector2D getBackRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return back()->getBackRecoPos2D(trajectory2D); }



    private:
      mutable CDCTrajectory2D m_trajectory2D; ///< Memory for the two dimensional trajectory fitted to this segment

    private:
      /// ROOT Macro to make CDCWireHitSegment a ROOT class.
      CDCLOCALTRACKING_SwitchableClassDef(CDCWireHitSegment, 1);

    }; //end class CDCWireHitSegment

  } // end namespace CDCLocalTracking
} // end namespace Belle2
#endif // CDCWIREHITSEGMENT_H_
