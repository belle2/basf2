/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLTaggedWireHit.h>

namespace Belle2 {

  /// Forward declaration
  class CDCSimHit;

  namespace TrackFindingCDC {

    /** Class representing an oriented hit wire including a hypotheses
     *  whether the causing track passes left or right.
     *  More than one CDCRLWireHit can point to one CDCWireHit.
     *  The CDCWireHitTopology manages all posible instance.
     *  Instances can only be obtained from it with a decided orientation c_Right or c_Left.
     */
    class CDCRLWireHit : public CDCRLTaggedWireHit {
    public:
      CDCRLWireHit(): CDCRLTaggedWireHit(nullptr, ERightLeft::c_Unknown)
      {}
    private:
      /// Only the wire hit topology is allowed to create wire hits
      friend class CDCWireHitTopology;

      /** Constructs an oriented wire hit.
       *
       *  @param wireHit the wire hit the oriented hit is associated with;
       *                 Defaulting to nullptr is necessary because of ROOT dictionaries,
       *                 that are needed for the WireHitTopology.
       *  @param rlInfo the right left passage information of the _wire_ relative to the track */
      CDCRLWireHit(const CDCWireHit* wireHit, ERightLeft rlInfo = ERightLeft::c_Unknown);

    public:
      /// Returns the oriented wire hit with the opposite right left information.
      const CDCRLWireHit* reversed() const;

      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference
       *  there is no way to tell if one should use the dot '.' or operator '->' for method look up.
       *  So this function defines the -> operator for the object.
       *  No matter you have a pointer or an object access is given with '->'
       *  The object is effectively equal to a pointer to itself.
       */
      const CDCRLWireHit* operator->() const
      { return this; }

    }; //class CDCRLWireHit
  } // namespace TrackFindingCDC
} // namespace Belle2
