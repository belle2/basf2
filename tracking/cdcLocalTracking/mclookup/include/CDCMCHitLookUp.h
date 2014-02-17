/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCMCHITLOOKUP_H
#define CDCMCHITLOOKUP_H

#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>
#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/gearbox/Unit.h>

#include "CDCMCTrackStore.h"
#include "CDCSimHitLookUp.h"

#include <map>
#include <list>
#include <vector>

namespace Belle2 {
  namespace CDCLocalTracking {

    ///Class to organize and present the monte carlo hit information

    class CDCMCHitLookUp : public UsedTObject {

    public:
      /// Type for an ordered sequence of pointers to the CDCHit
      typedef CDCMCTrackStore::CDCHitVector CDCHitVector;

    public:
      CDCMCHitLookUp();

      ~CDCMCHitLookUp();

    public:
      static CDCMCHitLookUp& getInstance();

    public:
      void clear();
      void fill();


    public:
      const Belle2::CDCSimHit* getSimHit(const CDCWireHit& wireHit) const {
        const CDCHit* hit = wireHit.getHit();
        return hit ? hit->getRelated<CDCSimHit>() : nullptr;
      }

      const Belle2::MCParticle* getMCParticle(const CDCWireHit& wireHit) const {
        const CDCHit* hit = wireHit.getHit();
        return hit ? hit->getRelated<MCParticle>() : nullptr;
      }

    public:
      /// Indicates if the hit was reassigned to a different mc particle because it was caused by a secondary.
      bool isReassignedSecondary(const CDCWireHit& wireHit) const;

      /// Getter for the closest simulated hit of a primary particle to the given hit - may return nullptr of no closest is found
      const CDCSimHit* getClosestPrimarySimHit(const CDCWireHit& wireHit) const;

      /// Returns the track id for the hit
      ITrackType getMCTrackId(const CDCWireHit& wireHit) const;

      /// Returns if this hit is considered background
      bool isBackground(const CDCWireHit& wireHit) const;

      /// Returns the position if the wire hit in the track along the travel direction
      int getInTrackId(const CDCWireHit& wireHit) const;

      /// Returns the id of the segment in the track.
      int getInTrackSegmentId(const CDCWireHit& wireHit) const;

      /// Returns the number of superlayers the track traversed until this hit.
      int getNPassedSuperLayers(const CDCWireHit& wireHit) const;

      /// Returns the true right left passage information
      RightLeftInfo getRLInfo(const CDCWireHit& wireHit) const;

    }; //class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCMCHITLOOKUP
