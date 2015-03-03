//+
// File : PantherInutModule.cc
// Description : A module to read panther records in basf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 16 - Feb - 2015
//-

#include <b2bii/modules/PantherInput/PantherInputModule.h>

#include <framework/core/Environment.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <cmath>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

// Panther tables
#include "panther/belletdf.h"
#include "panther/mdst.h"

// Belle2 objects
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/V0.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>

// ROOT classes
#include "TLorentzVector.h"
#include "TVector3.h"


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PantherInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PantherInputModule::PantherInputModule() : Module()
{
  //Set module properties
  setDescription("Panther input module");
  setPropertyFlags(c_Input);

  m_nevt = -1;

  //Parameter definition
  addParam("inputFileName"  , m_inputFileName, "Panther file name.", string("PantherInput.sroot"));

  B2DEBUG(1, "PantherInput: Constructor done.");
}


PantherInputModule::~PantherInputModule()
{
}

void PantherInputModule::initialize()
{
  // Initialize Panther
  BsInit(0);

  // Initialize EvtMetaData
  StoreObjPtr<EventMetaData>::registerPersistent();
  StoreArray<Particle>::registerPersistent();  // just to use Particle Class

  // Open data file
  m_fd = new Belle::Panther_FileIO(m_inputFileName.c_str(), BBS_READ);

  // Read first event
  m_fd->read();

  // Process first event
  Convert();

  B2INFO("PantherInput: initialized.");
}


void PantherInputModule::beginRun()
{
  B2INFO("PantherInput: beginRun called.");
}


void PantherInputModule::event()
{
  m_nevt++;
  // First event is already loaded
  if (m_nevt == 0) return;

  // Convert events
  Convert();
}

void PantherInputModule::Convert()
{
  // Read event from Panther
  int rectype = -1;
  while (rectype < 0 && rectype != -2) {
    rectype = m_fd->read();
    //    printf ( "rectype = %d\n", rectype );
  }
  if (rectype == -2) return;   // EoF detected

  // 1. Fill EventMetadata
  // Get Belle_event_Manager
  Belle::Belle_event_Manager& evman = Belle::Belle_event_Manager::get_manager();
  Belle::Belle_event& evt = evman[0];

  // Fill EventMetaData
  StoreObjPtr<EventMetaData> evtmetadata;
  evtmetadata.create();
  evtmetadata->setExperiment(evt.ExpNo());
  evtmetadata->setRun(evt.RunNo());
  evtmetadata->setEvent(evt.EvtNo() & 0x0fffffff);


  // 2. Particle Class
  StoreArray<Particle> plist;

  // 2.1 Mdst_charged
  Belle::Mdst_charged_Manager& chgman = Belle::Mdst_charged_Manager::get_manager();
  for (vector<Belle::Mdst_charged>::iterator it = chgman.begin(); it != chgman.end(); ++it) {
    Belle::Mdst_charged& chg = *it;
    Particle* b2chg = plist.appendNew();
    b2chg->set4Vector(TLorentzVector(chg.px(), chg.py(), chg.pz(), chg.mass()));
    //    printf ( "charged: px = %f, py = %f, pz = %f\n", chg.px(), chg.py(), chg.pz() );
  }
  // 2.2 Mdst_gamma
  Belle::Mdst_gamma_Manager& gamman = Belle::Mdst_gamma_Manager::get_manager();
  for (vector<Belle::Mdst_gamma>::iterator it = gamman.begin(); it != gamman.end(); ++it) {
    Belle::Mdst_gamma& gam = *it;
    Particle* b2gam = plist.appendNew();
    b2gam->set4Vector(TLorentzVector(gam.px(), gam.py(), gam.pz(), 0.0));
    //    printf ( "gamma: px = %f, py = %f, pz = %f\n", gam.px(), gam.py(), gam.pz() );
  }

}

void PantherInputModule::endRun()
{
  B2INFO("PantherInput: endRun done.");
}


void PantherInputModule::terminate()
{
  delete m_fd;
  B2INFO("PantherInput: terminate called")
}

