/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <algorithm>

#include "../include/WireHitCreator.h"

//#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

WireHitCreator::WireHitCreator() {;}

WireHitCreator::~WireHitCreator() {;}

void WireHitCreator::create(StoreArray <CDCHit>& cdcHits, CDCWireHitCollection& cdcWireHitCollection) const
{

  //clear all wirehits which may have been present from former events
  cdcWireHitCollection.clear();

  //create the wirehits into a collection
  size_t nHits = cdcHits.getEntries();
  cdcWireHitCollection.reserve(nHits);

  for (size_t iHit = 0; iHit < nHits; ++iHit) {

    CDCHit* hit = cdcHits[iHit];
    cdcWireHitCollection.push_back(CDCWireHit(hit, iHit));

  }

  cdcWireHitCollection.sort();
  if (not cdcWireHitCollection.checkSorted())
  { B2WARNING("Wirehits are not sorted after creation"); }

}








