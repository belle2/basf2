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
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


namespace {
  CDCWireHitTopology* g_wireHitTopology = nullptr;
}

CDCWireHitTopology& CDCWireHitTopology::getInstance()
{
  if (not g_wireHitTopology) g_wireHitTopology = new CDCWireHitTopology;
  return *g_wireHitTopology;
}

size_t CDCWireHitTopology::fill(const std::string& cdcHitsStoreArrayName)
{
  //clear all wire hits and oriented wire hits that might be left over from the last event
  clear();

  // get the relevant cdc hits from the datastore
  StoreArray<CDCHit> cdcHits(cdcHitsStoreArrayName);

  //create the wirehits into a collection
  size_t nHits = cdcHits.getEntries();
  m_wireHits.reserve(nHits);
  m_rlWireHits.reserve(2 * nHits);

  for (size_t iHit = 0; iHit < nHits; ++iHit) {
    CDCHit* ptrHit = cdcHits[iHit];
    CDCHit& hit = *ptrHit;
    if (iHit != hit.getArrayIndex()) {
      B2ERROR("CDCHit.getArrayIndex() produced wrong result. Expected : " << iHit << " Actual : " << hit.getArrayIndex());
    }

    m_wireHits.push_back(CDCWireHit(ptrHit, iHit));

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

  for (const CDCWireHit & wireHit : m_wireHits) {
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

const Belle2::CDCLocalTracking::CDCRLWireHit* CDCWireHitTopology::getRLWireHit(const Belle2::CDCLocalTracking::CDCWireHit& wireHit, const RightLeftInfo& rlInfo) const
{

  std::pair<const CDCRLWireHit*, const CDCRLWireHit*> rlWireHitPair = getRLWireHitPair(wireHit);
  if (rlInfo == LEFT) {
    return rlWireHitPair.first;
  } else {
    return rlWireHitPair.second;
  }

}


const Belle2::CDCLocalTracking::CDCRLWireHit* CDCWireHitTopology::getRLWireHit(const Belle2::CDCHit* ptrHit, const RightLeftInfo& rlInfo) const
{
  const CDCWireHit* ptrWireHit = getWireHit(ptrHit);
  return  ptrWireHit ? getRLWireHit(*ptrWireHit, rlInfo) : nullptr;
}
