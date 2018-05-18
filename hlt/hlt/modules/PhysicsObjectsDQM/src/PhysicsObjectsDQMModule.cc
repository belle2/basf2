//+
// File : PhysicsObjectsDQMModule.cc
// Description : Module to monitor physics objects on HLT
//
// Author : Boqun Wang, University of Cincinnati
// Date : May - 2018
//-

#include <hlt/hlt/modules/PhysicsObjectsDQM/PhysicsObjectsDQMModule.h>
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

  StoreObjPtr<ParticleList> pionParticles("pi+:HLT");
  StoreObjPtr<ParticleList> gammaParticles("gamma:HLT");

  if (pionParticles.isValid() && gammaParticles.isValid()) {
    unsigned int n_pi = pionParticles->getListSize();
    if (n_pi >= 2) {
      for (unsigned int i = 0; i < n_pi - 1; i++) {
        Particle* pi1 = pionParticles->getParticle(i);
        for (unsigned int j = i + 1; j < n_pi; j++) {
          Particle* pi2 = pionParticles->getParticle(j);
          if (pi1->getCharge()*pi2->getCharge() > 0) continue;
          TLorentzVector V4p1 = pi1->get4Vector();
          TLorentzVector V4p2 = pi2->get4Vector();
          double mks0 = (V4p1 + V4p2).M();
          if (mks0 > 0.48 && mks0 < 0.52) {
            m_h_mKS0->Fill(mks0);
          }
        }
      }
    }

    unsigned int n_gam = gammaParticles->getListSize();
    if (n_gam >= 2) {
      for (unsigned int i = 0; i < n_gam - 1; i++) {
        Particle* gam1 = gammaParticles->getParticle(i);
        for (unsigned int j = i + 1; j < n_gam; j++) {
          Particle* gam2 = gammaParticles->getParticle(j);
          TLorentzVector V4p1 = gam1->get4Vector();
          TLorentzVector V4p2 = gam2->get4Vector();
          double mpi0 = (V4p1 + V4p2).M();
          if (mpi0 > 0.10 && mpi0 < 0.15) {
            m_h_mPI0->Fill(mpi0);
          }
        }
      }
    }
  }
}
