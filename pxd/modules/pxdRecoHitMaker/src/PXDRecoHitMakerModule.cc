/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Own include
#include <pxd/modules/pxdRecoHitMaker/PXDRecoHitMakerModule.h>
#include <pxd/geopxd/SiGeoCache.h>
#include <pxd/dataobjects/PXDHit.h>
#include <pxd/dataobjects/PXDRecoHit.h>

#include <time.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRecoHitMaker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDRecoHitMakerModule::PXDRecoHitMakerModule() : Module()
{
  // Set description()
  setDescription("PXDRecoHit maker");

  // Add parameters
  addParam("InputColName", m_inColName, "Input collection name", string(DEFAULT_PXDHITS));
  addParam("OutputColName", m_outColName, "Output collection name", string(DEFAULT_PXDRECOHITS));
  addParam("RelationColName", m_relColName,
           "Relation collection (MC particles to PXDRecoHits", string(DEFAULT_PXDRECOHITSREL));
}

PXDRecoHitMakerModule::~PXDRecoHitMakerModule()
{
  ;
}

void PXDRecoHitMakerModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // Print set parameters
  // printModuleParams();

  // CPU time start
  m_timeCPU = clock() * Unit::us;
}

void PXDRecoHitMakerModule::beginRun()
{
  // Print run number
  B2INFO("PXDRecoHitMaker: Processing run: " << m_nRun);

  // Re-read geometry for a new run
  SiGeoCache::instance()->refresh();
}

void PXDRecoHitMakerModule::event()
{
  //------------------------------------------------------
  // Get the collection of PXDHits from the DataStore.
  //------------------------------------------------------
  StoreArray<PXDHit> pxdInArray(m_inColName);
  if (!pxdInArray) {
    B2INFO("PXDRecoHitMaker: Input collection " << m_inColName << " unavailable.")
    return;
  }

  //-----------------------------------------------------
  // Get the collection of PXDRecoHits from the Data store,
  // (or have one created)
  //-----------------------------------------------------
  StoreArray<PXDRecoHit> pxdOutArray(m_outColName);
  if (!pxdOutArray) {
    B2ERROR("PXDRecoHitMaker: Output collection " << m_inColName << " cannot be created.")
    return;
  }

  //---------------------------------------------------------------------
  // Convert PXDHits one by one to RecoHits.
  //---------------------------------------------------------------------

  // Get number of hits in this event
  int nHits = pxdInArray->GetEntries();

  // Loop over all hits
  for (int iHit = 0; iHit < nHits; ++iHit) {
    // Make space and turn next PXDHit into PXDRecoHit.
    new(pxdOutArray->AddrAt(iHit)) PXDRecoHit(pxdInArray[iHit]);
  }

  // I don't hand over relations at this stage.

  m_nEvent++;
}

void PXDRecoHitMakerModule::endRun()
{
  m_nRun++;
}

void PXDRecoHitMakerModule::terminate()
{
  // CPU time end
  m_timeCPU = clock() * Unit::us - m_timeCPU;

  // Announce
  B2INFO("PXDRecoHitMaker finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");
}

void PXDRecoHitMakerModule::printModuleParams() const
{
  B2INFO("PXDRecoHitMaker parameters:")
  B2INFO("  Input collection name:  " << m_inColName)
  B2INFO("  Output collection name: " << m_outColName)
  B2INFO("  Relation collection name: " << m_relColName)
  ;
}
