/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka, Zbynek Drasal             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Own include
#include <pxd/modules/pxdDigitizer/PXDDigiModule.h>

#include <time.h>

// Hit classes
#include <pxd/hitpxd/PXDSimHit.h>
#include <pxd/hitpxd/PXDHit.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector3.h>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDigi)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDigiModule::PXDDigiModule() : Module(),
    m_cheater(new PXDcheater()),
    m_cid(new CIDManager(0)),
    m_random(new TRandom3(0))
{
  // Set description()
  setDescription("PXDDigitizer");

  // Add parameters
  addParam("InputColName", m_inColName, "Input collection name", string("PXDSimHitArray"));
  addParam("OutputColName", m_outColName, "Output collection name", string("PXDHitArray"));
  //addParam("RelationColNameMC2Digi", m_relColNameMC2Digi,
  //      "Name of relation collection - MC hits to Digitizer hits. (created if non-null)", string("PXDMC2DigiHitRel"));
}

PXDDigiModule::~PXDDigiModule()
{
  if (m_random) delete m_random;
  if (m_cid) delete m_cid;
  if (m_cheater) delete m_cheater;
}

void PXDDigiModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // Print set parameters
  printModuleParams();

  // CPU time start
  m_timeCPU = clock() * Unit::us;
}

void PXDDigiModule::beginRun()
{
  // Print run number
  B2INFO("PXDDigi: Processing run: " << m_nRun);
}

void PXDDigiModule::event()
{
  //------------------------------------------------------
  // Get the collection of PXDSimHits from the Data store.
  //------------------------------------------------------
  StoreArray<PXDSimHit> pxdInArray(m_inColName);
  if (!pxdInArray) {
    B2ERROR("PXDDigi: Input collection " << m_inColName << " unavailable.");
  }

  //-----------------------------------------------------
  // Get the collection of PXDHits from the Data store,
  // (or have one created)
  //-----------------------------------------------------
  StoreArray<PXDHit> pxdOutArray(m_outColName);
  if (!pxdOutArray) {
    B2ERROR("PXDDigi: Output collection " << m_inColName << " unavailable.");
  }

  //---------------------------------------------------------------------
  // Convert SimHits one by one to digitizer hits.
  //---------------------------------------------------------------------

  // Get number of hits in this event
  int nHits = pxdInArray->GetEntries();

  // Loop over all hits
  for (int iHit = 0; iHit < nHits; ++iHit) {
    // Get a simhit
    PXDSimHit* aSimHit = pxdInArray[iHit];

    // The processing is simplistic here, one in / one out.

    // Create a digi hit
    new(pxdOutArray->AddrAt(iHit)) PXDHit();
    PXDHit* newHit = pxdOutArray[iHit];

    // Geometry
    m_cid->setCID(0);
    short int layerID = aSimHit->getLayerID();
    short int ladderID = aSimHit->getLadderID();
    short int sensorID = aSimHit->getSensorID();

    m_cid->setLayerID(layerID);
    m_cid->setLadderID(ladderID);
    m_cid->setSensorID(sensorID);
    newHit->setSensorCID(m_cid->getCID());

    // Position + smear + errors
    TVector3 posIn = aSimHit->getPosIn();
    TVector3 posOut = aSimHit->getPosOut();
    float u = 0.5 * (posIn[1] + posOut[1]);   // mid-positions
    float v = 0.5 * (posIn[2] + posOut[2]);

    // Get resolutions from the cheater
    float du = m_cheater->getResolutionRPhi(layerID, ladderID, sensorID, aSimHit->getTheta());
    float dv = m_cheater->getResolutionZ(layerID, ladderID, sensorID, aSimHit->getTheta());
    float duv = 0; // covariance zero in detector coordinates

    // Smear
    u = u + m_random->Gaus(0, du);
    v = v + m_random->Gaus(0, dv);

    newHit->setU(u); newHit->setUError(du);
    newHit->setV(v); newHit->setVError(dv);
    newHit->setUVCov(duv);

    // Physics
    double eDep = aSimHit->getEnergyDep();
    double deDep = 0;
    newHit->setEnergyDep(eDep); newHit->setEnergyDepError(deDep);
  } // for iHit

  m_nEvent++;
}

void PXDDigiModule::endRun()
{
  m_nRun++;
}

void PXDDigiModule::terminate()
{
  // CPU time end
  m_timeCPU = clock() * Unit::us - m_timeCPU;

  // Announce
  B2INFO("PXDDigi finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");
}

void PXDDigiModule::printModuleParams() const
{
  B2INFO("PXDDigi parameters:")
  B2INFO("  Input collection name:  " << m_inColName)
  B2INFO("  Output collection name: " << m_outColName)
}


