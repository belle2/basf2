/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOHIT2D_H
#define CDCRECOHIT2D_H

#include <cdc/dataobjects/CDCSimHit.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include "CDCWireHit.h"
#include "CDCRLWireHit.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing a two dimensional reconstructed hit in the central drift chamber
    /** A recohit represents a likely point where the particle went through. It is always assoziated with a \n
     *  wire hit it seeks to reconstruct. The reconstructed point is stored as a displacement from the \n
     *  wire reference position assoziated with the hit. The displacement generally is as long as the drift length \n
     *  but must not. In addition the reconstructed hit takes a right left passage information which indicates if \n
     *  the hit wire lies to the right or to the left of the particle trajectory causing the hit. The later readily \n
     *  indicates a flight direction from the reconstructed it, if a tangential approch of the trajectory to the \n
     *  drift circle is assumed. */
    class CDCRecoHit2D : public CDCLocalTracking::UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoHit2D();

      /// Constructs a reconstructed hit based on the given oriented wire hit with the given displacement from the wire reference position.
      CDCRecoHit2D(const CDCRLWireHit* rlWireHit, const Vector2D& recoDisp2D);


      /// Constructs a reconstructed hit based on the oriented wire hit with no displacement.
      explicit CDCRecoHit2D(const CDCRLWireHit* rlWireHit);


      /// Constructs the average of two reconstructed hit positions and snaps it to the drift circle. \n
      /** Averages the hit positions first. But the result will not lie on the circle, so we scale the \n
       *  the displacement to snap onto the drift circle again. The function averages only reconstructed hits \n
       *  assoziated with the same wire hit. If not all recostructed hits are on the same wire hit, the first hit \n
       *  is returned unchanged. Also averages the right left passage information with averageInfo(). */
      static CDCRecoHit2D average(const CDCRecoHit2D& recoHit1, const CDCRecoHit2D& recoHit2);


      /// Constructs the average of three reconstructed hit positions and snaps it to the drift circle. \n
      /** Averages the hit positions first. But the result will not lie on the circle, so we scale the \n
       *  the displacement to snap onto the drift circle again. The function averages only reconstructed hits \n
       *  assoziated with the same wire hit. If not all recostructed hits are on the same wire, the first hit \n
       *  is returned unchanged. Also averages the right left passage information with averageInfo(). */
      static CDCRecoHit2D average(const CDCRecoHit2D& recoHit1,
                                  const CDCRecoHit2D& recoHit2 ,
                                  const CDCRecoHit2D& recoHit3);

      /// Constructs a two dimensional reconstructed hit from an absolute position
      /** Constructs a two dimensional reconstructed hit from
       *  @param rlWireHit the oriented wire hit the reconstructed hit is assoziated to
       *  @param pos2D the absolut position of the wire
       *  @param snap optional indicator if the displacement shall be shrank to the drift circle (default true)
       */
      static CDCRecoHit2D fromAbsPos2D(
        const CDCRLWireHit* rlWireHit,
        const Vector2D& pos2D,
        bool snap = true
      );

      /// Turns the orientation in place.
      /** Changes the sign of the right left passage information, since the position remains the same by this reversion.*/
      void reverse();

      /** Returns the recohit with the opposite right left information */
      CDCRecoHit2D reversed() const;

      /// Empty deconstructor
      ~CDCRecoHit2D();

      /// Constructs a two dimensional reconstructed hit from a sim hit and the assoziated wirehit.
      /** This translates the sim hit to a reconstructed hit mainly to be able to compare the \n
       *  reconstructed values from the algorithm with the Monte Carlo information. \n
       *  It merely takes the displacement from the wire, projects it to the reference plane and \n
       *  scales it onto the drift circle defined by the wire. */
      static CDCRecoHit2D fromSimHit(const CDCWireHit* wireHit, const CDCSimHit& simHit);


      /// Make the wire hit automatically castable to its underlying cdcHit
      operator const Belle2::CDCHit* () const
      { return (const CDCHit*)getRLWireHit(); }


      /// Equality comparision based on the oriented wire hit and displacement.
      bool operator==(const CDCRecoHit2D& other) const {
        return getRLWireHit() == other.getRLWireHit() and
               getRecoDisp2D() == other.getRecoDisp2D();
      }

      /// Total ordering relation based on wire hit, left right passage information and displacement in this order of importance.
      bool operator<(const CDCRecoHit2D& other) const {
        return getRLWireHit() <  other.getRLWireHit() or (
                 getRLWireHit() == other.getRLWireHit() and
                 getRecoDisp2D() < other.getRecoDisp2D());
      }

      /// Defines wires and the two dimensional reconstructed hits as coaligned
      friend bool operator<(const CDCRecoHit2D& recoHit2D, const CDCWire& wire) { return recoHit2D.getWire() < wire; }

      /// Defines wires and the two dimensional reconstructed hits as coaligned
      friend bool operator<(const CDCWire& wire, const CDCRecoHit2D& recoHit2D) { return wire < recoHit2D.getWire(); }

      /// Defines wire hits and the two dimensional reconstructed hits as coaligned
      friend bool operator<(const CDCRecoHit2D& recoHit2D, const CDCWireHit& wireHit) { return recoHit2D.getWireHit() < wireHit; }

      /// Defines wire hits and the two dimensional reconstructed hits as coaligned
      friend bool operator<(const CDCWireHit& wireHit, const CDCRecoHit2D& recoHit2D) { return wireHit < recoHit2D.getWireHit(); }

      /// Output operator. Help debugging.
      friend std::ostream& operator<<(std::ostream& output, const CDCRecoHit2D& recohit) {
        output << "CDCRecoHit2D(" << recohit.getRLWireHit() << ","
               << recohit.getRecoDisp2D() << ")" ;
        return output;
      }

      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell \n
       *  if one should use the dot '.' or operator '->' for method look up. \n
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'*/
      const CDCRecoHit2D* operator->() const { return this; }


      /// Return the skew line assoziated with the reconstructed  two dimensional hit
      /** The two dimensional reconstructed hit stores only the displacement at the reference position. \n
       *  Like the wire is related to its reference position, the reconstructed position represents many possible \n
       *  three dimensional positions. This method returns a line parallel to the wire moved by the reconstructed \n
       *  displacement. This line represents all possible three dimensional reconstructed positions.
       *
       *  Note : This is not optimal yet. In computation steps as well as the correctness of the line. \n
       *  It could be corrected for flight time and in wire delays. The effect of this adjustments might \n
       *  be worth while investigating */
      BoundSkewLine getSkewLine() const
      { return getWire().getSkewLine().movedBy(getRecoDisp2D()); }

      /// Getter for the axial type of the underlying wire.
      AxialType getAxialType() const
      { return getRLWireHit().getAxialType(); }

      /// Getter for the superlayer id
      ILayerType getISuperLayer() const
      { return getRLWireHit().getISuperLayer(); }



      /// Getter for the wire the reconstructed hit assoziated to.
      const CDCWire& getWire() const { return getRLWireHit().getWire(); }

      /// Checks if the reconstructed hit is assoziated with the give wire
      bool hasWire(const CDCWire& wire) const
      { return getRLWireHit().hasWire(wire); }



      /// Getter for the wire hit assoziated with the reconstructed hit.
      const CDCWireHit& getWireHit() const
      { return getRLWireHit().getWireHit(); }

      /// Checks if the reconstructed hit is assoziated with the give wire hit
      bool hasWireHit(const CDCWireHit& wireHit) const
      { return getRLWireHit().hasWireHit(wireHit); }



      /// Getter for the right left passage information.
      const RightLeftInfo& getRLInfo() const
      { return getRLWireHit().getRLInfo(); }



      /// Getter for the position in the reference plane
      Vector2D getRecoPos2D() const
      { return getRecoDisp2D() + getWireHit().getRefPos2D(); }

      /// Getter for the displacement from the wire reference position
      const Vector2D& getRecoDisp2D() const
      { return m_recoDisp2D; }

      /// Scales the displacement vector in place to lie on the dirft circle.
      void snapToDriftCircle()
      { m_recoDisp2D.normalizeTo(getRLWireHit().getRefDriftLength()); }

      /// Projects the hit's reconstructed position onto the given trajectory
      Vector2D getRecoPos2D(const CDCTrajectory2D& trajectory2D) const;

      /// Estimate the transvers travel distance on the given circle.
      /** Uses the point of closest approach to the reconstructed position
       *  on the circle and calculates the arc length from the reference on the circle.
       *  @return The arc length on the circle from the reference */
      FloatType getPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getRecoPos2D()); }

      /// Same as getRecoPos2D()
      Vector2D getFrontRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return getRecoPos2D(trajectory2D); }

      /// Same as getRecoPos2D()
      Vector2D getBackRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return getRecoPos2D(trajectory2D); }

      /// Same as getPerpS().
      FloatType getFrontPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Same as getPerpS().
      FloatType getBackPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Center of mass is just the reconstructed position
      Vector2D getCenterOfMass2D() const
      { return getRecoPos2D(); }

      /// Calculates the squared distance of the reconstructed position to a circle as see from the transvers plane.
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const
      { FloatType distance = trajectory2D.getDist2D(getRecoPos2D()); return distance * distance; }


      /// Getter for the oriented wire hit assoziated with the reconstructed hit.
      const CDCRLWireHit& getRLWireHit() const
      { return *m_rlWireHit; }

      /// Setter for the oriented wire hit assoziated with the reconstructed hit.
      void setRLWireHit(const CDCRLWireHit* rlWireHit)
      { m_rlWireHit = rlWireHit; }

    private:

      const CDCRLWireHit* m_rlWireHit;  ///< Memory for the reference to the assiziated wire hit
      Vector2D m_recoDisp2D; ///< Memory for the displacement fo the assoziated wire reference position

      /// ROOT Macro to make CDCRecoHit2D a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoHit2D, 1);

    }; //class CDCRecoHit2D

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOHIT2D_H
