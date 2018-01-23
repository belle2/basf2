/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/mclookup/CDCMCMap.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  template <class MapType>
  void print_map(const MapType& m)
  {
    using const_iterator = typename MapType::const_iterator;
    for (const_iterator iter = m.begin(), iend = m.end(); iter != iend; ++iter) {
      std::cout << iter->first << "-->" << iter->second << std::endl;
    }
  }
}

void CDCMCMap::clear()
{
  B2DEBUG(100, "In CDCMCMap::clear()");
  m_simHitsByHit.clear();

  m_hitsByMCParticle.clear();
  m_simHitsByMCParticle.clear();

  m_reassignedSecondaryHits.clear();
  m_reassignedSecondarySimHits.clear();
}

void CDCMCMap::fill()
{
  B2DEBUG(100, "In CDCMCMap::fill()");
  clear();

  fillSimHitByHitMap();
  fillMCParticleByHitMap();
  fillMCParticleBySimHitMap();

  validateRelations();
  validateReassignedSecondaries();

  B2DEBUG(100, "m_simHitsByHit.size(): " << m_simHitsByHit.size());

  B2DEBUG(100, "m_hitsByMCParticle.size(): " << m_hitsByMCParticle.size());
  B2DEBUG(100, "m_simHitsByMCParticle.size(): " << m_simHitsByMCParticle.size());

  B2DEBUG(100, "m_reassignedSecondaryHits.size(): " << m_reassignedSecondaryHits.size());
  B2DEBUG(100, "m_reassignedSecondarySimHits.size(): " << m_reassignedSecondarySimHits.size());
}

void CDCMCMap::fillSimHitByHitMap()
{
  StoreArray<CDCSimHit> simHits;
  StoreArray<CDCHit> hits;

  // Pickup an iterator for hinted insertion
  auto itInsertHint = m_simHitsByHit.end();

  for (const CDCSimHit& simHit : simHits) {
    const CDCSimHit* ptrSimHit = &simHit;
    RelationVector<CDCHit> relatedHits = simHit.getRelationsTo<CDCHit>();

    int nRelatedHits = relatedHits.size();
    if (nRelatedHits > 1) {
      B2WARNING("CDCSimHit as more than one related CDCHit - reorganize the mapping");
    }

    for (const CDCHit& hit : relatedHits) {
      const CDCHit* ptrHit = &hit;

      if (m_simHitsByHit.count(ptrHit) != 0) {
        B2WARNING("CDCHit as more than one related CDCSimHit - reorganize the mapping");
      }

      itInsertHint = m_simHitsByHit.insert(itInsertHint, {ptrHit, ptrSimHit});
    }
  }

  // Check if every hit has a corresponding simulated hit
  for (const CDCHit& hit : hits) {
    const CDCHit* ptrHit = &hit;

    if (m_simHitsByHit.count(ptrHit) == 0) {
      B2WARNING("CDCHit has no related CDCSimHit in CDCMCMap::fill()");
    }
  }
}

void CDCMCMap::fillMCParticleByHitMap()
{
  StoreArray<MCParticle> mcParticles;
  StoreArray<CDCHit> hits;

  std::map<const MCParticle*, std::vector<const CDCSimHit*>> primarySimHitsByMCParticle;

  for (const CDCHit& hit : hits) {
    const CDCHit* ptrHit = &hit;
    RelationVector<MCParticle> relatedMCParticles = hit.getRelationsFrom<MCParticle>();

    const int nRelatedMCParticles = relatedMCParticles.size();

    if (nRelatedMCParticles == 0 and not isBackground(ptrHit)) {
      B2WARNING("CDCHit has no related MCParticle but CDCHit indicates that it is no "
                "background in CDCMCMap::fill()");
    }

    if (nRelatedMCParticles > 1) {
      B2WARNING("CDCHit as more than one related MCParticle - reorganize the mapping");
    }

    // Pickup an iterator for hinted insertion
    auto itInsertHint = m_hitsByMCParticle.end();
    for (int iRelatedMCParticle = 0; iRelatedMCParticle < nRelatedMCParticles; ++iRelatedMCParticle) {
      const MCParticle* ptrMCParticle = relatedMCParticles[iRelatedMCParticle];
      double weight = relatedMCParticles.weight(iRelatedMCParticle);

      if (indicatesReassignedSecondary(weight)) {
        m_reassignedSecondaryHits.insert(ptrHit);
      } else {
        const CDCSimHit* ptrSimHit = ptrHit->getRelatedFrom<CDCSimHit>();
        if (ptrMCParticle->isPrimaryParticle() and ptrSimHit) {
          primarySimHitsByMCParticle[ptrMCParticle].push_back(ptrSimHit);
        }
      }

      itInsertHint = m_hitsByMCParticle.insert(itInsertHint, {ptrMCParticle, ptrHit});
    }
  }

  // Check time ordering of primary hits
  int nSortedIncorretly = 0;
  auto lessFlightTime = [](const CDCSimHit * lhs, const CDCSimHit * rhs) {
    return lhs->getFlightTime() < rhs->getFlightTime();
  };

  auto lessArrayIndex = [](const CDCSimHit * lhs, const CDCSimHit * rhs) -> bool {
    return lhs->getArrayIndex() < rhs->getArrayIndex();
  };

  for (std::pair<const MCParticle* const, std::vector<const CDCSimHit*>>&
       primarySimHitsForMCParticle : primarySimHitsByMCParticle) {

    const MCParticle* ptrMCParticle = primarySimHitsForMCParticle.first;
    std::vector<const CDCSimHit*>& simHits = primarySimHitsForMCParticle.second;
    std::sort(simHits.begin(), simHits.end(), lessArrayIndex);
    auto itSorted = std::is_sorted_until(simHits.begin(), simHits.end(), lessFlightTime);
    if (itSorted != simHits.end()) {
      ++nSortedIncorretly;
      B2DEBUG(100,
              "CDCSimHits for MCParticle " << ptrMCParticle->getArrayIndex()
              << " only sorted correctly up to hit number "
              << std::distance(simHits.begin(), itSorted));
      --itSorted;
      B2DEBUG(100,
              "Between wire " << (*itSorted)->getWireID() << " " << (*itSorted)->getFlightTime()
              << "ns "
              << (*itSorted)->getArrayIndex());
      ++itSorted;
      B2DEBUG(100,
              "and wire " << (*itSorted)->getWireID() << " " << (*itSorted)->getFlightTime()
              << "ns "
              << (*itSorted)->getArrayIndex());
    }
  }
  if (nSortedIncorretly) {
    B2WARNING("(BII-2136) CDCSimHits for "
              << nSortedIncorretly
              << " primary mc particles are not sorted correctly by their time of flight");
  }
}

void CDCMCMap::fillMCParticleBySimHitMap()
{
  StoreArray<MCParticle> mcParticles;
  StoreArray<CDCSimHit> simHits;

  for (const CDCSimHit& simHit : simHits) {
    const CDCSimHit* ptrSimHit = &simHit;
    RelationVector<MCParticle> relatedMCParticles = simHit.getRelationsFrom<MCParticle>();

    const int nRelatedMCParticles = relatedMCParticles.size();

    if (nRelatedMCParticles == 0 and not isBackground(ptrSimHit)) {
      B2WARNING("CDCSimHit has no related MCParticle but CDCSimHit indicates that it is no "
                "background in CDCMCMap::fill()");
    }

    if (nRelatedMCParticles > 1) {
      B2WARNING("CDCSimHit as more than one related MCParticle - reorganize the mapping");
    }

    // Pickup an iterator for hinted insertion
    auto itInsertHint = m_simHitsByMCParticle.end();
    for (int iRelatedMCParticle = 0; iRelatedMCParticle < nRelatedMCParticles; ++iRelatedMCParticle) {
      const MCParticle* ptrMCParticle = relatedMCParticles[iRelatedMCParticle];
      double weight = relatedMCParticles.weight(iRelatedMCParticle);

      if (indicatesReassignedSecondary(weight)) {
        m_reassignedSecondarySimHits.insert(ptrSimHit);
      }

      itInsertHint = m_simHitsByMCParticle.insert(itInsertHint, {ptrMCParticle, ptrSimHit});
    }
  }
}

void CDCMCMap::validateRelations() const
{
  StoreArray<CDCHit> hits;

  for (const CDCHit& hit : hits) {
    const CDCHit* ptrHit = &hit;

    const CDCSimHit* ptrSimHit = getSimHit(ptrHit);
    const MCParticle* ptrMCParticle = getMCParticle(ptrHit);

    const MCParticle* ptrMCParticleFromSimHit = getMCParticle(ptrSimHit);

    if (ptrMCParticle != ptrMCParticleFromSimHit) {
      B2WARNING("MCParticle from CDCHit and MCParticle from related CDCSimHit mismatch in "
                "CDCMCMap::validateRelations()");
    }
  }
}

void CDCMCMap::validateReassignedSecondaries() const
{
  for (const CDCHit* ptrHit : m_reassignedSecondaryHits) {

    const CDCSimHit* ptrSimHit = getSimHit(ptrHit);
    if (not isReassignedSecondary(ptrSimHit)) {
      B2WARNING("CDCHit is reassigned secondary but related CDCSimHit is not.");
    }
  }
}

MayBePtr<const CDCSimHit> CDCMCMap::getSimHit(const CDCHit* hit) const
{
  return hit ? hit->getRelated<CDCSimHit>() : nullptr;
}

MayBePtr<const CDCHit> CDCMCMap::getHit(const CDCSimHit* simHit) const
{
  return simHit ? simHit->getRelated<CDCHit>() : nullptr;
}

bool CDCMCMap::isBackground(const CDCSimHit* simHit) const
{
  return simHit ? simHit->getBackgroundTag() != CDCSimHit::bg_none : false;
}

bool CDCMCMap::isBackground(const CDCHit* hit) const
{
  return isBackground(getSimHit(hit));
}

MayBePtr<const MCParticle> CDCMCMap::getMCParticle(const CDCHit* hit) const
{
  return hit ? hit->getRelated<MCParticle>() : nullptr;
}

MayBePtr<const MCParticle> CDCMCMap::getMCParticle(const CDCSimHit* simHit) const
{
  return simHit ? simHit->getRelated<MCParticle>() : nullptr;
}
