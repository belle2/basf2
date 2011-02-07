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
#include <time.h>

#include <arich/hitarich/ARICHAeroHit.h>
#include <arich/modules/arichReconstruction/ARICHTrack.h>

#include <framework/core/ModuleManager.h>


// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector3.h>
#include <TH1F.h>
#include <TFile.h>

using namespace std;
using namespace boost;
using namespace Belle2;

TH1F* refll;
TH1F* norefl;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ARICHRec)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ARICHRecModule::ARICHRecModule() : Module(), m_ana(new ARICHReconstruction())
{
  // Set description()
  setDescription("ARICHRec");

  // Add parameters
  addParam("InputColName", m_inColName, "Input collection name", string("AeroHitARICHArray"));
  addParam("OutputColName", m_outColName, "Output col lection name",  string("ARICHTrackArray"));
}

ARICHRecModule::~ARICHRecModule()
{
  delete m_ana;
}

void ARICHRecModule::initialize()
{
  // Initialize variables

  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // Print set parameters
  printModuleParams();

  // CPU time start
  m_timeCPU = clock() * Unit::us;

  refll = new TH1F("refll", "refll", 200, 0, 1.5);
  norefl = new TH1F("norefl", "norefl", 200, 0, 1.5);

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

  TFile* f = new TFile("testrefr.root", "RECREATE");
  refll->Write();
  norefl->Write();
  f->Close();

}

void ARICHRecModule::printModuleParams() const
{
  B2INFO("ARICHRecModule parameters:")
  B2INFO("Input collection name:  " << m_inColName)
  B2INFO("Output collection name: " << m_outColName)
}

