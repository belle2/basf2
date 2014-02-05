/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRLWIREHIT_H
#define CDCRLWIREHIT_H


#include <cdc/dataobjects/CDCSimHit.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include "CDCWireHit.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing an oriented hit wire including a hypotheses whether the causing track passes left or right
    class CDCRLWireHit : public UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRLWireHit();

      /// Constructs an oriented wire hit
      /** Constructs an oriented wire hit
       *  @param wirehit the wire hit the oriented hit is assoziated with.
       *  @param rlinfo the right left passage information the _wire_ relativ to the track */
      CDCRLWireHit(const CDCWireHit* wirehit, RightLeftInfo rlInfo = 0);

      /// Empty deconstructor
      ~CDCRLWireHit();

      /// Constructs an oriented wire hit from a sim hit and the assoziated wirehit.
      /** This translates the sim hit to an oriented wire hit mainly to be able to compare the \n
       *  reconstructed values from the algorithm with the Monte Carlo information. \n
       *  It merely evalutates, if the true trajectory passes right or left of the wire. */
      static CDCRLWireHit fromSimHit(const CDCWireHit* wirehit, const CDCSimHit& simhit);

      /** Returns the oriented wire hit with the opposite right left information */
      CDCRLWireHit reversed() const;

      /** @name Equality comparision
       *  Based on the equality of the wire hit, the left right passage information*/
      /**@{*/

      /// Equality comparision based on wire hit, left right passage information.
      bool operator==(const CDCRLWireHit& other) const {
        return getWireHit() == other.getWireHit() and getRLInfo() == other.getRLInfo();
      }

      /// Equality comparision based on wire hit, left right passage information and displacement.
      /** Equality comparision of oriented hits based on  wire hit, left right passage information and displacement.
       *  This is still usable if a nullptr is given. The nullptr is always different to an actual wire object.
       *  Compatible for use with ROOT containers.
       */
      bool IsEqual(const CDCRLWireHit* const& other) const
      { return other == nullptr ? false : operator==(*other); }
      /**@}*/

      /** @name Total ordering
       *  Comparing the wire hit and the right left passage info in this order of importance. */
      /**@{*/
      /// Total ordering relation based on wire hit and left right passage information in this order of importance.
      bool operator<(const CDCRLWireHit& other) const {
        return getWireHit() <  other.getWireHit() or (
                 getWireHit() == other.getWireHit() and (
                   getRLInfo() < other.getRLInfo()));
      }

      /// Total ordering relation based on wire hit and left right passage information usable with pointers
      /** Retains the total ordering sheme for oriented hit objects, \n
       *  but introduces the special nullptr case to the ordering.
       *  The nullptr is always smallest. Therefore it forms a lower bound \n
       *  for the oriented hit pointers.
       *  This also enables compatibility with all sorts of ROOT containers */
      bool IsLessThan(const CDCRLWireHit* const& other) const
      { return other == nullptr ? false : operator<(*other); }

      /// Getter for the lowest possible oriented wire hit.
      /** Returns oriented hit that compares less to all other possible instances. \n
       *  Maybe used by higher order tracking entities as a sentinal component for look up \n
       *  into sorted ranges. */
      static const CDCRLWireHit getLowest()
      { return CDCRLWireHit(&(CDCWireHit::getLowest()), LEFT); }

      /// Getter for the lowest possible oriented hit on the given wire hit.
      /** Returns oriented hit that compare less to all other possible instances on the given wire. \n
       *  This enables us to find all oriented hits in a sorted range that are assoziated with the same wire, \n
       */
      static const CDCRLWireHit getLowerBound(const CDCWireHit* wirehit)
      { return CDCRLWireHit(wirehit, LEFT); }
      /**@}*/

      /// Getter for the wire the oriented hit assoziated to.
      const CDCWire& getWire() const { return getWireHit().getWire(); }

      /// Getter for the wire hit assoziated with the oriented hit.
      const CDCWireHit& getWireHit() const { return *m_wirehit; }

      /// Getter for the right left passage information.
      const RightLeftInfo& getRLInfo() const { return m_rlInfo; }

      /// The two dimensional reference position of the underlying wire
      const Vector2D& getRefPos2D() const { return getWireHit().getRefPos2D(); }

      /// Getter for the  drift length at the reference position of the wire
      FloatType getRefDriftLength() const
      { return getWireHit().getRefDriftLength(); }


      /// Estimate the transvers travel distance on the given circle.
      /** Uses the point of closest approach to the wire hit position
       *  on the circle and calculates the arc length from the reference on the circle.
       *  @return The arc length on the circle from the reference */
      FloatType getPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getWireHit().getPerpS(trajectory2D); }

      /** @name Mimic pointer
        */
      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell \n
       *  if one should use the dot '.' or operator '->' for method look up. \n
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'*/
      /**@{*/
      const CDCRLWireHit* operator->() const { return this; }
      /**@}*/

      /** @name Methods common to all tracking entities
       *  All entities ( track parts contained in a single superlayer ) share this interface to help definition of collections of them.
       */
      /**@{*/
      /// Checks if the oriented hit is assoziated with the give wire
      bool hasWire(const CDCWire& wire) const
      { return getWireHit().getWire() == wire; }

      /// Checks if the oriented hit is assoziated with the give wire hit
      bool hasWireHit(const CDCWireHit& wirehit) const
      { return getWireHit() == wirehit; }

      /// Center of mass is the refernce position for underlying wire hits.
      Vector2D getCenterOfMass2D() const
      { return getWireHit().getRefPos2D(); }

      /// Getter for the axial type of the underlying wire.
      AxialType getAxialType() const
      { return getWireHit().getAxialType(); }

      /// Getter for the superlayer id
      ILayerType getISuperLayer() const
      { return getWireHit().getISuperLayer(); }

      /// Same as getPerpS().
      FloatType getStartPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Same as getPerpS().
      FloatType getEndPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Calculates the squared distance of the oriented hit to a circle as see from the transvers plane.
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const {
        FloatType correctedDistance = trajectory2D.getDist2D(getRefPos2D()) - getRLInfo() * getRefDriftLength();
        return correctedDistance * correctedDistance;
      }

      /// Output operator. Help debugging.
      friend std::ostream& operator<<(std::ostream& output, const CDCRLWireHit& rlWireHit) {
        output << "CDCRLWireHit(" << rlWireHit.getWireHit() << ","
               << rlWireHit.getRLInfo() << ")" ;
        return output;
      }

    private:

      const CDCWireHit* m_wirehit;  ///< Memory for the reference to the assiziated wire hit
      RightLeftInfo m_rlInfo; ///< Memory for the right left passage information of the oriented wire hit.

      /// ROOT Macro to make CDCRLWireHit a ROOT class.
      ClassDefInCDCLocalTracking(CDCRLWireHit, 1);

    }; //class CDCRLWireHit

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRLWIREHIT_H
