/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcrecohitmaker/CDCRecoHitMakerModule.h>
#include <framework/datastore/SimpleVec.h>
#include <cdc/hitcdc/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <TH1F.h>
#include <TNtuple.h>
#include <TCanvas.h>
#include <iostream>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/dataobjects/Relation.h>
using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCRecoHitMaker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCRecoHitMakerModule::CDCRecoHitMakerModule() : Module()
{
  //Set module properties
  setDescription("Prints the current event meta data information (exp, run, evt).");

  //Parameter definition
}


CDCRecoHitMakerModule::~CDCRecoHitMakerModule()
{

}


void CDCRecoHitMakerModule::initialize()
{
}


void CDCRecoHitMakerModule::beginRun()
{
}


void CDCRecoHitMakerModule::event()
{
  StoreArray<Relation> arraySimHitToCDCHit("SimHitToCDCHitCollection");
  StoreObjPtr<SimpleVec<float> > c1("ResolutionCanvas", DataStore::c_Persistent);

  std::vector<float> myvector(arraySimHitToCDCHit.GetEntries());
  /*
    for (int ii = 0; ii < arraySimHitToCDCHit.GetEntries(); ii++) {
      CDCSimHit* simhitptr = static_cast<CDCSimHit*>(arraySimHitToCDCHit[ii]->getFrom());
      if (!simhitptr) {B2WARNING("Should not work");}


      float trueDriftTime      = (static_cast<CDCSimHit*>(arraySimHitToCDCHit[ii]->getFrom()))->getDriftLength();
      float simulatedDriftTime = (static_cast<CDCHit*>(arraySimHitToCDCHit[ii]->getTo()))->getDriftTime();
      myvector[ii] = simulatedDriftTime - trueDriftTime;
      B2WARNING("True: " << trueDriftTime);
      B2WARNING("Simulated: " << simulatedDriftTime);
      B2WARNING("Simulated Value: " << simulatedDriftTime - trueDriftTime);

    }
  //  c1->setVector(myvector);
  */

}


void CDCRecoHitMakerModule::endRun()
{
}

void CDCRecoHitMakerModule::terminate()
{
}
