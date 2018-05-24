//+
// File : PhysicsObjectsDQMModule.cc
// Description : Module to monitor physics objects on HLT
//
// Author : Boqun Wang, University of Cincinnati
// Date : May - 2018
//-

#include <dqm/modules/PhysicsObjectsDQM/PhysicsObjectsDQMModule.h>
#include <TLorentzVector.h>
#include <TDirectory.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <iostream>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PhysicsObjectsDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PhysicsObjectsDQMModule::PhysicsObjectsDQMModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor Physics Objects Quality");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TriggerIdentifier", m_triggerIdentifier,
           "Trigger identifier string used to select events for the histograms", std::string("software_trigger_cut&hlt&accept_hadron"));
  addParam("PI0PListName", m_pi0PListName, "Name of the pi0 particle list", std::string("pi0:physDQM"));
  addParam("KS0PListName", m_ks0PListName, "Name of the KS0 particle list", std::string("K_S0:physDQM"));
}

void PhysicsObjectsDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("PhysicsObjects")->cd();

  m_h_mKS0 = new TH1F("mKS0", "KS0 Invariant Mass", 40, 0.48, 0.52);
  m_h_mKS0->SetXTitle("M(K_{S}^{0}) [GeV]");

  m_h_mPI0 = new TH1F("mPI0", "PI0 Invariant Mass", 50, 0.10, 0.15);
  m_h_mPI0->SetXTitle("M(#pi^{0}) [GeV]");

  oldDir->cd();
}


void PhysicsObjectsDQMModule::initialize()
{
  REG_HISTOGRAM

  StoreObjPtr<SoftwareTriggerResult> result;
  result.isRequired();
}


void PhysicsObjectsDQMModule::beginRun()
{
  m_h_mKS0->Reset();
  m_h_mPI0->Reset();
}


void PhysicsObjectsDQMModule::endRun()
{
}


void PhysicsObjectsDQMModule::terminate()
{
}


void PhysicsObjectsDQMModule::event()
{
  StoreObjPtr<SoftwareTriggerResult> result;
  if (!result.isValid()) {
    B2FATAL("SoftwareTriggerResult object not available but needed to select events for the histograms.");
  }

  const bool accepted = (result->getResult(m_triggerIdentifier) == SoftwareTriggerCutResult::c_accept);

  if (accepted == false) return;

  StoreObjPtr<ParticleList> pi0Particles(m_pi0PListName);
  StoreObjPtr<ParticleList> ks0Particles(m_ks0PListName);

  if (pi0Particles.isValid() && abs(pi0Particles->getPDGCode()) == 111) {
    for (unsigned int i = 0; i < pi0Particles->getListSize(); i++) {
      Particle* pi0 = pi0Particles->getParticle(i);
      m_h_mPI0->Fill(pi0->getMass());
    }
  }
  if (ks0Particles.isValid() && abs(pi0Particles->getPDGCode()) == 310) {
    for (unsigned int i = 0; i < ks0Particles->getListSize(); i++) {
      Particle* ks0 = ks0Particles->getParticle(i);
      m_h_mKS0->Fill(ks0->getMass());
    }
  }
}
