/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <algorithm>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <framework/datastore/StoreArray.h>


#include <framework/logging/Logger.h>

#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <unordered_set>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCWireHitTopology& CDCWireHitTopology::getInstance()
{
  StoreObjPtr<CDCWireHitTopology> instance("", DataStore::c_Persistent);
  if (not instance.isValid()) {
    B2FATAL("CDCWireHitTopology could not by found in the StoreArray! Have you added the WireHitTopologyPreparer module?");
  }

  return *instance;
}

void CDCWireHitTopology::initialize()
{
  StoreObjPtr<CDCWireHitTopology> wireHitTopology("", DataStore::c_Persistent);
  if (wireHitTopology.isValid()) {
    return;
  }
  wireHitTopology.registerInDataStore(DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  if (not wireHitTopology.create(false)) {
    B2FATAL("Could not create the StoreObject for the Wire Hit Topology properly!");
  }
}

namespace {
  /** Checks if the sougth item is in the range and jumps to it.
   *  Falls back to std::lower_bound if not found like this
   *  Assumes that the range has only unique items,
   *  hence if it the item located in memory within the range
   *  it is also the correct lower bound.
   */
  template<class AIt, class T>
  AIt unique_lower_bound_fast(const AIt& first, const AIt& last, const T& t)
  {
    AIt itT(&t);
    if (first <= itT and itT < last) {
      return itT;
    } else {
      return std::lower_bound(first, last, t);
    }
  }
}


CDCWireHitTopology::CDCWireHitTopology() :
  m_initialTDCCountTranslator(static_cast<CDC::TDCCountTranslatorBase*>(new CDC::RealisticTDCCountTranslator()))
{
}

CDCWireHitTopology::~CDCWireHitTopology()
{
  if (m_initialTDCCountTranslator) {
    delete m_initialTDCCountTranslator;
    m_initialTDCCountTranslator = nullptr;
  }
}

size_t CDCWireHitTopology::useAll()
{
  // Unblock every wire hit
  for (const CDCWireHit& wireHit : getWireHits()) {
    wireHit.getAutomatonCell().unsetTakenFlag();
  }

  return getWireHits().size();
}

size_t CDCWireHitTopology::dontUse(const std::vector<int>& iHits)
{
  StoreArray<CDCHit> storedHits;
  int nHits = storedHits.getEntries();

  size_t nBlockedHits = 0;
  for (const int iHit : iHits) {
    if (iHit >= 0 and iHit < nHits) {
      const CDCHit* ptrHit = storedHits[iHit];
      const CDCWireHit* ptrWireHit = getWireHit(ptrHit);
      if (ptrWireHit) {
        const CDCWireHit& wireHit = *ptrWireHit;
        if (wireHit.getAutomatonCell().hasTakenFlag()) {
          wireHit.getAutomatonCell().setTakenFlag();
          ++nBlockedHits;
        }
      } else {
        B2WARNING("No CDCWireHit for CDCHit");
      }
    } else {
      B2WARNING("Hit index out of bounds");
    }
  }

  return nBlockedHits;
}

size_t CDCWireHitTopology::useAllBut(const std::vector<int>& iHits)
{
  size_t nHits = useAll();
  size_t nBlockedHits = dontUse(iHits);
  return nHits - nBlockedHits;
}

size_t CDCWireHitTopology::useOnly(const std::vector<int>& iHits)
{
  // Block every wire hit first
  for (const CDCWireHit& wireHit : getWireHits()) {
    wireHit.getAutomatonCell().setTakenFlag();
  }

  StoreArray<CDCHit> storedHits;
  int nHits = storedHits.getEntries();

  for (const int iHit : iHits) {
    if (iHit >= 0 and iHit < nHits) {
      const CDCHit* ptrHit = storedHits[iHit];
      const CDCWireHit* ptrWireHit = getWireHit(ptrHit);
      if (ptrWireHit) {
        const CDCWireHit& wireHit = *ptrWireHit;
        wireHit.getAutomatonCell().unsetTakenFlag();
      } else {
        B2WARNING("No CDCWireHit for CDCHit");
      }
    } else {
      B2WARNING("Hit index out of bounds");
    }
  }

  return iHits.size();
}

std::vector<int> CDCWireHitTopology::getIHitsRelatedFrom(const std::string& storeArrayName) const
{
  std::vector<int> iHits;

  StoreArray<CDCHit> storedHits;
  int nHits = storedHits.getEntries();

  for (Index iHit = 0; iHit < nHits; ++iHit) {
    const CDCHit* ptrHit = storedHits[iHit];
    if (ptrHit) {
      const CDCHit& hit = *ptrHit;
      TObject* ptrRelatedObj = hit.getRelatedFrom<TObject>(storeArrayName);
      if (ptrRelatedObj) {
        iHits.push_back(iHit);
      }
    }
  }
  return iHits;
}

void CDCWireHitTopology::fill(const std::vector<CDCWireHit>& wireHits)
{
  // Copy the wire hits
  m_wireHits = wireHits;

  std::sort(m_wireHits.begin(), m_wireHits.end());

  if (not std::is_sorted(m_wireHits.begin(), m_wireHits.end())) {
    B2ERROR("Wire hits are not sorted after creation");
  }
}


void CDCWireHitTopology::clear()
{
  m_wireHits.clear();
}

/// Getter for the wire hit that is based on the given CDCHit.
const CDCWireHit* CDCWireHitTopology::getWireHit(const CDCHit* ptrHit) const
{
  if (not ptrHit) return nullptr;
  const CDCHit& hit = *ptrHit;

  CDCWireHitRange wireHitRange = getWireHits(hit);
  if (wireHitRange.empty()) {
    B2WARNING("No CDCWireHit for the CDCHit in the CDCWireHitTopology.");
    return nullptr;
  }

  const CDCWireHit& wireHit =  wireHitRange.front();
  return &wireHit;

}
