//+
// File : PhysicsObjectsDQMModule.cc
// Description : Module to monitor physics objects on HLT
//
// Author : Boqun Wang, University of Cincinnati
// Date : May - 2018
//-

#include <dqm/modules/PhysicsObjectsDQM/PhysicsObjectsDQMModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/variables/EventShapeVariables.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <TDirectory.h>
#include <map>

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
           "Trigger identifier string used to select events for the histograms", std::string("software_trigger_cut&skim&accept_hadron"));
  addParam("PI0PListName", m_pi0PListName, "Name of the pi0 particle list", std::string("pi0:physDQM"));
  addParam("KS0PListName", m_ks0PListName, "Name of the KS0 particle list", std::string("K_S0:physDQM"));
}

void PhysicsObjectsDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("PhysicsObjects")->cd();

  m_h_mKS0 = new TH1F("mKS0", "KS0 Invariant Mass", 20, 0.48, 0.52);
  m_h_mKS0->SetXTitle("M(K_{S}^{0}) [GeV]");

  m_h_mPI0 = new TH1F("mPI0", "pi0 Invariant Mass", 25, 0.10, 0.15);
  m_h_mPI0->SetXTitle("M(#pi^{0}) [GeV]");

  m_h_R2 = new TH1F("R2", "Event Level R2", 36, 0, 1.2);
  m_h_R2->SetXTitle("R2");

  oldDir->cd();
}


void PhysicsObjectsDQMModule::initialize()
{
  REG_HISTOGRAM

  StoreObjPtr<SoftwareTriggerResult> result;
  result.isOptional();
}


void PhysicsObjectsDQMModule::beginRun()
{
  m_h_mKS0->Reset();
  m_h_mPI0->Reset();
  m_h_R2->Reset();
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
    B2WARNING("SoftwareTriggerResult object not available but needed to select events for the histograms.");
    return;
  }

  const std::map<std::string, int>& results = result->getResults();
  if (results.find(m_triggerIdentifier) == results.end()) {
    B2WARNING("PhysicsObjectsDQM: Can't find trigger identifier: " << m_triggerIdentifier);
    return;
  }

  const bool accepted = (result->getResult(m_triggerIdentifier) == SoftwareTriggerCutResult::c_accept);

  if (accepted == false) return;

  StoreObjPtr<ParticleList> pi0Particles(m_pi0PListName);
  StoreObjPtr<ParticleList> ks0Particles(m_ks0PListName);

  double R2 = Belle2::Variable::foxWolframR2(nullptr);
  m_h_R2->Fill(R2);

  if (pi0Particles.isValid() && abs(pi0Particles->getPDGCode()) == Const::pi0.getPDGCode()) {
    for (unsigned int i = 0; i < pi0Particles->getListSize(); i++) {
      Particle* pi0 = pi0Particles->getParticle(i);
      m_h_mPI0->Fill(pi0->getMass());
    }
  }
  if (ks0Particles.isValid() && abs(ks0Particles->getPDGCode()) == Const::Kshort.getPDGCode()) {
    for (unsigned int i = 0; i < ks0Particles->getListSize(); i++) {
      Particle* ks0 = ks0Particles->getParticle(i);
      m_h_mKS0->Fill(ks0->getMass());
    }
  }
}
