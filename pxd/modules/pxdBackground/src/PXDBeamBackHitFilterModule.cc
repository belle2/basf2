/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdBackground/PXDBeamBackHitFilterModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <functional>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDBeamBackHitFilter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDBeamBackHitFilterModule::PXDBeamBackHitFilterModule() : Module()
{
  //Set module properties
  setDescription("This module filters out from the BeamBackHits StoreArray the BeamBackHits not related to PXD.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void PXDBeamBackHitFilterModule::initialize()
{
  //Register collections
  StoreArray<BeamBackHit> storeBeamBackHits("");
  storeBeamBackHits.isOptional();
}

void PXDBeamBackHitFilterModule::event()
{
  StoreArray<BeamBackHit> storeBeamBackHits("");
  // If no BeamBackHits, nothing to do
  if (!storeBeamBackHits || !storeBeamBackHits.getEntries()) return;

  // Now we loop over BeamBackHits and only leave PXD-related BeamBackHits.
  int nBBHits = storeBeamBackHits.getEntries();
  int lastLeft(0);
  for (int iHit = 0; iHit < nBBHits; ++iHit) {
    const BeamBackHit& hit = *storeBeamBackHits[iHit];
    if (hit.getSubDet() == 1) {
      // Don't copy to itself
      if (iHit > lastLeft) *storeBeamBackHits[lastLeft] = hit;
      lastLeft++;
    }
  }

  //Resize if we omitted one or more elements
  storeBeamBackHits.getPtr()->ExpandCreate(lastLeft);
}
