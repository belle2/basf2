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

#include <tracking/trackFindingCDC/utilities/Range.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <map>
#include <set>

namespace Belle2 {
  class MCParticle;
  class CDCSimHit;
  class CDCHit;

  namespace TrackFindingCDC {

    /// Class to organize and present the monte carlo hit information
    class CDCMCMap {

    public:
      /// Delete the assignement operator in order to avoid accidental copies.
      CDCMCMap& operator=(const CDCMCMap&) = delete;

    public:

      /// Clear all information from the former event.
      void clear();

      /// Fill the Monte Carlo information retrieved from the DataStore into the local multimaps.
      void fill();

    private:
      /**
       *  Indicate if the given weight suggests that the corresponding hit to MCParticle relation
       *  has been redirected to point to a primary particle instead of the discarded secondary particle.
       */
      static bool indicatesReassignedSecondary(double weight)
      {
        return weight <= 0;
      }

      /// Retrieve the relations array from CDCSimHits to CDCHits and fill it in to the local map which does the inverse mapping
      void fillSimHitByHitMap();

      /// Retrieve the relations array from MCParticle to CDCHits and fill it in to the local map which does the inverse mapping
      void fillMCParticleByHitMap();

      /// Retrieve the relations array from MCParticle to CDCSimHits and fill it in to the local map which does the inverse mapping
      void fillMCParticleBySimHitMap();

      /// Checks if the relations CDCHit -> MCParticle and CDCHit -> CDCSimHit -> MCParticle commute.
      void validateRelations() const;

      /// Checks if each CDCHit is marked as reassigned secondary is related to a reassigned secondary CDCSimHit.
      void validateReassignedSecondaries() const;

    public:
      /// Seeks the CDCSimHit related to the CDCHit.
      MayBePtr<const CDCSimHit> getSimHit(const CDCHit* hit) const;

      /// Seeks the CDCHit related to the CDCSimHit - nullptr if no CDCHit is related.
      MayBePtr<const CDCHit> getHit(const CDCSimHit* simHit) const;

      /// Indicates if the CDCSimHit is considered background.
      bool isBackground(const CDCSimHit* simHit) const;

      /// Indicates if the CDCSimHit is considered background.
      bool isBackground(const CDCHit* hit) const;

      /**
       *  Seeks the MCParticle related to the CDCHit.
       *  Nullptr if no MCParticle is related, this is also the case for background hits.
       */
      MayBePtr<const MCParticle> getMCParticle(const CDCHit* hit) const;

      /**
       *  Seeks the MCParticle related to the CDCSimHit.
       *  Nullptr if no MCParticle is related, this is also the case for background hits.
       */
      MayBePtr<const MCParticle> getMCParticle(const CDCSimHit* simHit) const;

      /// Getter for the range MCParticle to CDCSimHits relations which come from the given MCParticle
      auto getSimHits(const MCParticle* mcParticle) const
      {
        return asRange(m_simHitsByMCParticle.equal_range(mcParticle));
      }

      /// Getter for the range MCParticle to CDCHits relations which come from the given MCParticle.
      auto getHits(const MCParticle* mcParticle) const
      {
        return asRange(m_hitsByMCParticle.equal_range(mcParticle));
      }

      /// Indicates if the CDCSimHit has been reassigned to a primary MCParticle
      bool isReassignedSecondary(const CDCSimHit* ptrSimHit) const
      {
        return m_reassignedSecondarySimHits.count(ptrSimHit) > 0;
      }

      /// Indicates if the CDCHit has been reassigned to a primary MCParticle
      bool isReassignedSecondary(const CDCHit* ptrHit) const
      {
        return m_reassignedSecondaryHits.count(ptrHit) > 0;
      }

      /// Getter for all reassigned secondary CDCHits.
      const std::set<const CDCHit*>& getReassignedSecondaryHits() const
      {
        return m_reassignedSecondaryHits;
      }

      /// Getter for all reassigned secondary CDCSimHits.
      const std::set<const CDCSimHit*>& getReassignedSecondarySimHits() const
      {
        return m_reassignedSecondarySimHits;
      }

      /// Getter for the CDCHit -> CDCSimHit relations.
      const std::multimap<const CDCHit*, const CDCSimHit*>& getSimHitsByHit() const
      {
        return m_simHitsByHit;
      }

      /// Getter for the MCParticle -> CDCHit relations.
      const std::multimap<const MCParticle*, const CDCHit*>& getHitsByMCParticle() const
      {
        return m_hitsByMCParticle;
      }

      /// Getter for the MCParticle -> CDCSimHit relations.
      const std::multimap<const MCParticle*, const CDCSimHit*>& getSimHitsByMCParticle() const
      {
        return m_simHitsByMCParticle;
      }

    private:
      /// Memory for a one to one relation from CDCHit to CDCSimHits
      std::multimap<const CDCHit*, const CDCSimHit*> m_simHitsByHit;

      /// Memory for a one to n relation from MCParticles to CDCHit
      std::multimap<const MCParticle*, const CDCHit*> m_hitsByMCParticle;

      /// Memory for a one to n relation from MCParticles to CDCSimHit
      std::multimap<const MCParticle*, const CDCSimHit*> m_simHitsByMCParticle;

      /// The set of reassigned secondary CDCHits
      std::set<const CDCHit*> m_reassignedSecondaryHits;

      /// The set of reassigned secondary CDCSimHits
      std::set<const CDCSimHit*> m_reassignedSecondarySimHits;
    };
  }
}
