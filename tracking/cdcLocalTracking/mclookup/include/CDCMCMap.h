/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCMCMAP_H
#define CDCMCMAP_H

#include <boost/bimap/bimap.hpp>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    ///Class to organize and present the monte carlo hit information

    class CDCMCMap {


    public:
      CDCMCMap();

      ~CDCMCMap();

    public:
      static CDCMCMap& getInstance();

    public:

      void clear();
      void fill();

      //void validateRelations();

    private:
      static bool indicatesReassignedSecondary(double weight)
      { return weight <= 0; }

      void fillHitToSimHitMap();

    public:

      bool isBackground(const CDCSimHit* simHit) const
      { return simHit ? simHit->getBackgroundTag() != CDCSimHit::bg_none : false; }


      const Belle2::CDCSimHit* getSimHit(const CDCHit* hit) const
      { return hit ? m_simHitByHit.by<CDCHit>().at(hit) : nullptr; }

      const Belle2::CDCHit* getHit(const CDCSimHit* simHit) const {
        auto itFound = m_simHitByHit.by<CDCSimHit>().find(simHit);
        return itFound != m_simHitByHit.by<CDCSimHit>().end() ? itFound->get<CDCHit>() : nullptr;
      }



      //{ return hit.getRelated<CDCSimHit>(); }
      /*
      bool isBackground(const CDCHit* hit) const
      { return isBackground(getSimHit(hit)); }



      const Belle2::MCParticle* getMCParticle(const CDCHit* hit) const
      { return hit ? hit->getRelated<MCParticle>() : nullptr; }

      const Belle2::MCParticle* getMCParticle(const CDCSimHit* simHit) const
      { return simHit ? simHit->getRelated<MCParticle>() : nullptr; }



      const Belle2::CDCSimHit* getSimHits(const MCParticle* mcParticle) const
      { return mcParticle ? mcParticle->getRelated<CDCSimHit>() : nullptr; }

      const Belle2::CDCSimHit* getHits(const MCParticle* mcParticle) const
      { return mcParticle ? mcParticle->getRelated<CDCSimHit>() : nullptr; }



      bool isReassignedSecondary(const CDCSimHit* simHit) const
      { return m_reassignedSecondaryHits.count(simHit) > 0; }

      bool isReassignedSecondary(const CDCHit* hit) const
      { return m_reassignedSecondaryHits.count(hit) > 0; }
      */

    private:

      typedef
      boost::bimaps::bimap <
      boost::bimaps::set_of< boost::bimaps::tagged<const CDCHit*, CDCHit> >,
            boost::bimaps::set_of< boost::bimaps::tagged<const CDCSimHit*, CDCSimHit> >
            > CDCSimHitByCDCHitMap;

      typedef CDCSimHitByCDCHitMap::value_type CDCSimHitCDCHitRelation;

      CDCSimHitByCDCHitMap m_simHitByHit;

      /*
      boost::bimap<
        boost::bimaps::set_of<const CDCHit*>,
        boost::bimaps::multiset_of<const MCParticle*>
        > m_mcParticlesByHit;

      boost::bimap<
      boost::bimaps::set_of<const CDCSimHit*>,
      boost::bimaps::multiset_of<const MCParticle*>
      > m_mcParticlesBySimHit;
      */

      std::set<const CDCHit*> m_reassignedSecondaryHits;
      std::set<const CDCSimHit*> m_reassignedSecondarySimHits;

    }; //class
  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCMCHITLOOKUP
