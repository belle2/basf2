//+
// File : PhysicsObjectsDQMModule.cc
// Description : Module to monitor physics objects on HLT
//
// Author : Boqun Wang,
// Date : May - 2018
//-

#include <hlt/hlt/modules/PhysicsObjectsDQM/PhysicsObjectsDQMModule.h>
#include <TLorentzVector.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

using namespace std;
using namespace Belle2;


//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PhysicsObjectsDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PhysicsObjectsDQMModule::PhysicsObjectsDQMModule() : HistoModule(), m_pionParticles("pi+:HLT"), m_gammaParticles("gamma:HLT")
{
  //Set module properties

  setDescription("Monitor Physics Trigger");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("CutString", m_cut_string,
           "Cut to filter events", std::string("software_trigger_cut&hlt&accept_hadron"));
}



PhysicsObjectsDQMModule::~PhysicsObjectsDQMModule()
{
}

void PhysicsObjectsDQMModule::defineHisto()
{

  m_h_mKS0 = new TH1F("mKS0", "KS0 Invariant Mass", 40, 0.48, 0.52);
  m_h_mKS0->SetXTitle("M_{K_{S}^{0}} [GeV]");

  m_h_mPI0 = new TH1F("mPI0", "PI0 Invariant Mass", 50, 0.10, 0.15);
  m_h_mPI0->SetXTitle("M_{#pi^{0}} [GeV]");

}


void PhysicsObjectsDQMModule::initialize()
{
  REG_HISTOGRAM

  m_pionParticles.isRequired();
  m_gammaParticles.isRequired();
}


void PhysicsObjectsDQMModule::beginRun()
{
  B2INFO("beginRun called.");
  m_h_mKS0->Reset();
  m_h_mPI0->Reset();
}


void PhysicsObjectsDQMModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void PhysicsObjectsDQMModule::terminate()
{
  B2INFO("terminate called");
}


void PhysicsObjectsDQMModule::event()
{
  StoreObjPtr<SoftwareTriggerResult> result;
  if (!result.isValid()) {
    B2FATAL("SoftwareTriggerResult object not available but needed to generate the ROI payload.");
  }
  const bool accepted =
    result->getResult(m_cut_string.c_str()) ==
    SoftwareTriggerCutResult::c_accept;

  if (accepted == false) return;

  unsigned int n_pi = m_pionParticles->getListSize();
  if (n_pi >= 2) {
    for (unsigned int i = 0; i < n_pi - 1; i++) {
      Particle* pi1 = m_pionParticles->getParticle(i);
      for (unsigned int j = i + 1; j < n_pi; j++) {
        Particle* pi2 = m_pionParticles->getParticle(j);
        TLorentzVector V4p1 = pi1->get4Vector();
        TLorentzVector V4p2 = pi2->get4Vector();
        double mks0 = (V4p1 + V4p2).M();
        if (mks0 > 0.48 && mks0 < 0.52)
          m_h_mKS0->Fill(mks0);
      }
    }
  }

  unsigned int n_gam = m_gammaParticles->getListSize();
  if (n_gam >= 2) {
    for (unsigned int i = 0; i < n_gam - 1; i++) {
      Particle* gam1 = m_gammaParticles->getParticle(i);
      for (unsigned int j = i + 1; j < n_gam; j++) {
        Particle* gam2 = m_gammaParticles->getParticle(j);
        TLorentzVector V4p1 = gam1->get4Vector();
        TLorentzVector V4p2 = gam2->get4Vector();
        double mpi0 = (V4p1 + V4p2).M();
        if (mpi0 > 0.10 && mpi0 < 0.15)
          m_h_mPI0->Fill(mpi0);
      }
    }
  }
}
