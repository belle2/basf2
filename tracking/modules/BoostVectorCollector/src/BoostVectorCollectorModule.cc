/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/BoostVectorCollector/BoostVectorCollectorModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/ReferenceFrame.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <mdst/dataobjects/PIDLikelihood.h>

using namespace Belle2;
using namespace std;

#include <iostream>

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BoostVectorCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BoostVectorCollectorModule::BoostVectorCollectorModule() : CalibrationCollectorModule(),
  m_exp(-99), m_run(-99), m_evt(-99),
  m_time(-99),
  m_mu0_pid(-99), m_mu1_pid(-99)
{
  //Set module properties

  setDescription("Collect data for BoostVector calibration algorithm using the momenta of the mu+mu- events");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("Y4SPListName", m_Y4SPListName, "Name of the Y4S particle list", std::string("Upsilon(4S):IPDQM"));
}

void BoostVectorCollectorModule::prepare()
{
  B2INFO("Init of the trees");
  TString objectName = "events";
  //Data object creation --------------------------------------------------
  TTree* tree = new TTree(objectName, "");

  tree->Branch<int>("event", &m_evt);
  tree->Branch<int>("exp", &m_exp);
  tree->Branch<int>("run", &m_run);
  tree->Branch<double>("time", &m_time);

  tree->Branch<double>("mu0_pid", &m_mu0_pid);
  tree->Branch<double>("mu1_pid", &m_mu1_pid);

  tree->Branch<TVector3>("mu0_p", &m_mu0_p);
  tree->Branch<TVector3>("mu1_p", &m_mu1_p);


  // We register the objects so that our framework knows about them.
  // Don't try and hold onto the pointers or fill these objects directly
  // Use the getObjectPtr functions to access collector objects
  registerObject<TTree>(objectName.Data(), tree);
}


void BoostVectorCollectorModule::collect()
{
  m_evt  = m_emd->getEvent();
  m_run  = m_emd->getRun();
  m_exp  = m_emd->getExperiment();
  m_time = m_emd->getTime() / 1e9 / 3600.; //from ns to hours


  StoreObjPtr<ParticleList> Y4SParticles(m_Y4SPListName);


  if (!Y4SParticles.isValid() || abs(Y4SParticles->getPDGCode()) != 300553)
    return;

  if (Y4SParticles->getListSize() !=  1)
    return;


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


  getObjectPtr<TTree>("events")->Fill();

}
