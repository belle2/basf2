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

#include "../include/CDCWireHitTopology.h"

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/logging/Logger.h>

#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

#include<unordered_set>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


namespace {
  CDCWireHitTopology* g_wireHitTopology = nullptr;
}

CDCWireHitTopology& CDCWireHitTopology::getInstance()
{
  if (not g_wireHitTopology) g_wireHitTopology = new CDCWireHitTopology;
  return *g_wireHitTopology;
}

namespace {
  bool maximalOneHitPerWire(StoreArray<CDCHit>& storedHits)
  {
    unordered_set<unsigned short> hitEWires;

    for (const CDCHit& hit : storedHits) {
      unsigned short eWire = hit.getID();
      if (hitEWires.count(eWire)) {
        return false;
      } else {
        hitEWires.insert(eWire);
      }
    }
    return true;
  }

}


CDCWireHitTopology::CDCWireHitTopology() :
  m_eventMetaData(-999, -999, -999),
  m_useSimpleTDCCountTranslator(false),
  m_initialTDCCountTranslator(m_useSimpleTDCCountTranslator ?
                              static_cast<CDC::TDCCountTranslatorBase*>(new CDC::SimpleTDCCountTranslator()) :
                              static_cast<CDC::TDCCountTranslatorBase*>(new CDC::RealisticTDCCountTranslator())
                             )
{
}

CDCWireHitTopology::~CDCWireHitTopology()
{
  if (m_initialTDCCountTranslator) {
    delete m_initialTDCCountTranslator;
    m_initialTDCCountTranslator = nullptr;
  }
}

void CDCWireHitTopology::initialize()
{
  StoreObjPtr<EventMetaData>::required();
  StoreArray<CDCHit>::required();
}


size_t CDCWireHitTopology::event()
{
  StoreObjPtr<EventMetaData> storedEventMetaData;

  if (storedEventMetaData.isValid() and m_eventMetaData == *storedEventMetaData) {
    return m_wireHits.size();
  } else {
    size_t nHits = fill("");
    m_eventMetaData = *storedEventMetaData;
    return nHits;
  }

}

size_t CDCWireHitTopology::useAll()
{
  // Ensure the store array is filled with the basic hits.
  event();

  // Unblock every wire hit
  for (const CDCWireHit& wireHit : m_wireHits) {
    wireHit.getAutomatonCell().unsetTakenFlag();
  }

  return m_wireHits.size();
}

size_t CDCWireHitTopology::dontUse(const std::vector<int>& iHits)
{
  // Ensure the store array is filled with the basic hits.
  event();

  StoreArray<CDCHit> storedHits;
  int nHits = storedHits.getEntries();

  size_t nBlockedHits = 0;
  for (const int& iHit : iHits) {
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
  // Ensure the store array is filled with the basic hits.
  event();

  // Block every wire hit first
  for (const CDCWireHit& wireHit : m_wireHits) {
    wireHit.getAutomatonCell().setTakenFlag();
  }

  StoreArray<CDCHit> storedHits;
  int nHits = storedHits.getEntries();

  for (const int& iHit : iHits) {
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

size_t CDCWireHitTopology::fill(const std::string& cdcHitsStoreArrayName)
{
  // clear all wire hits and oriented wire hits that might be left over from the last event
  clear();

  // Refresh the TDC count translator?
  if (m_initialTDCCountTranslator) {
    delete m_initialTDCCountTranslator;
    m_initialTDCCountTranslator = nullptr;
  }
  if (m_useSimpleTDCCountTranslator) {
    m_initialTDCCountTranslator = new CDC::SimpleTDCCountTranslator();
  } else {
    m_initialTDCCountTranslator = new CDC::RealisticTDCCountTranslator();
  }


  // get the relevant cdc hits from the datastore
  StoreArray<CDCHit> cdcHits(cdcHitsStoreArrayName);

  assert(maximalOneHitPerWire(cdcHits));

  //create the wirehits into a collection
  Index nHits = cdcHits.getEntries();
  m_wireHits.reserve(nHits);
  m_rlWireHits.reserve(2 * nHits);

  for (Index iHit = 0; iHit < nHits; ++iHit) {
    CDCHit* ptrHit = cdcHits[iHit];
    CDCHit& hit = *ptrHit;
    if (iHit != hit.getArrayIndex()) {
      B2ERROR("CDCHit.getArrayIndex() produced wrong result. Expected : " << iHit << " Actual : " << hit.getArrayIndex());
    }

    m_wireHits.push_back(CDCWireHit(ptrHit, m_initialTDCCountTranslator));

    const WireID wireID(hit.getID());
    if (wireID.getEWire() != hit.getID()) {
      B2ERROR("WireID.getEWire() differs from CDCHit.getID()");
    }

    const CDCWireHit& wireHit = m_wireHits.back();
    if (hit.getID() != wireHit.getWire().getEWire()) {
      B2ERROR("CDCHit.getID() differs from CDCWireHit.getWire().getEWire()");
    }
    if (hit.getArrayIndex() != wireHit.getStoreIHit()) {
      B2ERROR("CDCHit.getArrayIndex() differs from CDCWireHit.getStoreIHit");
    }
  }

  m_wireHits.ensureSorted();
  if (not m_wireHits.checkSorted()) {
    B2ERROR("Wire hits are not sorted after creation");
  }

  for (const CDCWireHit& wireHit : m_wireHits) {
    m_rlWireHits.push_back(CDCRLWireHit(&wireHit, LEFT));
    m_rlWireHits.push_back(CDCRLWireHit(&wireHit, RIGHT));
  }

  if (not m_rlWireHits.checkSorted()) {
    B2ERROR("Oriented wire hits are not sorted after creation");
  }

  return nHits;
}


void CDCWireHitTopology::clear()
{
  m_rlWireHits.clear();
  m_wireHits.clear();
}


const CDCRLWireHit* CDCWireHitTopology::getReverseOf(const CDCRLWireHit& rlWireHit) const
{

  SortableVector<CDCRLWireHit>::const_iterator itFoundRLWireHit = m_rlWireHits.findFast(rlWireHit);
  if (itFoundRLWireHit == m_rlWireHits.end()) {
    B2ERROR("An oriented wire hit can not be found in the CDCWireHitTopology.");
    return nullptr;
  }
  if (rlWireHit.getRLInfo() == RIGHT) {
    // The oriented wire hit with left passage is stored in the vector just before this one -- see above in fill()
    --itFoundRLWireHit;

  } else if (rlWireHit.getRLInfo() == LEFT) {
    // The oriented wire hit with right passage is stored in the vector just after this one -- see above in fill()
    ++itFoundRLWireHit;
  } else {
    B2ERROR("An oriented wire hit does not have the right left passage variable assigned correctly.");
  }

  return &*itFoundRLWireHit;

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





std::pair<const CDCRLWireHit*, const CDCRLWireHit*> CDCWireHitTopology::getRLWireHitPair(const CDCWireHit& wireHit) const
{

  SortableVector<CDCWireHit>::const_iterator itFoundWireHit = m_wireHits.findFast(wireHit);

  if (itFoundWireHit == m_wireHits.end()) {
    B2ERROR("A wire hit can not be found in the CDCWireHitTopology.");
    return std::pair<const CDCRLWireHit*, const CDCRLWireHit*>(nullptr, nullptr);
  } else {

    size_t idxWireHit = std::distance(m_wireHits.begin(), itFoundWireHit);

    size_t idxLeftWireHit = 2 * idxWireHit;
    size_t idxRightWireHit = idxLeftWireHit + 1;

    const CDCRLWireHit& leftWireHit = m_rlWireHits[idxLeftWireHit];
    const CDCRLWireHit& rightWireHit = m_rlWireHits[idxRightWireHit];
    return std::pair<const CDCRLWireHit*, const CDCRLWireHit*>(&leftWireHit, &rightWireHit);
  }
}

const Belle2::TrackFindingCDC::CDCRLWireHit* CDCWireHitTopology::getRLWireHit(const Belle2::TrackFindingCDC::CDCWireHit& wireHit,
    const RightLeftInfo& rlInfo) const
{

  std::pair<const CDCRLWireHit*, const CDCRLWireHit*> rlWireHitPair = getRLWireHitPair(wireHit);
  if (rlInfo == LEFT) {
    return rlWireHitPair.first;
  } else {
    return rlWireHitPair.second;
  }

}


const Belle2::TrackFindingCDC::CDCRLWireHit* CDCWireHitTopology::getRLWireHit(const Belle2::CDCHit* ptrHit,
    const RightLeftInfo& rlInfo) const
{
  const CDCWireHit* ptrWireHit = getWireHit(ptrHit);
  return  ptrWireHit ? getRLWireHit(*ptrWireHit, rlInfo) : nullptr;
}
