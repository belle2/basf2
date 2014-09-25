/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/kkmc/KKGenInterface.h>
#include <generators/modules/kkgeninput/KKGenInputModule.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <generators/utilities/cm2LabBoost.h>
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

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(KKGenInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

KKGenInputModule::KKGenInputModule() : Module()
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
  addParam("tauinputFile", m_tauinputFileName, "user-defined tau/mu-pairs generation setting", default_tauinputFileName);
  addParam("taudecaytableFile", m_taudecaytableFileName, "tau-decay-table file name", default_taudecaytableFileName);
  addParam("evtpdlfilename", m_EvtPDLFileName, "EvtPDL filename", default_evtpdlfilename);
}


void KKGenInputModule::initialize()
{
  B2INFO("starting initialisation of KKGen Input Module. ");

  m_Ikkgen.setup(m_KKdefaultFileName, m_tauinputFileName,
                 m_taudecaytableFileName, m_EvtPDLFileName);

  //Initialize MCParticle collection
  StoreArray<MCParticle>::registerPersistent();

  B2INFO("finished initialising the KKGen Input Module. ");

}

void KKGenInputModule::beginRun()
{
  //Initialize the beam energy for each run separatly
  GearDir ler("/Detector/SuperKEKB/LER/");
  GearDir her("/Detector/SuperKEKB/HER/");
  m_ELER = ler.getDouble("energy");
  m_EHER = her.getDouble("energy");
  m_LER_Espread = ler.getDouble("energyError");
  m_HER_Espread = her.getDouble("energyError");
  m_crossing_angle = her.getDouble("angle") - ler.getDouble("angle");
  m_angle = her.getDouble("angle");
  B2INFO("KKGenInputModule::beginRun() called.");
  char buf[100];
  sprintf(buf, "ELER = %f, LER beam energy spread = %f", m_ELER, m_LER_Espread);
  B2DEBUG(100, buf);
  sprintf(buf, "EHER = %f, HER beam energy spread = %f", m_EHER, m_HER_Espread);
  B2DEBUG(100, buf);
  sprintf(buf, "Crossing angls = %f, Detector tilted angle = %f", m_crossing_angle, m_angle);
  B2DEBUG(100, buf);

  double mEle = Const::electronMass;

  double angleLerToB = M_PI - m_angle;
  double Eler = m_ELER;
  double pLerZ = Eler * cos(angleLerToB);
  double pLerX = Eler * sin(angleLerToB);
  double pLerY = 0.;

  double angleHerToB = m_crossing_angle - m_angle;
  double Eher = m_EHER;
  double pHerZ = Eher * cos(angleHerToB);
  double pHerX = Eher * sin(angleHerToB);
  double pHerY = 0.;

  TLorentzVector vlLer, vlHer;
  vlLer.SetXYZM(pLerX, pLerY, pLerZ, mEle);
  vlHer.SetXYZM(pHerX, pHerY, pHerZ, mEle);

  sprintf(buf, "roots = %f", (vlLer + vlHer).Mag());
  B2DEBUG(100, buf);
  m_Ikkgen.set_beam_info(vlLer, m_LER_Espread, vlHer, m_HER_Espread);
}


void KKGenInputModule::event()
{

  B2INFO("Starting event simulation.");
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);

  mpg.clear();
  int nPart =  m_Ikkgen.simulateEvent(mpg);

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
    /*
        B2DEBUG(100, boost::format("IntC: %3d %4d %8d %4d %4d %4d %9.4f %9.4f %9.4f %9.4f" %
                                   p->getIndex() % p->getStatus() % p->getPDG() % moID %
                                   p->getFirstDaughter() % p->getLastDaughter() %
                                   p->get4Vector().Px() % p->get4Vector().Py() %
                                   p->get4Vector().Pz() % p->get4Vector().E()));
    */
  }
  B2INFO("Simulated event " << eventMetaDataPtr->getEvent() << " with " << nPart << " particles.");
}

