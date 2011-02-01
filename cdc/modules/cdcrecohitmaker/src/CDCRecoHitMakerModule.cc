/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcrecohitmaker/CDCRecoHitMakerModule.h>

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
REG_MODULE(CDCRecoHitMaker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCRecoHitMakerModule::CDCRecoHitMakerModule() : Module()
{
  //Set module properties
  setDescription("Creates CDCRecoHits from CDCHits.");

  //Parameter definition

  addParam("SimHitToHitCollectionExists", m_mc,
           "Is the Relation for CDCSimHits to CDCHits available?", false);

  addParam("CDCHitCollection", m_cdcHitCollectionName,
           "Name of Collection holdung CDCHits.", string("CDCHitCollection"));

  addParam("SimHitToCDCHitCollectionName", m_simHitToCDCHitCollectionName,
           "Name of Collection holdung Relation from CDCSimHits to CDCHits.", string("SimHitToCDCHitCollection"));

  addParam("CDCRecoHitCollection", m_cdcRecoHitCollectionName,
           "Name of Collection holdung CDCRecoHits//Output of this modul.", string("CDCRecoHitCollection"));

}


CDCRecoHitMakerModule::~CDCRecoHitMakerModule()
{

}

void CDCRecoHitMakerModule::event()
{

  //currently doesn't use relations in any way.

  StoreArray<CDCHit>     cdcHitArray(m_cdcHitCollectionName);
  StoreArray<CDCRecoHit> cdcRecoHitArray(m_cdcRecoHitCollectionName);

  for (int ii = 0; ii < cdcHitArray->GetEntriesFast(); ii++) {
    new(cdcRecoHitArray->AddrAt(ii)) CDCRecoHit(*(cdcHitArray[ii]));
  }
}

