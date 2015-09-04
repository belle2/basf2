/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TVector2.h>

#include <cdc/dataobjects/WireID.h>

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /** This is a mock class, which defines the interface expected by the CDCGenHitVector.

        It is not the meant as a base class yet but rather as a guidline and for testing
        purposes being a drop in replacement for a concrete hit type.
    */

    class CDCGenHit  {

    public:
      /// Default constructor for ROOT compatibility.
      CDCGenHit();

      /// Constructor to taking a wire ID and a dummy position
      CDCGenHit(const WireID& wireID, const Vector2D& pos);

      /// Constructor to taking a wire and a dummy position
      CDCGenHit(const CDCWire* wire, const Vector2D& pos);

      /// Empty deconstructor
      ~CDCGenHit();

      /// Equality comparision based the wire and the dummy position.
      bool operator==(const CDCGenHit& other) const
      { return getWire() == other.getWire() and getDummyPos2D() == other.getDummyPos2D(); }

      /// Total ordering relation based on the wire and the dummy position.
      bool operator<(const CDCGenHit& other) const
      {
        return getWire() < other.getWire() or
               (getWire() == other.getWire() and getDummyPos2D() < other.getDummyPos2D());
      }

      /// Defines wires and dummy hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCGenHit& genHit, const CDCWire& wire)
      { return genHit.getWire() < wire; }

      /// Defines wires and dummy hits to be coaligned on the wire on which they are based.
      friend bool operator<(const CDCWire& wire, const CDCGenHit& genHit)
      { return wire < genHit.getWire(); }

      /** @name Mimic pointer
        */
      /// Access the object methods and methods from a pointer in the same way.
      /**@{*/
      const CDCGenHit* operator->() const { return this; }
      /**@}*/


      /** @name Methods common to all tracking hits
       *  All hits ( track parts contained in a single superlayer ) share this interface to help the definition of collections of them. */
      /**@{*/
      /// Checks of the generic hit is base on the wire given
      bool isOnWire(const CDCWire& wire) const
      { return getWire() == wire; }

      /// Checks of the generic wire hit
      bool hasWireHit(const CDCWireHit&) const
      { return false; }

      /// Getter for the stereo type of the underlying wire.
      StereoType getStereoType() const
      { return getWire().getStereoType(); }

      /// Getter for the super layer id
      ILayerType getISuperLayer() const
      { return getWire().getISuperLayer(); }
      /**@}*/

      /// Getter for the CDCWire the hit is located on.
      const CDCWire& getWire() const
      { return *m_wire; }

      /// The two dimensional dummy position
      const Vector2D& getDummyPos2D() const
      { return m_dummyPos; }

      /// The two dimensional dummy position
      void setDummyPos2D(const Vector2D& dummyPos)
      { m_dummyPos = dummyPos; }

    private:
      const CDCWire* m_wire;  ///< Memory for the CDCWire reference
      Vector2D m_dummyPos; ///< Memory for the dummy position for distance measures


    }; //end class GenHit
  } //end namespace TrackFindingCDC
} //end namespace Belle2

