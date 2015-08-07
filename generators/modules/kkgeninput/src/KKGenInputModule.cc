/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka, Torben Ferber                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/kkmc/KKGenInterface.h>
#include <generators/modules/kkgeninput/KKGenInputModule.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <framework/core/Environment.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/GearDir.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include <TRandom.h>
#include <stdio.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(KKGenInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

KKGenInputModule::KKGenInputModule() : Module(), m_initial(BeamParameters::c_smearVertex)
{
  //Set module properties
  setDescription("KKGenInput module. This an interface for KK2f Event Generator for basf2. The generated events are stored into MCParticles. You can find an expample of its decay file (tau_decaytable.dat) for tau-pair events at ${BELLE2_RELEASE_DIR}/data/generators/kkmc. On the other hand, when you like to generate mu-pair events, ${BELLE2_RELEASE_DIR}/data/generators/kkmc/mu.input.dat should be set to tauinputFile in your steering file.");
  setPropertyFlags(c_Input);

  //Get ENVIRONMENTs
  char* belle2_release_dir = std::getenv("BELLE2_RELEASE_DIR");
  char* belle2_local_dir = std::getenv("BELLE2_LOCAL_DIR");
  char* belle2_externals_dir = std::getenv("BELLE2_EXTERNALS_DIR");

  //Get default full filename of evt.pdl
  string default_evtpdlfilename = string("./evt.pdl");
  if (belle2_externals_dir != NULL)
    default_evtpdlfilename = string(belle2_externals_dir)
                             + string("/share/evtgen/evt.pdl");

  //Set default full filenames of KK2f setting files
  string default_KKdefaultFileName = string("");
  string default_tauinputFileName = string("");
  string default_taudecaytableFileName = string("");
  string default_KKMCOutputFileName = string("kkmc.txt");
  B2DEBUG(100, "Default setting files are set: empty...");

  if (belle2_release_dir != NULL) {
    string tentative_KKdefaultFileName = string(belle2_release_dir) +
                                         string("/data/generators/kkmc/KK2f_defaults.dat");
    boost::filesystem::path fpath_b2r_kk2f(tentative_KKdefaultFileName);
    if (!boost::filesystem::exists(fpath_b2r_kk2f)) {
      B2DEBUG(100, boost::format("Default directory for the setting files is re-set: %s/data/generators/kkmc") % belle2_release_dir);
      default_KKdefaultFileName = string(belle2_release_dir) +
                                  string("/data/generators/kkmc/KK2f_defaults.dat");
      default_tauinputFileName = string(belle2_release_dir) +
                                 string("/data/generators/kkmc/tau.input.dat");
      default_taudecaytableFileName = string(belle2_release_dir) +
                                      string("/data/generators/kkmc/tau_decaytable.dat");
    }
  }

  if (belle2_local_dir != NULL) {
    string tentative_KKdefaultFileName = string(belle2_local_dir) +
                                         string("/data/generators/kkmc/KK2f_defaults.dat");
    boost::filesystem::path fpath_b2l_kk2f(tentative_KKdefaultFileName);
    if (boost::filesystem::exists(fpath_b2l_kk2f)) {
      default_KKdefaultFileName = string(belle2_local_dir) +
                                  string("/data/generators/kkmc/KK2f_defaults.dat");
      default_tauinputFileName = string(belle2_local_dir) +
                                 string("/data/generators/kkmc/tau.input.dat");
      default_taudecaytableFileName = string(belle2_local_dir) +
                                      string("/data/generators/kkmc/tau_decaytable.dat");
      B2DEBUG(100, boost::format("Default directory for the setting files is re-set: %s/data/generators/kkmc") % belle2_local_dir);
    }
  }

  //Parameter definition
  addParam("KKdefaultFile", m_KKdefaultFileName, "default KKMC setting filename", default_KKdefaultFileName);
  addParam("tauinputFile", m_tauinputFileName, "user-defined tau/mu/q-pairs generation setting", default_tauinputFileName);
  addParam("taudecaytableFile", m_taudecaytableFileName, "tau-decay-table file name", default_taudecaytableFileName);
  addParam("evtpdlfilename", m_EvtPDLFileName, "EvtPDL filename", default_evtpdlfilename);
  addParam("kkmcoutputfilename", m_KKMCOutputFileName, "KKMC output filename", default_KKMCOutputFileName);

}


void KKGenInputModule::initialize()
{
  B2INFO("starting initialisation of KKGen Input Module. ");
  FILE* fp;

  fp = fopen(m_KKMCOutputFileName.c_str(), "r");
  if (fp) {
    fclose(fp);
    remove(m_KKMCOutputFileName.c_str());
  } else {
    fp = fopen(m_KKMCOutputFileName.c_str(), "w");
    if (fp) {
      fclose(fp);
      remove(m_KKMCOutputFileName.c_str());
    } else {
      B2FATAL("failed to make KKMC output file!");
      exit(1);
    }
  }

  m_Ikkgen.setup(m_KKdefaultFileName, m_tauinputFileName,
                 m_taudecaytableFileName, m_EvtPDLFileName, m_KKMCOutputFileName);

  //initial particle for beam parameters
  m_initial.initialize();
  MCInitialParticles& initial = m_initial.generate();
  TLorentzVector v_ler = initial.getLER();
  TLorentzVector v_her = initial.getHER();

  //Initialize MCParticle collection
  StoreArray<MCParticle>::registerPersistent();

  //set the beam parameters, ignoring beam energy spread for the moment
  B2INFO("Energy spread is ignored!");
  m_Ikkgen.set_beam_info(v_ler, 0.0, v_her, 0.0);

  B2INFO("Finished initialising the KKGen Input Module. ");

}

void KKGenInputModule::beginRun()
{

}

void KKGenInputModule::event()
{

  B2INFO("Starting event generation.");
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);

  //generate an MCInitialEvent (for vertex smearing)
  MCInitialParticles& initial = m_initial.generate();
  TVector3 vertex = initial.getVertex();

  mpg.clear();
  int nPart =  m_Ikkgen.simulateEvent(mpg, vertex);

  // to check surely generated events are received or not
  for (int i = 0; i < nPart; ++i) {
    MCParticleGraph::GraphParticle* p = &mpg[i];
    int moID = 0;
    char buf[200];
    sprintf(buf, "IntC: %3d %4d %8d %4d %4d %4d %9.4f %9.4f %9.4f %9.4f",
            p->getIndex(), p->getStatus(), p->getPDG(), moID,
            p->getFirstDaughter(), p->getLastDaughter(),
            p->get4Vector().Px(), p->get4Vector().Py(),
            p->get4Vector().Pz(), p->get4Vector().E());
    B2DEBUG(100, buf);
  }

  B2INFO("Generated event " << eventMetaDataPtr->getEvent() << " with " << nPart << " particles.");
}

void KKGenInputModule::terminate()
{
  m_Ikkgen.term();
}
