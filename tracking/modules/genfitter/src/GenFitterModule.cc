/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/genfitter/GenFitterModule.h>

//#include <cdc/hitcdc/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <framework/datastore/StoreArray.h>
//#include <framework/dataobjects/Relation.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GenFitter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GenFitterModule::GenFitterModule() : Module()
{
  //Set module properties
  setDescription("Creates CDCRecoHits from CDCHits.");

  //Parameter definition
  addParam("TrackToCDCRecoHitCollectionName", m_trackToCDCRecoHitCollectionName,
           "Name of collection holding the relations between track and CDCRecoHit", string("TrackToCDCRecoHitCollection"));

}


GenFitterModule::~GenFitterModule()
{

}


void GenFitterModule::initialize()
{
}


void GenFitterModule::beginRun()
{
}


void GenFitterModule::event()
{


  /*
  StoreArray<Relation> arraySimHitToCDCHit("SimHitToCDCHitCollection");
  StoreObjPtr<SimpleVec<float> > c1("ResolutionCanvas", DataStore::c_Persistent);

  std::vector<float> myvector(arraySimHitToCDCHit.GetEntries());

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


void GenFitterModule::endRun()
{
}

void GenFitterModule::terminate()
{
}
