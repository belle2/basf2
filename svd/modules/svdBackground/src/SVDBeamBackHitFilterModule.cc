/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdBackground/SVDBeamBackHitFilterModule.h>

#include <simulation/dataobjects/BeamBackHit.h>

#include <framework/datastore/StoreArray.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDBeamBackHitFilter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDBeamBackHitFilterModule::SVDBeamBackHitFilterModule() : Module()
{
  //Set module properties
  setDescription("This module filters out from the BeamBackHits StoreArray the BeamBackHits not related to SVD.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void SVDBeamBackHitFilterModule::initialize()
{
  //Register collections
  StoreArray<BeamBackHit> storeBeamBackHits("");
  storeBeamBackHits.isOptional();
}

void SVDBeamBackHitFilterModule::event()
{
  StoreArray<BeamBackHit> storeBeamBackHits("");
  // If no BeamBackHits, nothing to do
  if (!storeBeamBackHits || !storeBeamBackHits.getEntries()) return;

  // Now we loop over BeamBackHits and only leave SVD-related BeamBackHits.
  int nBBHits = storeBeamBackHits.getEntries();
  int lastLeft(0);
  for (int iHit = 0; iHit < nBBHits; ++iHit) {
    const BeamBackHit& hit = *storeBeamBackHits[iHit];
    if (hit.getSubDet() == 2) {
      // Don't copy to itself
      if (iHit > lastLeft) *storeBeamBackHits[lastLeft] = hit;
      lastLeft++;
    }
  }

  //Resize if we omitted one or more elements
  storeBeamBackHits.getPtr()->ExpandCreate(lastLeft);
}
