/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/karlsruhe/CDCDummyModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <framework/dataobjects/Relation.h>
#include <tracking/dataobjects/Track.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/hitcdc/CDCSimHit.h>

using namespace std;
using namespace Belle2;

REG_MODULE(CDCDummy);


CDCDummyModule::CDCDummyModule() : Module()
{
  setDescription("Add all CDCHits up as a single track");

  addParam("CDCRecoHitCollectionName", m_cdcRecoHitCollectionName,
           "Name of the CDCRecoHitCollection", string("CDCRecoHits"));
  addParam("TrackToCDCRecoHitCollectionName", m_trackToCDCRecoHitCollectionName,
           "Name of relation between Track and CDCRecoHits.", string("TrackToCDCRecoHits"));

}

CDCDummyModule::~CDCDummyModule()
{
}

void CDCDummyModule::initialize()
{
}

void CDCDummyModule::beginRun()
{
}

void CDCDummyModule::event()
{
  StoreArray<Relation> relations("SimHitToCDCHits");
  if (relations->GetEntriesFast()) { //test if the collection includes at least one element - not so if e.g. particle didn't hit CDC
    //The name and the durability of the corresponding StoreAccessor are the same for all the relations in question
    //So I set them only once to save time:
    StoreArray<CDCSimHit> simHits(relations[0]->getFromAccessorInfo());
    StoreArray<CDCHit> cdcHits(relations[0]->getToAccessorInfo());

    for (int ii = 0; ii < relations->GetEntriesFast(); ++ii) {
      //print out the pull for the CDC Resolution
      B2INFO("Pull: " << cdcHits[relations[ii]->getToIndex()]->getDriftTime() - simHits[relations[ii]->getFromIndex()]->getDriftLength());
    }
  }
}

void CDCDummyModule::endRun()
{

}

void CDCDummyModule::terminate()
{
}



