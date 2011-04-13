/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Own include
#include <arich/modules/arichDigitizer/ARICHDigiModule.h>
#include <arich/geoarich/ARICHGeometryPar.h>

#include <framework/core/ModuleManager.h>
#include <time.h>

// Hit classes
#include <arich/hitarich/ARICHSimHit.h>
#include <arich/hitarich/ARICHHit.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector2.h>

using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(ARICHDigi)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ARICHDigiModule::ARICHDigiModule() : Module(),
    m_random(new TRandom3(0))
{
  // Set description()
  setDescription("ARICHDigitizer");

  // Add parameters
  addParam("InputColName", m_inColName, "Input collection name", string("ARICHSimHitArray"));
  addParam("OutputColName", m_outColName, "Output collection name", string("ARICHHitArray"));
}

ARICHDigiModule::~ARICHDigiModule()
{
  if (m_random) delete m_random;
}

void ARICHDigiModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // Print set parameters
  printModuleParams();

  // CPU time start
  m_timeCPU = clock() * Unit::us;

}

void ARICHDigiModule::beginRun()
{
  // Print run number
  B2INFO("ARICHDigi: Processing run: " << m_nRun);
}

void ARICHDigiModule::event()
{
  //------------------------------------------------------
  // Get the collection of ARICHSimHits from the Data store.
  //------------------------------------------------------

  StoreArray<ARICHSimHit> arichInArray(m_inColName);
  if (!arichInArray) {
    B2ERROR("ARICHDigi: Input collection " << m_inColName << " unavailable.");
  }

  //-----------------------------------------------------
  // Get the collection of ARICHHits from the Data store,
  // (or have one created)
  //-----------------------------------------------------
  StoreArray<ARICHHit> arichOutArray(m_outColName);
  if (!arichOutArray) {
    B2ERROR("ARICHDigi: Output collection " << m_inColName << " unavailable.");
  }

  ARICHGeometryPar * arichgp = ARICHGeometryPar::Instance();

  //---------------------------------------------------------------------
  // Convert SimHits one by one to digitizer hits.
  //---------------------------------------------------------------------

  // Get number of hits in this event
  int nHits = arichInArray->GetEntries();

  // Loop over all hits
  for (int iHit = 0; iHit < nHits; ++iHit) {
    // Get a simhit
    ARICHSimHit* aSimHit = arichInArray[iHit];

    double energy = aSimHit->getEnergy();

    // Apply q.e. of detector
    if (!DetectorQE(energy)) continue;

    TVector2 locpos(aSimHit->getLocalPosition().X(), aSimHit->getLocalPosition().Y());

    // Get id number of hit channel
    int channelID = arichgp->getChannelID(locpos);
    if (channelID < 0) continue;

    double globaltime = aSimHit->getGlobalTime();
    int moduleID = aSimHit->getModuleID();

    // Check if channel already registered hit in this event(no multiple hits)
    bool newhit = true;
    int nSig = arichOutArray->GetEntries();
    for (int iSig = 0; iSig < nSig; ++iSig) {
      ARICHHit* aHit = arichOutArray[iSig];
      if (aHit->getModuleID() == moduleID && aHit->getChannelID() == channelID) { newhit = false; break; }
    }
    if (!newhit) continue;

    // Add new ARIHCHit to datastore
    new(arichOutArray->AddrAt(nSig)) ARICHHit();
    ARICHHit* newHit = arichOutArray[nSig];
    newHit->setModuleID(moduleID);
    newHit->setChannelID(channelID);
    newHit->setGlobalTime(globaltime);

  } // for iHit

  m_nEvent++;
}

void ARICHDigiModule::endRun()
{
  m_nRun++;
}

void ARICHDigiModule::terminate()
{
  // CPU time end
  m_timeCPU = clock() * Unit::us - m_timeCPU;

  // Announce
  B2INFO("ARICHDigi finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");

}

void ARICHDigiModule::printModuleParams() const
{
  B2INFO("ARICHDigi parameters:")
  B2INFO("Input collection name:  " << m_inColName)
  B2INFO("Output collection name: " << m_outColName)
}

double ARICHDigiModule::QESuperBialkali(double energy)
{
  const float qe[44] = {0.197, 0.251 , 0.29 , 0.313 , 0.333 , 0.343 , 0.348 , 0.35 , 0.35, 0.348,
                        0.346, 0.343, 0.34, 0.335, 0.327 , 0.317 , 0.306 , 0.294 , 0.280 , 0.263 , 0.244 ,
                        0.220 , 0.197 , 0.174 , 0.153 , 0.133 , 0.116 , 0.990E-01, 0.830E-01, 0.700E-01,
                        0.580E-01, 0.480E-01, 0.410E-01, 0.330E-01, 0.260E-01, 0.190E-01, 0.140E-01, 0.100E-01, 0.600E-02,
                        0.400E-02, 0.200E-02, 0.100E-02, 0.00 , 0.00
                       };

  int ich = (int)((1239.85 / energy - 270) / 10.);
  if (ich < 0 || ich >= 44) return 0;
  return 0.8*qe[ich];// collection efficiency 0.8
}

int ARICHDigiModule::DetectorQE(double energy)
{
  return (m_random->Uniform(1) < QESuperBialkali(energy));
}
