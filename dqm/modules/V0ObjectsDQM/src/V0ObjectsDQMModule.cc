/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : V0ObjectsDQMModule.cc
// Description : Module to monitor displaced vertices on HLT
//-

#include <dqm/modules/V0ObjectsDQM/V0ObjectsDQMModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <analysis/dataobjects/ParticleList.h>
#include <TDirectory.h>

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

  addParam("V0PListName", m_V0PListName, "Name of the vertexed particle list", std::string("K_S0:V0DQM"));
}

void V0ObjectsDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("V0Objects");
  oldDir->cd("V0Objects");

  for (int j = 0; j < 32; j++) {
    m_h_xvsy[j] = new TH2F(Form("xvsy[%i]", j), Form("xvsy[%i]", j), 200, -10, 10, 200, -10, 10);
    m_h_xvsy[j]->SetXTitle("x [cm]");
    m_h_xvsy[j]->SetYTitle("y [cm]");
    m_h_xvsy[j]->SetStats(kFALSE);
  }
  m_h_xvsz = new TH2F("xvsz", "xvsz", 1500, -75, 75, 400, -10, 10);
  m_h_xvsz->SetXTitle("z [cm]");
  m_h_xvsz->SetYTitle("x [cm]");
  m_h_xvsz->SetStats(kFALSE);

  oldDir->cd();
}


void V0ObjectsDQMModule::initialize()
{
  REG_HISTOGRAM

}


void V0ObjectsDQMModule::beginRun()
{
  for (int j = 0; j < 32; j++) {
    m_h_xvsy[j]->Reset();
  }
  m_h_xvsz->Reset();
}


void V0ObjectsDQMModule::event()
{

  StoreObjPtr<ParticleList> V0Particles(m_V0PListName);

  if (V0Particles.isValid()) {
    for (unsigned int i = 0; i < V0Particles->getListSize(); i++) {
      Particle* V0 = V0Particles->getParticle(i);
      //Get the vertex position, fill accordingly
      float vtxx = V0->getX();
      float vtxy = V0->getY();
      float vtxz = V0->getZ();
      if (fabs(vtxz) < 75 && fabs(vtxx) < 10 && fabs(vtxy) < 10) {
        m_h_xvsy[int(floor((vtxz + 75) / 5))]->Fill(vtxx, vtxy);
        if (vtxz <= -5. || vtxz >= 8.) m_h_xvsz->Fill(vtxz, vtxx);
      }
    }
  }

}
