/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/eCmsCollector/eCmsCollectorModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/ReferenceFrame.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <analysis/variables/Variables.h>
#include <analysis/variables/ContinuumSuppressionVariables.h>

#include <mdst/dataobjects/PIDLikelihood.h>

#include <analysis/utility/PCmsLabTransform.h>

using namespace Belle2;
using namespace std;

#include <iostream>




//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eCmsCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

eCmsCollectorModule::eCmsCollectorModule() : CalibrationCollectorModule(),
  m_exp(-99), m_run(-99), m_evt(-99),
  m_time(-99)
{
  //Set module properties

  setDescription("Collect data for eCMS calibration algorithm using the momenta of the hadronic events");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("Y4SPListName", m_Y4SPListName, "Name of the Y4S particle list", std::string("Upsilon(4S):IPDQM"));
}

void eCmsCollectorModule::prepare()
{
  B2INFO("Init of the trees");
  TString objectName = "events";
  //Data object creation --------------------------------------------------
  TTree* tree = new TTree(objectName, "");

  tree->Branch<int>("event", &m_evt);
  tree->Branch<int>("exp", &m_exp);
  tree->Branch<int>("run", &m_run);
  tree->Branch<double>("time", &m_time);

  tree->Branch<double>("mBC", &m_mBC);
  tree->Branch<double>("deltaE", &m_deltaE);
  tree->Branch<int>("pdg", &m_pdg);
  tree->Branch<int>("mode", &m_mode);
  tree->Branch<double>("Kpid", &m_Kpid);
  tree->Branch<double>("R2", &m_R2);
  tree->Branch<double>("mD", &m_mD);
  tree->Branch<double>("dmDstar", &m_dmDstar);


  // We register the objects so that our framework knows about them.
  // Don't try and hold onto the pointers or fill these objects directly
  // Use the getObjectPtr functions to access collector objects
  registerObject<TTree>(objectName.Data(), tree);
}


void eCmsCollectorModule::collect()
{
  // store event info
  m_evt  = m_emd->getEvent();
  m_run  = m_emd->getRun();
  m_exp  = m_emd->getExperiment();
  m_time = m_emd->getTime() / 1e9 / 3600.; //from ns to hours


  StoreObjPtr<ParticleList> B0("B0:merged");
  StoreObjPtr<ParticleList> Bm("B-:merged");

  const Particle* Bpart = nullptr;

  if (B0.isValid() && B0->getParticle(0)) {
    Bpart = B0->getParticle(0);
  } else if (Bm.isValid() && Bm->getParticle(0)) {
    Bpart = Bm->getParticle(0);
  }

  if (!Bpart) return;

  m_mBC    = Variable::particleMbc(Bpart);
  m_deltaE = Variable::particleDeltaE(Bpart);
  m_pdg    = Bpart->getPDGCode();
  m_mode   = Bpart->getExtraInfo("decayModeID");
  m_R2     = Variable::R2(Bpart);


  const Particle* D    = nullptr;
  m_dmDstar = -99;

  //if D0 or D+ meson
  if (abs(Bpart->getDaughter(0)->getPDGCode()) == 421 || abs(Bpart->getDaughter(0)->getPDGCode()) == 411) {
    D = Bpart->getDaughter(0);
  } else if (abs(Bpart->getDaughter(0)->getPDGCode()) == 413 || abs(Bpart->getDaughter(0)->getPDGCode()) == 423) {
    const Particle* Dstar = Bpart->getDaughter(0);
    D = Dstar->getDaughter(0);
    m_dmDstar = Dstar->getMass() - D->getMass();
  } else {
    cout << "Nothing founded" << endl;
  }
  m_mD = D->getMass();
  const Particle* Kaon =  D->getDaughter(0);


  cout << m_pdg << " " << m_mode << " " << m_mBC << " " << m_deltaE <<  " " << m_dmDstar << " : " << m_R2 << endl;
  cout << "K pdg code :  " << Kaon->getPDGCode() << endl;


  m_Kpid = -99;
  if (Kaon && Kaon->getPIDLikelihood()) {
    m_Kpid = Kaon->getPIDLikelihood()->getProbability(Const::ChargedStable(321), Const::ChargedStable(211));
  }
  cout << "Kpid = " << m_Kpid << endl;


  /*
  if (!Y4SParticles.isValid() || abs(Y4SParticles->getPDGCode()) != 300553)
    return;

  //m_mu0_pid = part0->getPIDLikelihood()->getProbability(Const::ChargedStable(13), Const::ChargedStable(11));




  std::vector<int> indxes =  Y4SParticles->getParticle(0)->getDaughterIndices();
  if (indxes.size() != 2) return;

  const Particle* part0 = Y4SParticles->getParticle(0)->getDaughter(0);
  const Particle* part1 = Y4SParticles->getParticle(0)->getDaughter(1);

  // Get the mu/e PID
  m_mu0_pid = part0->getPIDLikelihood()->getProbability(Const::ChargedStable(13), Const::ChargedStable(11));
  m_mu1_pid = part1->getPIDLikelihood()->getProbability(Const::ChargedStable(13), Const::ChargedStable(11));

  // get 3-vectors of the mu/e momenta
  m_mu0_p = part0->getMomentum();
  m_mu1_p = part1->getMomentum();
  */


  getObjectPtr<TTree>("events")->Fill();

}
