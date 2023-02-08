/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own include. */
#include <dqm/modules/PhysicsObjectsDQM/IPDQMModule.h>

/* Basf2 headers. */
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/ReferenceFrame.h>
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TDirectory.h>

/* C++ headers. */
#include <cmath>

using namespace Belle2;

REG_MODULE(IPDQM);

IPDQMModule::IPDQMModule() : HistoModule()
{
  setDescription("Monitor the position and the size of the interaction point using mu+mu- events");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("Y4SPListName", m_Y4SPListName, "Name of the Y4S particle list", std::string("Upsilon(4S):IPDQM"));
  addParam("onlineMode", m_onlineMode, "Mode of the online processing ('hlt' or 'expressreco')", std::string("expressreco"));
}

void IPDQMModule::defineHisto()
{
  TDirectory* newDirectory{gDirectory->mkdir("IPMonitoring")};
  TDirectory::TContext context{gDirectory, newDirectory};
  // Common set of plots for HLT and ExpressReco
  // Let's add a suffix to the plots when we run on HLT: necessary for the analysis module
  std::string suffix = (m_onlineMode == "hlt") ? "_hlt" : "";
  m_h_x = new TH1F(std::string{"Y4S_Vertex.X" + suffix}.c_str(), "IP position - coord. X", 1000, -0.5, 0.5);
  m_h_x->SetXTitle("IP_coord. X [cm]");
  m_h_y = new TH1F(std::string{"Y4S_Vertex.Y" + suffix}.c_str(), "IP position - coord. Y", 1000, -0.5, 0.5);
  m_h_y->SetXTitle("IP_coord. Y [cm]");
  m_h_z = new TH1F(std::string{"Y4S_Vertex.Z" + suffix}.c_str(), "IP position - coord. Z", 2000, -2.0, 2.0);
  m_h_z->SetXTitle("IP_coord. Z [cm]");
  if (m_onlineMode == "expressreco") {
    m_h_px = new TH1F("Y4S_Vertex.pX", "Total momentum in lab. frame - coord. X", 100, -2, 2);
    m_h_px->SetXTitle("pX [GeV/c]");
    m_h_py = new TH1F("Y4S_Vertex.pY", "Total momentum in lab. frame - coord. Y", 100, -2, 2);
    m_h_py->SetXTitle("pY [GeV/c]");
    m_h_pz = new TH1F("Y4S_Vertex.pZ", "Total momentum in lab. frame - coord. Z", 100, 1, 5);
    m_h_pz->SetXTitle("pZ [GeV/c]");
    m_h_E = new TH1F("Y4S_Vertex.E", "Energy in lab. frame", 100, 8, 13);
    m_h_E->SetXTitle("E [GeV]");
    m_h_cov_x_x = new TH1F("Var.X", "X Variance", 500, 0., 0.005);
    m_h_cov_x_x->SetXTitle("Var. X [cm^{2} ]");
    m_h_cov_y_y = new TH1F("Var.Y", "Y Variance", 500, 0., 0.005);
    m_h_cov_y_y->SetXTitle("Var. Y [cm^{2} ]");
    m_h_cov_z_z = new TH1F("Var.Z", "Z Variance", 500, 0., 0.005);
    m_h_cov_z_z->SetXTitle("Var. Z [cm^{2} ]");
    m_h_cov_x_y = new TH1F("Covar.XY", "XY Covariance", 1000, -0.005, 0.005);
    m_h_cov_x_y->SetXTitle("Var. XY [cm^{2} ]");
    m_h_cov_x_z = new TH1F("Covar.XZ", "XZ Covariance", 1000, -0.005, 0.005);
    m_h_cov_x_z->SetXTitle("Covar. XZ [cm^{2} ]");
    m_h_cov_y_z = new TH1F("Covar.YZ", "YZ Covariance", 1000, -0.005, 0.005);
    m_h_cov_y_z->SetXTitle("Covar. YZ [cm^{2} ]");
  }
  // We currently don't have specific plots for HLT: in case, they can be added here.
}

void IPDQMModule::initialize()
{
  if (not(m_onlineMode == "hlt" or m_onlineMode == "expressreco")) {
    B2FATAL("Unknown online processing mode" << LogVar("Set mode", m_onlineMode));
  }
  REG_HISTOGRAM
}

void IPDQMModule::beginRun()
{
  m_h_x->Reset();
  m_h_y->Reset();
  m_h_z->Reset();
  if (m_onlineMode == "expressreco") {
    m_h_cov_x_x->Reset();
    m_h_cov_y_y->Reset();
    m_h_cov_z_z->Reset();
    m_h_cov_x_y->Reset();
    m_h_cov_x_z->Reset();
    m_h_cov_y_z->Reset();
    m_h_px->Reset();
    m_h_py->Reset();
    m_h_pz->Reset();
    m_h_E->Reset();
  }
}

void IPDQMModule::event()
{
  StoreObjPtr<ParticleList> Y4SParticles(m_Y4SPListName);
  if (Y4SParticles.isValid() && abs(Y4SParticles->getPDGCode()) == 300553) {
    const auto& frame = ReferenceFrame::GetCurrent();
    for (unsigned int i = 0; i < Y4SParticles->getListSize(); i++) {
      Particle* Y4S = Y4SParticles->getParticle(i);
      B2Vector3D IPVertex = frame.getVertex(Y4S);
      double IPX{IPVertex.X()};
      double IPY{IPVertex.Y()};
      double IPZ{IPVertex.Z()};
      if (std::abs(IPX) < 0.5 and std::abs(IPY) < 0.5 and std::abs(IPZ) < 2.0) { // in cm
        m_h_x->Fill(IPVertex.X());
        m_h_y->Fill(IPVertex.Y());
        m_h_z->Fill(IPVertex.Z());
        if (m_onlineMode == "expressreco") {
          const auto& errMatrix = Y4S->getVertexErrorMatrix();
          m_h_cov_x_x->Fill(errMatrix(0, 0));
          m_h_cov_y_y->Fill(errMatrix(1, 1));
          m_h_cov_z_z->Fill(errMatrix(2, 2));
          m_h_cov_x_y->Fill(errMatrix(0, 1));
          m_h_cov_x_z->Fill(errMatrix(0, 2));
          m_h_cov_y_z->Fill(errMatrix(1, 2));
          m_h_px->Fill(frame.getMomentum(Y4S).Px());
          m_h_py->Fill(frame.getMomentum(Y4S).Py());
          m_h_pz->Fill(frame.getMomentum(Y4S).Pz());
          m_h_E->Fill(frame.getMomentum(Y4S).E());
        }
      }
    }
  }
}
