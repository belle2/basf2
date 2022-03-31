/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own include. */
#include <dqm/modules/PhysicsObjectsDQM/IPDQMModule.h>

/* Belle 2 headers. */
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/ReferenceFrame.h>
#include <framework/core/Environment.h>
#include <framework/logging/LogConfig.h>
#include <framework/logging/Logger.h>
#include <hlt/utilities/Units.h>

/* ROOT headers. */
#include <TDirectory.h>

using namespace Belle2;

REG_MODULE(IPDQM)

IPDQMModule::IPDQMModule() : HistoModule()
{
  setDescription("Monitor the position and the size of the interaction point using mu+mu- events");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("Y4SPListName", m_Y4SPListName, "Name of the Y4S particle list", std::string("Upsilon(4S):IPDQM"));
  addParam("onlineMode", m_onlineMode, "Mode of the online processing ('hlt' or 'expressreco')", std::string("expressreco"));
  if (m_onlineMode == "hlt") {
    m_no_units = HLTUnits::max_hlt_units;
  } else if (m_onlineMode == "expressreco") {
    m_no_units = ExpressRecoUnits::max_ereco_units;
  } else {
    if (Environment::Instance().getRealm() == LogConfig::c_Online)
      B2FATAL("Unknown online processing mode"
              << LogVar("Set mode", m_onlineMode));
    else // We are likely running offline (for testing purposes, for example): let's assume we use only 1 unit.
      m_no_units = 1;
  }
  m_size_per_unit = m_size / m_no_units;
}

void IPDQMModule::defineHisto()
{
  TDirectory* newDirectory{gDirectory->mkdir("IPMonitoring")};
  TDirectory::TContext context{gDirectory, newDirectory};
  // Common set of plots for HLT and ExpressReco
  m_h_x = new TH1F("Y4S_Vertex.X", "IP position - coord. X", 1000, -0.5, 0.5);
  m_h_x->SetXTitle("IP_coord. X [cm]");
  m_h_y = new TH1F("Y4S_Vertex.Y", "IP position - coord. Y", 1000, -0.5, 0.5);
  m_h_y->SetXTitle("IP_coord. Y [cm]");
  m_h_z = new TH1F("Y4S_Vertex.Z", "IP position - coord. Z", 2000, -2, 2);
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
    m_h_pull = new TH1F("Y4S_pull.Y", "Pull - coord. Y", 100, -10, 10);
    m_h_pull->SetXTitle("(y_{reco}- y_{med})/#sigma_{y}^{reco}");
    m_h_pull->GetXaxis()->SetTitleOffset(1.2);
    m_h_pull->GetXaxis()->SetTitleSize(.04);
    m_h_y_risol = new TH1F("Y4S_Res.Y", "Resolution - coord. Y", 1000, -1, 1);
    m_h_y_risol->GetXaxis()->SetTitleOffset(1.2);
    m_h_y_risol->SetXTitle("(y_{reco}- y_{med}) [cm]");
    m_h_temp = new TH1F("Y4S_temp.Y", "temp - coord. Y", 2000, -2, 2);
    m_h_temp->SetXTitle("IP_coord. Y [cm]");
    m_h_xx = new TH1F("Y4S_Prod.XX", "IP position - prod. XX", 1000, 0, 0.5);
    m_h_xx->SetXTitle("IP_prod. XX [cm^{2} ]");
    m_h_yy = new TH1F("Y4S_Prod.YY", "IP position - prod. YY", 1000, 0, 0.5);
    m_h_yy->SetXTitle("IP_prod. YY [cm^{2} ]");
    m_h_zz = new TH1F("Y4S_Prod.ZZ", "IP position - prod. ZZ", 2000, 0, 4);
    m_h_zz->SetXTitle("IP_prod. ZZ [cm^{2} ]");
    m_h_xy = new TH1F("Y4S_Prod.XY", "IP position - prod. XY", 1000, -1, 1);
    m_h_xy->SetXTitle("IP_prod. XY [cm^{2} ]");
    m_h_yz = new TH1F("Y4S_Prod.YZ", "IP position - prod. YZ", 1000, -1, 1);
    m_h_yz->SetXTitle("IP_prod. YZ [cm^{2} ]");
    m_h_xz = new TH1F("Y4S_Prod.XZ", "IP position - prod. XZ", 1000, -1, 1);
    m_h_xz->SetXTitle("IP_prod. XZ [cm^{2} ]");
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
  REG_HISTOGRAM
}


void IPDQMModule::beginRun()
{
  m_h_x->Reset();
  m_h_y->Reset();
  m_h_z->Reset();
  if (m_onlineMode == "expressreco") {
    m_h_pull->Reset();
    m_h_temp->Reset();
    m_h_y_risol->Reset();
    m_h_cov_x_x->Reset();
    m_h_cov_y_y->Reset();
    m_h_cov_z_z->Reset();
    m_h_cov_x_y->Reset();
    m_h_cov_x_z->Reset();
    m_h_cov_y_z->Reset();
    m_h_xx->Reset();
    m_h_yy->Reset();
    m_h_zz->Reset();
    m_h_xy->Reset();
    m_h_xz->Reset();
    m_h_yz->Reset();
    m_h_px->Reset();
    m_h_py->Reset();
    m_h_pz->Reset();
    m_h_E->Reset();
    m_v_y.clear();
    m_err_y.clear();
    m_r = 0;
  }
}


void IPDQMModule::endRun()
{
  m_h_x->GetXaxis()->SetRangeUser(m_h_x->GetMean(1) - 5 * m_h_x->GetRMS(1), m_h_x->GetMean(1) + 5 * m_h_x->GetRMS(1));
  m_h_y->GetXaxis()->SetRangeUser(m_h_y->GetMean(1) - 5 * m_h_y->GetRMS(1), m_h_y->GetMean(1) + 5 * m_h_y->GetRMS(1));
  m_h_z->GetXaxis()->SetRangeUser(m_h_z->GetMean(1) - 5 * m_h_z->GetRMS(1), m_h_z->GetMean(1) + 5 * m_h_z->GetRMS(1));
  if (m_onlineMode == "expressreco") {
    m_h_xy->GetXaxis()->SetRangeUser(m_h_xy->GetMean(1) - 5 * m_h_xy->GetRMS(1), m_h_xy->GetMean(1) + 5 * m_h_xy->GetRMS(1));
    m_h_xz->GetXaxis()->SetRangeUser(m_h_xz->GetMean(1) - 5 * m_h_xz->GetRMS(1), m_h_xz->GetMean(1) + 5 * m_h_xz->GetRMS(1));
    m_h_yz->GetXaxis()->SetRangeUser(m_h_yz->GetMean(1) - 5 * m_h_yz->GetRMS(1), m_h_yz->GetMean(1) + 5 * m_h_yz->GetRMS(1));
    m_h_xx->GetXaxis()->SetRangeUser(m_h_xx->GetMean(1) - 5 * m_h_xx->GetRMS(1), m_h_xx->GetMean(1) + 5 * m_h_xx->GetRMS(1));
    m_h_yy->GetXaxis()->SetRangeUser(m_h_yy->GetMean(1) - 5 * m_h_yy->GetRMS(1), m_h_yy->GetMean(1) + 5 * m_h_yy->GetRMS(1));
    m_h_zz->GetXaxis()->SetRangeUser(m_h_zz->GetMean(1) - 5 * m_h_zz->GetRMS(1), m_h_zz->GetMean(1) + 5 * m_h_zz->GetRMS(1));
    m_h_pull->GetXaxis()->SetRangeUser(m_h_pull->GetMean(1) - 5 * m_h_pull->GetRMS(1), m_h_pull->GetMean(1) + 5 * m_h_pull->GetRMS(1));
    m_h_y_risol->GetXaxis()->SetRangeUser(m_h_y_risol->GetMean(1) - 5 * m_h_y_risol->GetRMS(1),
                                          m_h_y_risol->GetMean(1) + 5 * m_h_y_risol->GetRMS(1));
    m_h_cov_x_x->GetXaxis()->SetRangeUser(m_h_cov_x_x->GetMean(1) - 5 * m_h_cov_x_x->GetRMS(1),
                                          m_h_cov_x_x->GetMean(1) + 5 * m_h_cov_x_x->GetRMS(1));
    m_h_cov_y_y->GetXaxis()->SetRangeUser(m_h_cov_y_y->GetMean(1) - 5 * m_h_cov_y_y->GetRMS(1),
                                          m_h_cov_y_y->GetMean(1) + 5 * m_h_cov_y_y->GetRMS(1));
    m_h_cov_z_z->GetXaxis()->SetRangeUser(m_h_cov_z_z->GetMean(1) - 5 * m_h_cov_z_z->GetRMS(1),
                                          m_h_cov_z_z->GetMean(1) + 5 * m_h_cov_z_z->GetRMS(1));
    m_h_cov_x_z->GetXaxis()->SetRangeUser(m_h_cov_x_z->GetMean(1) - 5 * m_h_cov_x_z->GetRMS(1),
                                          m_h_cov_x_z->GetMean(1) + 5 * m_h_cov_x_z->GetRMS(1));
    m_h_cov_x_y->GetXaxis()->SetRangeUser(m_h_cov_x_y->GetMean(1) - 5 * m_h_cov_x_y->GetRMS(1),
                                          m_h_cov_x_y->GetMean(1) + 5 * m_h_cov_x_y->GetRMS(1));
    m_h_cov_y_z->GetXaxis()->SetRangeUser(m_h_cov_y_z->GetMean(1) - 5 * m_h_cov_y_z->GetRMS(1),
                                          m_h_cov_y_z->GetMean(1) + 5 * m_h_cov_y_z->GetRMS(1));
  }
}

void IPDQMModule::terminate()
{
}

void IPDQMModule::event()
{
  StoreObjPtr<ParticleList> Y4SParticles(m_Y4SPListName);
  const auto& frame = ReferenceFrame::GetCurrent();

  if (Y4SParticles.isValid() && abs(Y4SParticles->getPDGCode()) == 300553) {

    for (unsigned int i = 0; i < Y4SParticles->getListSize(); i++) {

      Particle* Y4S = Y4SParticles->getParticle(i);
      B2Vector3D IPVertex = frame.getVertex(Y4S);
      const auto& errMatrix = Y4S->getVertexErrorMatrix();
      m_h_x->Fill(IPVertex.X());
      m_h_y->Fill(IPVertex.Y());
      m_h_z->Fill(IPVertex.Z());

      if (m_onlineMode == "expressreco") {
        m_h_xy->Fill(IPVertex.X()*IPVertex.Y());
        m_h_xz->Fill(IPVertex.X()*IPVertex.Z());
        m_h_yz->Fill(IPVertex.Y()*IPVertex.Z());
        m_h_xx->Fill(IPVertex.X()*IPVertex.X());
        m_h_yy->Fill(IPVertex.Y()*IPVertex.Y());
        m_h_zz->Fill(IPVertex.Z()*IPVertex.Z());
        m_h_temp->Fill(IPVertex.Y());
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
        m_err_y.push_back(std::sqrt(errMatrix(1, 1)));
        m_v_y.push_back(IPVertex.Y());

        if (m_r == m_size_per_unit) {
          m_h_temp->GetQuantiles(1, &m_median, &m_quantile);
          for (unsigned int u = 0; u < m_v_y.size(); u++) {
            m_h_y_risol->Fill(m_v_y.at(u) - m_median);
            m_h_pull->Fill((m_v_y.at(u) - m_median) / m_err_y.at(u));
          }
          m_r = 0;
          m_v_y.clear();
          m_err_y.clear();
        }
        m_r += 1;
      }
    }

  }

}
