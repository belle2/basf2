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
  setDescription("KKGen input");
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
  addParam("HER_Energy", m_EHER, "Energy for HER[GeV]", 7 * Unit::GeV);
  addParam("LER_Energy", m_ELER, "Energy for LER[GeV]", 4 * Unit::GeV);
  addParam("HER_Spread", m_HER_Espread, "Energy spread for HER[GeV]", 0.00513 * Unit::GeV);
  addParam("LER_Spread", m_LER_Espread, "Energy spread for LER[GeV]", 0.002375 * Unit::GeV);
  addParam("CrossingAngle", m_crossing_angle, "Beam pipe crossing angle[mrad]", 83 * Unit::mrad);
  addParam("RotationAngle", m_angle, "Rotation with respect to e- beampie[mrad]", 41.5 * Unit::mrad);
}


void KKGenInputModule::initialize()
{
  B2INFO("starting initialisation of KKGen Input Module. ");

  // beam 4-momenta setting

  double mEle = Const::electronMass;

  double angleLerToB = M_PI - m_angle * Unit::mrad;
  double Eler = m_ELER * Unit::GeV;
  double pLerZ = Eler * cos(angleLerToB);
  double pLerX = Eler * sin(angleLerToB);
  double pLerY = 0.;


  double angleHerToB = (m_crossing_angle * Unit::mrad) - (m_angle * Unit::mrad);
  double Eher = m_EHER * Unit::GeV;
  double pHerZ = Eher * cos(angleHerToB);
  double pHerX = Eher * sin(angleHerToB);
  double pHerY = 0.;

  TLorentzVector vlLer;
  vlLer.SetXYZM(pLerX, pLerY, pLerZ, mEle);

  TLorentzVector vlHer;
  vlHer.SetXYZM(pHerX, pHerY, pHerZ, mEle);

  m_Ikkgen.setup(m_KKdefaultFileName, m_tauinputFileName,
                 m_taudecaytableFileName, m_EvtPDLFileName, vlLer, vlHer);

  //Initialize MCParticle collection
  StoreArray<MCParticle>::registerPersistent();

  B2INFO("finished initialising the KKGen Input Module. ");

}


void KKGenInputModule::event()
{
  B2INFO("Starting event simulation.");
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);

  /*

  //Initialize the beam energy for each event separatly

  double mEle = Const::electronMass;

  double angleLerToB = M_PI - m_angle * Unit::mrad;
  double Eler = gRandom->Gaus(m_ELER, m_LER_Espread) * Unit::GeV; //Beam energy spread
  double pLerZ = Eler * cos(angleLerToB);
  double pLerX = Eler * sin(angleLerToB);
  double pLerY = 0.;


  double angleHerToB = (m_crossing_angle * Unit::mrad) - (m_angle * Unit::mrad);
  double Eher = gRandom->Gaus(m_EHER, m_HER_Espread) * Unit::GeV; //Beam energy spread
  double pHerZ = Eher * cos(angleHerToB);
  double pHerX = Eher * sin(angleHerToB);
  double pHerY = 0.;

  TLorentzVector vlLer;
  vlLer.SetXYZM(pLerX, pLerY, pLerZ, mEle);

  TLorentzVector vlHer;
  vlHer.SetXYZM(pHerX, pHerY, pHerZ, mEle);

  TLorentzVector pParentParticle;

  pParentParticle = (vlHer + vlLer);

  //end initialization

  */

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

