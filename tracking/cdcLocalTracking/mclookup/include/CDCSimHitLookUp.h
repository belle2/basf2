/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCSIMHITLOOKUP_H
#define CDCSIMHITLOOKUP_H

#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>
#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/gearbox/Unit.h>

#include "CDCMCTrackStore.h"

#include <map>
#include <list>
#include <vector>

namespace Belle2 {
  namespace CDCLocalTracking {

    ///Class to organize and present the monte carlo hit information

    class CDCSimHitLookUp : public UsedTObject {

    public:
      /// Type for an ordered sequence of pointers to the CDCHit
      typedef CDCMCTrackStore::CDCHitVector CDCHitVector;

    public:
      CDCSimHitLookUp();

      ~CDCSimHitLookUp();

    public:
      static CDCSimHitLookUp& getInstance();

    public:
      void clear();
      void fill();

    private:
      void fillPrimarySimHits();
      bool isReassignedSecondaryHit(const CDCSimHit& simHit) const;
      const CDCSimHit* getClosestPrimarySimHit(const CDCSimHit* simHit) const;


      void fillRLInfo();
      RightLeftInfo getPrimaryRLInfo(const CDCSimHit& simHit) const;
      //RightLeftInfo getRLInfo(const CDCSimHit& simHit) const;

    public:
      const Belle2::CDCSimHit* getSimHit(const CDCWireHit& wireHit) const {
        const CDCHit* hit = wireHit.getHit();
        return hit ? hit->getRelated<CDCSimHit>() : nullptr;
      }

    public:
      bool isReassignedSecondaryHit(const CDCHit* hit) const;
      const CDCSimHit* getClosestPrimarySimHit(const CDCHit* hit) const;
      RightLeftInfo getRLInfo(const CDCHit* wireHit) const;

    private:
      std::map<const CDCHit*, const CDCSimHit*>  m_primarySimHits;
      std::map<const CDCHit*, RightLeftInfo> m_rightLeftInfos;

    }; //class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCSIMHITLOOKUP
