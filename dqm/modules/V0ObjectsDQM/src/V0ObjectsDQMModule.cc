//+
// File : V0ObjectsDQMModule.cc
// Description : Module to monitor displaced vertices on HLT
//
// Author : Bryan Fulsom, PNNL
// Date : 2019-01-17
//-

#include <dqm/modules/V0ObjectsDQM/V0ObjectsDQMModule.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <TLorentzVector.h>
#include <TDirectory.h>
#include <iostream>
#include <cmath>
#include "TH2F.h"
#include "TH1F.h"

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(V0ObjectsDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

V0ObjectsDQMModule::V0ObjectsDQMModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor displaced vertices");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TriggerIdentifier", m_triggerIdentifier,
           "Trigger identifier string used to select events for the histograms", std::string("software_trigger_cut&hlt&accept_hadron"));
  addParam("V0PListName", m_V0PListName, "Name of the vertexed particle list", std::string("K_S0:V0DQM"));
}

void V0ObjectsDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("V0Objects")->cd();

  for (int j = 0; j < 32; j++) {
    m_h_xvsy[j] = new TH2F(Form("xvsy[%i]", j), Form("xvsy[%i]", j), 1000, -10, 10, 1000, -10, 10);
    m_h_xvsy[j]->SetXTitle("x [cm]");
    m_h_xvsy[j]->SetYTitle("y [cm]");
  }

  oldDir->cd();
}


void V0ObjectsDQMModule::initialize()
{
  REG_HISTOGRAM

  StoreObjPtr<SoftwareTriggerResult> result;
  result.isRequired();
}


void V0ObjectsDQMModule::beginRun()
{
  for (int j = 0; j < 32; j++) {
    m_h_xvsy[j]->Reset();
  }
}


void V0ObjectsDQMModule::endRun()
{
}


void V0ObjectsDQMModule::terminate()
{
}


void V0ObjectsDQMModule::event()
{

  StoreObjPtr<SoftwareTriggerResult> result;
  if (!result.isValid()) {
    B2FATAL("SoftwareTriggerResult object not available but needed to select events for the histograms.");
  }

  const bool accepted = (result->getResult(m_triggerIdentifier) == SoftwareTriggerCutResult::c_accept);

  if (accepted == false) return;

  StoreObjPtr<ParticleList> V0Particles(m_V0PListName);

  if (V0Particles.isValid()) {
    for (unsigned int i = 0; i < V0Particles->getListSize(); i++) {
      Particle* V0 = V0Particles->getParticle(i);
      //Get the vertex position, fill accordingly
      float vtxz = V0->getZ();
      if (vtxz > -75 && vtxz < 75) {
        m_h_xvsy[int(floor((vtxz + 75) / 5))]->Fill(V0->getX(), V0->getY());
      }
    }
  }

}
