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

ClassImp(CDCWireHitTopology);

CDCWireHitTopology& CDCWireHitTopology::getInstance()
{
  static StoreObjPtr<CDCWireHitTopology> instance("", DataStore::c_Persistent);
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
  template<class It, class T>
  It unique_lower_bound_fast(const It& first, const It& last, const T& t)
  {
    It itT(&t);
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

  m_rlWireHits.reserve(m_rlWireHits.size() + 2 * wireHits.size());
  for (const CDCWireHit& wireHit : m_wireHits) {
    m_rlWireHits.push_back(CDCRLWireHit(&wireHit, ERightLeft::c_Left));
    m_rlWireHits.push_back(CDCRLWireHit(&wireHit, ERightLeft::c_Right));
  }

  if (not std::is_sorted(m_rlWireHits.begin(), m_rlWireHits.end())) {
    B2ERROR("Oriented wire hits are not sorted after creation");
  }
}


void CDCWireHitTopology::clear()
{
  m_rlWireHits.clear();
  m_wireHits.clear();
}


const CDCRLWireHit*
CDCWireHitTopology::getReverseOf(const CDCRLWireHit& rlWireHit) const
{
  auto itRLWireHit = unique_lower_bound_fast(m_rlWireHits.begin(),
                                             m_rlWireHits.end(),
                                             rlWireHit);

  if (itRLWireHit == m_rlWireHits.end()) {
    B2ERROR("An oriented wire hit can not be found in the CDCWireHitTopology.");
    return nullptr;
  }
  if (rlWireHit.getRLInfo() == ERightLeft::c_Right) {
    // The oriented wire hit with left passage is stored in the vector just before this one -- see above in fill()
    --itRLWireHit;

  } else if (rlWireHit.getRLInfo() == ERightLeft::c_Left) {
    // The oriented wire hit with right passage is stored in the vector just after this one -- see above in fill()
    ++itRLWireHit;
  } else {
    B2ERROR("An oriented wire hit does not have the right left passage variable assigned correctly.");
  }

  return &*itRLWireHit;

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





std::pair<const CDCRLWireHit*, const CDCRLWireHit*>
CDCWireHitTopology::getRLWireHitPair(const CDCWireHit& wireHit) const
{
  auto itWireHit = unique_lower_bound_fast(m_wireHits.begin(), m_wireHits.end(), wireHit);

  if (itWireHit == m_wireHits.end()) {
    B2ERROR("A wire hit can not be found in the CDCWireHitTopology.");
    return std::pair<const CDCRLWireHit*, const CDCRLWireHit*>(nullptr, nullptr);
  }

  size_t idxWireHit = std::distance(m_wireHits.begin(), itWireHit);

  size_t idxLeftWireHit = 2 * idxWireHit;
  size_t idxRightWireHit = idxLeftWireHit + 1;

  const CDCRLWireHit& leftWireHit = m_rlWireHits[idxLeftWireHit];
  const CDCRLWireHit& rightWireHit = m_rlWireHits[idxRightWireHit];
  return std::pair<const CDCRLWireHit*, const CDCRLWireHit*>(&leftWireHit, &rightWireHit);
}

const CDCRLWireHit* CDCWireHitTopology::getRLWireHit(const CDCWireHit& wireHit,
                                                     const ERightLeft rlInfo) const
{

  std::pair<const CDCRLWireHit*, const CDCRLWireHit*> rlWireHitPair = getRLWireHitPair(wireHit);
  if (rlInfo == ERightLeft::c_Left) {
    return rlWireHitPair.first;
  } else {
    return rlWireHitPair.second;
  }

}

const CDCRLWireHit* CDCWireHitTopology::getRLWireHit(const Belle2::CDCHit* ptrHit,
                                                     const ERightLeft rlInfo) const
{
  const CDCWireHit* ptrWireHit = getWireHit(ptrHit);
  return  ptrWireHit ? getRLWireHit(*ptrWireHit, rlInfo) : nullptr;
}
