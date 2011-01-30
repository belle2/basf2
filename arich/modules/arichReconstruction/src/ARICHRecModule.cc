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

#include <arich/modules/arichReconstruction/ARICHRecModule.h>
#include <arich/hitarich/ARICHAeroHit.h>
#include <arich/modules/arichReconstruction/ARICHTrack.h>
#include <arich/modules/arichReconstruction/ARICHReconstruction.h>

#include <framework/core/ModuleManager.h>

#include <time.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
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
REG_MODULE(ARICHRec)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ARICHRecModule::ARICHRecModule() : Module()
{
  // Set description()
  setDescription("ARICHRec");

  // Add parameters
  addParam("InputColName", m_inColName, "Input collection name", string("AeroHitARICHArray"));
  addParam("OutputColName", m_outColName, "Output col lection name",  string("ARICHTrackArray"));
}

ARICHRecModule::~ARICHRecModule()
{
  if (m_ana) delete m_ana;
}

void ARICHRecModule::initialize()
{
  // Initialize variables

  m_ana = new ARICHReconstruction();
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // Print set parameters
  printModuleParams();

  // CPU time start
  m_timeCPU = clock() * Unit::us;

}

void ARICHRecModule::beginRun()
{
  // Print run number
  B2INFO("ARICHReconstruction: Processing run: " << m_nRun);
}

void ARICHRecModule::event()
{
  //------------------------------------------------------
  // Get the collection of ARICHSimHits from the DataStore.
  //------------------------------------------------------
  StoreArray<ARICHAeroHit> arichInArray(m_inColName);
  if (!arichInArray) {
    B2ERROR("ARICHRecModule: Input collection " << m_inColName << " unavailable.");
  }

  //-----------------------------------------------------
  // Get the collection of ARICHHits from the Data store,
  // (or have one created)
  //-----------------------------------------------------
  StoreArray<ARICHTrack> arichOutArray(m_outColName);
  if (!arichOutArray) {
    B2ERROR("ARICHRecModule: Output collection " << m_inColName << " unavailable.");
  }

  //---------------------------------------------------------------------
  // Convert SimHits one by one to digitizer hits.
  //---------------------------------------------------------------------

  // Get number of hits in this event
  int nTracks = arichInArray->GetEntries();

  // Loop over all tracks
  for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
    ARICHAeroHit* aeroHit = arichInArray[iTrack];
    new(arichOutArray->AddrAt(iTrack)) ARICHTrack(*aeroHit);
  } // for iTrack

  m_ana->ReconstructParticles();
  m_ana->Likelihood2();
  m_nEvent++;
}

void ARICHRecModule::endRun()
{
  m_nRun++;
}

void ARICHRecModule::terminate()
{
  // CPU time end
  m_timeCPU = clock() * Unit::us - m_timeCPU;

  // Announce
  B2INFO("ARICHRecModule finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");

}

void ARICHRecModule::printModuleParams() const
{
  B2INFO("ARICHRecModule parameters:")
  B2INFO("Input collection name:  " << m_inColName)
  B2INFO("Output collection name: " << m_outColName)
}

