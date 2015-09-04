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

#include <tracking/trackFindingCDC/eventdata/hits/CDCGenHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>

#include <cdc/dataobjects/WireID.h>
#include <TVector2.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// A try out of the restrained base class idiom
    class CDCDerivedGenHit {

    public:
      /// Default constructor for ROOT compatibility.
      CDCDerivedGenHit() {;}

      /// Constructor to taking a wire ID and a dummy position
      CDCDerivedGenHit(const WireID& wireID, const Vector2D& pos) : m_impl(wireID, pos) {;}

      /// Constructor to taking a wire ID and a dummy position
      CDCDerivedGenHit(const CDCWire* wire, const Vector2D& pos) : m_impl(wire, pos) {;}

      /// Cast to the restrained constant base class
      operator const Belle2::TrackFindingCDC::CDCGenHit& () const { return m_impl; }

      // made public as a work around for ROOT6 transition
      //private:
      /// Implementation of the restained base class
      class Impl : public CDCGenHit {
      public:
        /// Default constructor for ROOT compatibility.
        Impl() {;}

        /// Constructor to taking a wire ID and a dummy position
        Impl(const WireID& wireID, const Vector2D& pos) : CDCGenHit(wireID, pos) {;}

        /// Constructor to taking a wire ID and a dummy position
        Impl(const CDCWire* wire, const Vector2D& pos) : CDCGenHit(wire, pos) {;}

        /// Setter for a flag
        void setFlag(bool flag)
        { m_flag = flag;}

        /// Getter for a flag
        bool getFlag() const
        { return m_flag;}

        /// The two dimensional dummy position
        void setDummyPos2D(const Vector2D& dummyPos)
        {
          B2INFO("You have to pass this gate keeper");
          B2INFO("No way to access the base method (without a cast)");
          assert(not dummyPos.hasNAN());
          CDCGenHit::setDummyPos2D(dummyPos);
        }

      private:
        /// Try out data member
        bool m_flag;
      };

    public:
      /// Method access operator
      Impl* operator->() { return &m_impl; }

    private:
      /// Instance of the implementation hidding the inheritance.
      Impl m_impl;
    };
  }
}
