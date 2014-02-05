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

      /// Constructs a reconstructed hit
      /** Constructs a reconstructed hit
       *  @param wirehit the wire hit the reconstructed hit is assoziated with.
       *  @param displacement the two dimensional displacement of the reconstructed position relativ to the reference wire position
       *  @param rlinfo the right left passage information the _wire_ relativ to the track */
      CDCRecoHit2D(const CDCWireHit* wirehit, const Vector2D& displacement, RightLeftInfo rlInfo = 0);


      /// Constructs a reconstructed hit
      /** Constructs a reconstructed hit
       *  @param wirehit the wire hit the reconstructed hit is assoziated with.
       *  @param rlinfo the right left passage information the _wire_ relativ to the track
       *  The displacement is set to (0.0,0.0) in this case */
      explicit CDCRecoHit2D(const CDCWireHit* wirehit, RightLeftInfo rlinfo = 0);


      /// Constructs the average of two reconstructed hit positions and snaps it to the drift circle. \n
      /** Averages the hit positions first. But the result will not lie on the circle, so we scale the \n
       *  the displacement to snap onto the drift circle again. The function averages only reconstructed hits \n
       *  assoziated with the same wire hit. If not all recostructed hits are on the same wire hit, the first hit \n
       *  is returned unchanged. Also averages the right left passage information with averageInfo(). */
      static CDCRecoHit2D average(const CDCRecoHit2D& recohit1, const CDCRecoHit2D& recohit2);


      /// Constructs the average of three reconstructed hit positions and snaps it to the drift circle. \n
      /** Averages the hit positions first. But the result will not lie on the circle, so we scale the \n
       *  the displacement to snap onto the drift circle again. The function averages only reconstructed hits \n
       *  assoziated with the same wire hit. If not all recostructed hits are on the same wire, the first hit \n
       *  is returned unchanged. Also averages the right left passage information with averageInfo(). */
      static CDCRecoHit2D average(const CDCRecoHit2D& recohit1,
                                  const CDCRecoHit2D& recohit2 ,
                                  const CDCRecoHit2D& recohit3);


      /// Constructs a two dimensional reconstructed hit from an absolute position
      /** Constructs a two dimensional reconstructed hit from
       *  @param wirehit the wire hit the reconstructed hit is assoziated to
       *  @param rlinfo  the right left passage information of the wire relative to the track
       *  @param pos2D the absolut position of the wire
       *  @param snap optional indicator if the displacement shall be shrank to the drift circle (default true)
       */
      static CDCRecoHit2D fromAbsPos2D(
        const CDCWireHit* wirehit,
        RightLeftInfo rlinfo,
        const Vector2D& pos2D,
        bool snap = true
      );

      /// Empty deconstructor
      ~CDCRecoHit2D();

      /// Constructs a two dimensional reconstructed hit from a sim hit and the assoziated wirehit.
      /** This translates the sim hit to a reconstructed hit mainly to be able to compare the \n
       *  reconstructed values from the algorithm with the Monte Carlo information. \n
       *  It merely takes the displacement from the wire, projects it to the reference plane and \n
       *  scales it onto the drift circle defined by the wire. */
      static CDCRecoHit2D fromSimHit(const CDCWireHit* wirehit, const CDCSimHit& simhit);

      /** @name Equality comparision
       *  Based on the equality of the wire hit, the left right passage information and the displacement */
      /**@{*/

      /// Equality comparision based on wire hit, left right passage information and displacement.
      bool operator==(const CDCRecoHit2D& other) const {
        return *(getWireHit()) == *(other.getWireHit()) and
               getRLInfo() == other.getRLInfo() and
               getRefDisp2D() == other.getRefDisp2D();
      }

      /// Equality comparision based on wire hit, left right passage information and displacement.
      /** Equality comparision of reconstructed hits based on  wire hit, left right passage information and displacement.
       *  This is still usable if a nullptr is given. The nullptr is always different to an actual wire object.
       *  Compatible for use with ROOT containers.
       */
      bool IsEqual(const CDCRecoHit2D* const& other) const
      { return other == nullptr ? false : operator==(*other); }
      /**@}*/

      /** @name Total ordering
       *  Comparing the wire hit, the right left passage info and the displacement in this order of importance. */
      /**@{*/
      /// Total ordering relation based on wire hit, left right passage information and displacement in this order of importance.
      bool operator<(const CDCRecoHit2D& other) const {
        return *(getWireHit()) <  *(other.getWireHit()) or (
                 *(getWireHit()) == *(other.getWireHit()) and (
                   getRLInfo() < other.getRLInfo() or (
                     getRLInfo() == other.getRLInfo() and (
                       getRefDisp2D() <  other.getRefDisp2D()))));
      }

      /// Defines wires and the two dimensional reconstructed hits as coaligned
      friend bool operator<(const CDCRecoHit2D& recoHit2D, const CDCWire& wire) { return *(recoHit2D.getWire()) < wire; }

      /// Defines wires and the two dimensional reconstructed hits as coaligned
      friend bool operator<(const CDCWire& wire, const CDCRecoHit2D& recoHit2D) { return wire < *(recoHit2D.getWire()); }

      /// Defines wire hits and the two dimensional reconstructed hits as coaligned
      friend bool operator<(const CDCRecoHit2D& recoHit2D, const CDCWireHit& wireHit) { return *(recoHit2D.getWireHit()) < wireHit; }

      /// Defines wire hits and the two dimensional reconstructed hits as coaligned
      friend bool operator<(const CDCWireHit& wireHit, const CDCRecoHit2D& recoHit2D) { return wireHit < *(recoHit2D.getWireHit()); }

      /// Total ordering relation based on wire hit, left right passage information and displacement usable with pointers
      /** Retains the total ordering sheme for reconstructed hit objects, \n
       *  but introduces the special nullptr case to the ordering.
       *  The nullptr is always smallest. Therefore it forms a lower bound \n
       *  for the reconstructed hit pointers.
       *  This also enables compatibility with all sorts of ROOT containers */
      bool IsLessThan(const CDCRecoHit2D* const& other) const
      { return other == nullptr ? false : operator<(*other); }

      /// Getter for the lowest possible reconstructed hit.
      /** Returns reconstructed hit that compare less to all other possible instances. \n
       *  Maybe used by higher order tracking entities as a sentinal component for look up \n
       *  into sorted ranges of them */
      static const CDCRecoHit2D getLowest()
      { return CDCRecoHit2D(&(CDCWireHit::getLowest()), Vector2D::getLowest() , LEFT); }

      /// Getter for the lowest possible reconstructed hit on the given wire hit.
      /** Returns reconstructed hit that compare less to all other possible instances on the given wire. \n
       *  This enables us to find all reconstructed hits in a sorted range that are assoziated with the same wire, \n
       */
      static const CDCRecoHit2D getLowerBound(const CDCWireHit* wirehit)
      { return CDCRecoHit2D(wirehit, Vector2D::getLowest(), LEFT); }
      /**@}*/

      /// Getter for the wire the reconstructed hit assoziated to.
      const CDCWire* getWire() const { return &(getWireHit()->getWire()); }

      /// Getter for the wire hit assoziated with the reconstructed hit.
      const CDCWireHit* getWireHit() const
      { return m_wirehit; }

      /// Getter for the displacement from the wire reference position
      const Vector2D& getRefDisp2D() const
      { return m_displacement; }

      /// Getter for the right left passage information. Details.
      /** Returns the if the _wire hit_ is located right or left of the track,
       *  _not_ if the reconstructed position is right or left of it. */
      const RightLeftInfo& getRLInfo() const { return m_rlInfo; }

      /// Getter for the position in the reference plane
      Vector2D getRefPos2D() const
      { return getRefDisp2D() + getWireHit()->getRefPos2D(); }

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
      { return (getWire()->getSkewLine()).movedBy(getRefDisp2D()); }

      /// Estimate the transvers travel distance on the given circle.
      /** Uses the point of closest approach to the reconstructed position
       *  on the circle and calculates the arc length from the reference on the circle.
       *  @return The arc length on the circle from the reference */
      FloatType getPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getRefPos2D()); }

      /// Scales the displacement vector in place to lie on the dirft circle.
      void snapToDriftCircle()
      { m_displacement.normalizeTo(getWireHit()->getRefDriftLength()); }

      /// Turns the orientation in place.
      /** Changes the sign of the right left passage information, since the position remains the same by this reversion.*/
      void reverse()
      { m_rlInfo = -m_rlInfo; }

      /** Returns the recohit with the opposite right left information */
      CDCRecoHit2D reversed() const
      { return CDCRecoHit2D(getWireHit(), getRefDisp2D(), -getRLInfo()); }

      /** @name Mimic pointer
        */
      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell \n
       *  if one should use the dot '.' or operator '->' for method look up. \n
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'*/
      /**@{*/
      const CDCRecoHit2D* operator->() const { return this; }
      /**@}*/

      /** @name Methods common to all tracking entities
       *  All entities ( track parts contained in a single superlayer ) share this interface to help definition of collections of them.
       */
      /**@{*/
      /// Checks if the reconstructed hit is assoziated with the give wire
      bool hasWire(const CDCWire& wire) const
      { return getWireHit()->getWire() == wire; }

      /// Checks if the reconstructed hit is assoziated with the give wire hit
      bool hasWireHit(const CDCWireHit& wirehit) const
      { return *getWireHit() == wirehit; }

      /// Center of mass is just the reconstructed position
      Vector2D getCenterOfMass2D() const
      { return getRefPos2D(); }

      /// Getter for the axial type of the underlying wire.
      AxialType getAxialType() const
      { return getWireHit()->getAxialType(); }

      /// Getter for the superlayer id
      ILayerType getISuperLayer() const
      { return getWireHit()->getISuperLayer(); }

      /// Same as getPerpS().
      FloatType getStartPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Same as getPerpS().
      FloatType getEndPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Calculates the squared distance of the reconstructed position to a circle as see from the transvers plane.
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const
      { FloatType distance = trajectory2D.getDist2D(getRefPos2D()); return distance * distance; }

      /// Calculates the squared distance in z direction.
      /** This can not be calculated from the wire hit alone so the result is always NaN. */
      //FloatType getSquaredZDist( const CDCSZFit & szFit __attribute__ ((unused)) ) const
      //{ return std::numeric_limits<FloatType>::quiet_NaN(); }
      /**@}*/

      /// Output operator. Help debugging.
      friend std::ostream& operator<<(std::ostream& output, const CDCRecoHit2D& recohit) {
        output << "CDCRecoHit2D(" << recohit.getWireHit() << ","
               << recohit.getRefDisp2D() << ","
               << recohit.getRLInfo() << ")" ;
        return output;
      }

    private:

      const CDCWireHit* m_wirehit;  ///< Memory for the reference to the assiziated wire hit
      Vector2D m_displacement; ///< Memory for the displacement fo the assoziated wire reference position
      RightLeftInfo m_rlInfo; ///< Memory for the right left passage information of the wire hit.


      /// ROOT Macro to make CDCRecoHit2D a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoHit2D, 1);

    }; //class CDCRecoHit2D

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOHIT2D_H
