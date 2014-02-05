/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCGENHIT_H
#define CDCGENHIT_H

#include <TVector2.h>

#include <cdc/dataobjects/WireID.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/topology/CDCWire.h>
#include <tracking/cdcLocalTracking/eventdata/trajectories/CDCTrajectory2D.h>

#include "CDCWireHit.h"


namespace Belle2 {
  namespace CDCLocalTracking {

    /** This is a mock class, which defines the interface expected by the CDCGenHitVector.

        It is not the meant as a base class yet but rather as a guidline and for testing
        purposes being a drop in replacement for a concrete hit type.
    */

    class CDCGenHit : public UsedTObject {

    public:
      /// Default constructor for ROOT compatibility.
      CDCGenHit();

      /// Constructor to taking a wire ID and a dummy position
      CDCGenHit(const WireID& wireID, const Vector2D& pos);

      /// Empty deconstructor
      ~CDCGenHit();


      /** @name Equality comparision
       *  Based on the equality of the wire and the dummy position */
      /**@{*/
      /// Equality comparision based the wire and the dummy position.
      bool operator==(const CDCGenHit& other) const
      { return getWire() == other.getWire() and getDummyPos2D() == other.getDummyPos2D(); }

      /// Equality comparision based on the wire and the dummy position usable with pointer.
      bool inline IsEqual(const CDCGenHit* const& other) const
      { return other == nullptr ? false : operator==(*other); }

      /// Equality comparision based on on the wire and the dummy position usable with two pointer.
      static inline bool ptrIsEqual(const CDCGenHit* lhs , const CDCGenHit* rhs)
      { return lhs == nullptr ? (rhs == nullptr ? true : false) : lhs->IsEqual(rhs); }
      /**@}*/

      /** @name Total ordering
       *  Comparing the wire first and the dummy position second. Hence generic hits are coaligned with the wires.*/
      /**@{*/
      /// Total ordering relation based on the wire and the dummy position.
      bool operator<(const CDCGenHit& other) const
      { return getWire() < other.getWire() or (getWire() == other.getWire() and getDummyPos2D() < other.getDummyPos2D()); }


      /// Defines wires and dummy hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCGenHit& genHit, const CDCWire& wire) { return genHit.getWire() < wire; }

      /// Defines wires and dummy hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWire& wire, const CDCGenHit& genHit) { return wire < genHit.getWire(); }


      /// Total ordering relation based on the wire and the dummy position usable with pointers.
      bool inline IsLessThan(const CDCGenHit* const& other) const
      { return other == nullptr ? false : operator<(*other); }

      /// Total ordering relation based on the wire and the dummy position usable with two pointer
      static inline bool ptrIsLessThan(const CDCGenHit* lhs , const CDCGenHit* rhs)
      { return rhs == nullptr ? false : (lhs == nullptr ? true : *lhs < *rhs); }
      /**@}*/


      /** @name Mimic pointer
        */
      /// Access the object methods and methods from a pointer in the same way.
      /**@{*/
      const CDCGenHit* operator->() const { return this; }
      /**@}*/


      /** @name Methods common to all tracking entities
       *  All entities ( track parts contained in a single superlayer ) share this interface to help the definition of collections of them. */
      /**@{*/
      /// Checks of the generic hit is base on the wire given
      bool hasWire(const CDCWire& wire) const
      { return getWire() == wire; }

      /// Checks of the generic wire hit
      bool hasWireHit(const CDCWireHit& wirehit __attribute__((unused))) const
      { return false; }

      /// Center of mass is just the refernce position for wire hits.
      const Vector2D& getCenterOfMass2D() const { return getDummyPos2D(); }

      /// Getter for the axial type of the underlying wire.
      AxialType getAxialType() const
      { return getWire().getAxialType(); }

      /// Getter for the super layer id
      ILayerType getISuperLayer() const
      { return getWire().getISuperLayer(); }

      /// Same as getPerpS().
      FloatType getStartPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Same as getPerpS().
      FloatType getEndPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Calculates the squared distance of the wire hit to a circle as see from the transvers plane
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const {
        FloatType distance = trajectory2D.getDist2D(getDummyPos2D());
        return distance * distance;
      }
      /**@}*/

      /// Estimate the transvers travel distance on the given circle to this hit.
      FloatType getPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getDummyPos2D()); }

      /// Getter for the CDCWire the hit is located on.
      const CDCWire& getWire() const { return *m_wire; }

      /// The two dimensional dummy position
      const Vector2D& getDummyPos2D() const { return m_dummyPos; }

    private:

      const CDCWire* m_wire;  ///< Memory for the CDCWire reference
      Vector2D m_dummyPos; ///< Memory for the dummy position for distance measures

      /// ROOT Macro to make CDCGenHit a ROOT class.
      ClassDefInCDCLocalTracking(CDCGenHit, 1);

    }; //end class GenHit
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //CDCGENHIT_H

