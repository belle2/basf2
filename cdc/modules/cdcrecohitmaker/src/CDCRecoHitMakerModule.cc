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

#include <generators/dataobjects/MCParticle.h>
//#include <cdc/hitcdc/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <framework/datastore/StoreArray.h>
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
  setDescription("Creates CDCRecoHits from CDCHits. Creates also Relations between MCParticles and CDCRecoHits.");

  //Parameter definition

  addParam("SimHitToHitCollectionExists", m_mc,
           "Is the Relation for CDCSimHits to CDCHits available?", false);

  addParam("CDCHitCollection", m_cdcHitCollectionName,
           "Name of Collection holding CDCHits.", string("CDCHits"));

  //needed to create relation between MCParticles and CDCRecoHits
  addParam("SimHitToCDCHitCollectionName", m_simHitToCDCHitCollectionName,
           "Name of Collection holding Relation from CDCSimHits to CDCHits.", string("SimHitToCDCHits"));

  addParam("MCParticlesColName", m_mcParticlesCollectionName, "Name of collection holding the MCParticles", string("MCParticles"));

  addParam("MCParticleToCDCSimHitsColName", m_mcPartToCDCSimHitsCollectionName, "Name of collection holding the relations the MCParticles and the CDCSimHits", string("MCPartToCDCSimHits"));

  //output
  addParam("CDCRecoHitCollection", m_cdcRecoHitCollectionName,
           "Name of Collection holding CDCRecoHits//Output of this module.", string("CDCRecoHits"));

  addParam("MCParticlesToCDCRecoHits", m_mcParticleToCDCRecoHits, "Name of collection holding the Relations  MCParticles->CDCRecoHits//Output of this module.",
           string("MCParticleToCDCRecoHits"));

}


CDCRecoHitMakerModule::~CDCRecoHitMakerModule()
{

}

void CDCRecoHitMakerModule::event()
{

  //Input Array
  StoreArray<CDCHit>     cdcHitArray(m_cdcHitCollectionName);

  //Arrays needed for relation creation
  StoreArray<MCParticle> mcParticles(m_mcParticlesCollectionName);
  StoreArray<Relation> mcPartToCDCSimHit(m_mcPartToCDCSimHitsCollectionName);
  StoreArray<Relation> cdcSimHitToCDCHit(m_simHitToCDCHitCollectionName);

  //Output Arrays
  StoreArray<CDCRecoHit> cdcRecoHitArray(m_cdcRecoHitCollectionName);
  StoreArray<Relation> mcPartToCDCRecoHit(m_mcParticleToCDCRecoHits);

  for (int ii = 0; ii < cdcHitArray.getEntries(); ii++) {
    new(cdcRecoHitArray->AddrAt(ii)) CDCRecoHit(*(cdcHitArray[ii]));

    //Create a relation between the RecoHits and the MCParticles

    int simHitindex;
    int mcindex;

    //find the corresponding SimHit for the RecoHit
    for (int j = 0; j < cdcSimHitToCDCHit.getEntries(); j++) {
      if (cdcSimHitToCDCHit[j]->getToIndex() == ii) {
        simHitindex = cdcSimHitToCDCHit[j]->getFromIndex();
      }
    }
    //find the corresponding MCParticle for this SimHit
    for (int k = 0; k < mcPartToCDCSimHit.getEntries(); k++) {
      if (mcPartToCDCSimHit[k]->getToIndex() == simHitindex) {
        mcindex = mcPartToCDCSimHit[k]->getFromIndex();
      }
    }

    //create Relation
    new(mcPartToCDCRecoHit->AddrAt(ii)) Relation(mcParticles, cdcRecoHitArray, mcindex, ii);

  } //end loop over cdcHitArray

  B2DEBUG(149, "CDCRecoHitMaker: Number of CDCRecoHits: " << cdcRecoHitArray.getEntries());
  B2DEBUG(149, "CDCRecoHitMaker: Number of Relations MCParticle->CDCRecoHit: " << mcPartToCDCRecoHit.getEntries());


}

