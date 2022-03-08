/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/EcmsCollector/EcmsCollectorModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/variables/ContinuumSuppressionVariables.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <framework/particledb/EvtGenDatabasePDG.h>


using namespace Belle2;
using std::vector;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EcmsCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EcmsCollectorModule::EcmsCollectorModule() : CalibrationCollectorModule()
{
  //Set module properties

  setDescription("Collect data for eCMS calibration algorithm using the momenta of the hadronic events");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void EcmsCollectorModule::prepare()
{
  B2INFO("Init of the trees");
  TString objectName = "events";
  //Data object creation --------------------------------------------------
  TTree* tree = new TTree(objectName, "");

  tree->Branch<int>("event", &m_evt);
  tree->Branch<int>("exp", &m_exp);
  tree->Branch<int>("run", &m_run);
  tree->Branch<double>("time", &m_time);

  tree->Branch<vector<double>>("pBcms", &m_pBcms);
  tree->Branch<vector<double>>("mB", &m_mB);

  tree->Branch<vector<int>>("pdg", &m_pdg);
  tree->Branch<vector<int>>("mode", &m_mode);
  tree->Branch<vector<double>>("Kpid", &m_Kpid);
  tree->Branch<vector<double>>("R2", &m_R2);
  tree->Branch<vector<double>>("mD", &m_mD);
  tree->Branch<vector<double>>("dmDstar", &m_dmDstar);


  // We register the objects so that our framework knows about them.
  // Don't try and hold onto the pointers or fill these objects directly
  // Use the getObjectPtr functions to access collector objects
  registerObject<TTree>(objectName.Data(), tree);
}


void EcmsCollectorModule::resize(int n)
{
  m_pBcms.resize(n);
  m_mB.resize(n);
  m_pdg.resize(n);
  m_mode.resize(n);
  m_Kpid.resize(n);
  m_R2.resize(n);
  m_mD.resize(n);
  m_dmDstar.resize(n);
}



void EcmsCollectorModule::collect()
{
  // store event info
  m_evt  = m_emd->getEvent();
  m_run  = m_emd->getRun();
  m_exp  = m_emd->getExperiment();
  m_time = m_emd->getTime() / 1e9 / 3600.; //from ns to hours


  StoreObjPtr<ParticleList> B0("B0:merged");
  StoreObjPtr<ParticleList> Bm("B-:merged");


  //put all the B candidates into the vector
  vector<const Particle*> Bparts;

  if (B0.isValid()) {
    for (unsigned i = 0; i < B0->getListSize(); ++i)
      if (B0->getParticle(i))
        Bparts.push_back(B0->getParticle(i));
  }
  if (Bm.isValid()) {
    for (unsigned i = 0; i < Bm->getListSize(); ++i)
      if (Bm->getParticle(i))
        Bparts.push_back(Bm->getParticle(i));
  }

  if (Bparts.size() == 0) return;


  resize(Bparts.size());

  for (unsigned i = 0; i < Bparts.size(); ++i) {
    const Particle* Bpart = Bparts[i];

    //Convert mBC and deltaE to the Y4S reference
    m_pBcms[i]  = PCmsLabTransform::labToCms(Bpart->get4Vector()).P();
    m_mB[i]     = Bpart->getMass();
    m_pdg[i]    = Bpart->getPDGCode();
    m_mode[i]   = Bpart->getExtraInfo("decayModeID");
    m_R2[i]     = Variable::R2(Bpart);


    const Particle* D    = nullptr;
    m_dmDstar[i] = -99;

    //if D0 or D+ meson
    if (abs(Bpart->getDaughter(0)->getPDGCode()) == 421 || abs(Bpart->getDaughter(0)->getPDGCode()) == 411) {
      D = Bpart->getDaughter(0);
    } else if (abs(Bpart->getDaughter(0)->getPDGCode()) == 413 || abs(Bpart->getDaughter(0)->getPDGCode()) == 423) {
      const Particle* Dstar = Bpart->getDaughter(0);
      D = Dstar->getDaughter(0);
      m_dmDstar[i] = Dstar->getMass() - D->getMass();
    } else {
      B2INFO("No D meson found");
    }
    m_mD[i] = D->getMass();
    const Particle* Kaon =  D->getDaughter(0);



    m_Kpid[i] = -99;
    if (Kaon && Kaon->getPIDLikelihood()) {
      m_Kpid[i] = Kaon->getPIDLikelihood()->getProbability(Const::ChargedStable(321), Const::ChargedStable(211));
    }
  }


  getObjectPtr<TTree>("events")->Fill();

}
