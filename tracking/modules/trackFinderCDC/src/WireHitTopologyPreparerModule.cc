#include <tracking/modules/trackFinderCDC/WireHitTopologyPreparerModule.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/CDCHit.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <unordered_set>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(WireHitTopologyPreparer);

namespace {
  /// Helper function
  bool maximalOneHitPerWire(StoreArray<CDCHit>& storedHits)
  {
    std::unordered_set<unsigned short> hitEWires;

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

void WireHitTopologyPreparerModule::initialize()
{
  StoreArray<CDCHit>::required();
  CDCWireHitTopology::initialize();

  // Preload geometry during initialization
  // Marked as unused intentionally to avoid a compile warning
  CDC::CDCGeometryPar& cdcGeo __attribute__((unused)) = CDC::CDCGeometryPar::Instance();
  CDCWireTopology& wireTopology  __attribute__((unused)) = CDCWireTopology::getInstance();
}

void WireHitTopologyPreparerModule::event()
{
  StoreArray<CDCHit> cdcHits("");
  assert(maximalOneHitPerWire(cdcHits));

  // Create the wirehits - translate the CDCHits and attach the geometry.
  B2DEBUG(100, "Creating all CDCWireHits");
  CDC::RealisticTDCCountTranslator tdcCountTranslator;

  CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  //create the wirehits into a collection
  Index useNHits = cdcHits.getEntries();
  std::vector<CDCWireHit> wireHits;
  wireHits.reserve(useNHits);

  for (Index iHit = 0; iHit < useNHits; ++iHit) {
    CDCHit* ptrHit = cdcHits[iHit];
    CDCHit& hit = *ptrHit;
    if (iHit != hit.getArrayIndex()) {
      B2ERROR("CDCHit.getArrayIndex() produced wrong result. Expected : " << iHit << " Actual : " << hit.getArrayIndex());
    }

    wireHits.push_back(CDCWireHit(ptrHit, &tdcCountTranslator));

    const WireID wireID(hit.getID());
    if (wireID.getEWire() != hit.getID()) {
      B2ERROR("WireID.getEWire() differs from CDCHit.getID()");
    }

    const CDCWireHit& wireHit = wireHits.back();
    if (hit.getID() != wireHit.getWire().getEWire()) {
      B2ERROR("CDCHit.getID() differs from CDCWireHit.getWire().getEWire()");
    }
    if (hit.getArrayIndex() != wireHit.getStoreIHit()) {
      B2ERROR("CDCHit.getArrayIndex() differs from CDCWireHit.getStoreIHit");
    }
  }

  std::sort(wireHits.begin(), wireHits.end());
  std::shared_ptr<std::vector<CDCWireHit> > sharedWireHits{new std::vector<CDCWireHit>(std::move(wireHits))};
  wireHitTopology.fill(std::move(sharedWireHits));

  B2DEBUG(100, "  Created number of CDCWireHits == " << useNHits);
  B2DEBUG(100, "  Number of usable CDCWireHits == " << useNHits);

  if (useNHits == 0) {
    B2WARNING("Event with no hits");
  }
}
