/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <cdc/dataobjects/CDCHit.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>
#include <algorithm>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCWireHitTopology& CDCWireHitTopology::getInstance()
{
  StoreObjPtr<CDCWireHitTopology> instance("", DataStore::c_Persistent);
  if (not instance.isValid()) {
    B2FATAL("CDCWireHitTopology could not be found in the StoreArray! Have you added the WireHitTopologyPreparer module?");
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
    B2FATAL("Could not create the StoreObject for the WireHitTopology properly!");
  }
}

void CDCWireHitTopology::fill(std::shared_ptr<ConstVectorRange<CDCWireHit> > wireHits)
{
  B2ASSERT("Nullptr served to CDCWireHitTopology::fill",
           wireHits);

  m_wireHits = std::move(wireHits);

  B2ASSERT("Expect the wire hits to be sorted",
           std::is_sorted(getWireHits().begin(), getWireHits().end()));
}

/// Getter for the wire hit that is based on the given CDCHit.
const CDCWireHit* CDCWireHitTopology::getWireHit(const CDCHit* ptrHit) const
{
  if (not ptrHit) return nullptr;
  const CDCHit& hit = *ptrHit;

  ConstVectorRange<CDCWireHit> wireHitRange =
    std::equal_range(getWireHits().begin(), getWireHits().end(), hit);

  if (wireHitRange.empty()) {
    B2WARNING("No CDCWireHit for the CDCHit in the CDCWireHitTopology.");
    return nullptr;
  }

  const CDCWireHit& wireHit =  wireHitRange.front();
  return &wireHit;

}
