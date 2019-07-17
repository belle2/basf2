/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gaetano de Marino, Tadeas Bilka                          *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/BeamSpotCollector/BeamSpotCollectorModule.h>

#include <analysis/variables/ContinuumSuppressionVariables.h>
#include <analysis/variables/VertexVariables.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <analysis/utility/ReferenceFrame.h>
#include <TLorentzVector.h>
#include <TStyle.h>
#include <iostream>
#include <framework/logging/Logger.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BeamSpotCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BeamSpotCollectorModule::BeamSpotCollectorModule() : CalibrationCollectorModule()
{
  //Set module properties

  setDescription("Collect data for BeamSpot calibration algorithm using the position of mu+mu- events");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("Y4SPListName", m_Y4SPListName, "Name of the Y4S particle list", std::string("Upsilon(4S):IPDQM"));
}

void BeamSpotCollectorModule::prepare()
{
  auto m_h_x = new TH1F("Y4S_Vertex.X", "IP position - coord. X", 1000, -0.5, 0.5);
  m_h_x->SetXTitle("IP_coord. X [cm]");
  registerObject<TH1F>("Y4S_Vertex.X", m_h_x);

  auto m_h_y = new TH1F("Y4S_Vertex.Y", "IP position - coord. Y", 1000, -0.5, 0.5);
  m_h_y->SetXTitle("IP_coord. Y [cm]");
  registerObject<TH1F>("Y4S_Vertex.Y", m_h_y);

  auto m_h_z = new TH1F("Y4S_Vertex.Z", "IP position - coord. Z", 2000, -2, 2);
  m_h_z->SetXTitle("IP_coord. Z [cm]");
  registerObject<TH1F>("Y4S_Vertex.Z", m_h_z);

//   m_h_px = new TH1F("Y4S_Vertex.pX", "Total momentum in lab. frame - coord. X", 100, -2, 2);
//   m_h_px->SetXTitle("pX [GeV/c]");
//   m_h_py = new TH1F("Y4S_Vertex.pY", "Total momentum in lab. frame - coord. Y", 100, -2, 2);
//   m_h_py->SetXTitle("pY [GeV/c]");
//   m_h_pz = new TH1F("Y4S_Vertex.pZ", "Total momentum in lab. frame - coord. Z", 100, 1, 5);
//   m_h_pz->SetXTitle("pZ [GeV/c]");
//   m_h_E = new TH1F("Y4S_Vertex.E", "Energy in lab. frame", 100, 8, 13);
//   m_h_E->SetXTitle("E [GeV]");

  m_h_pull = new TH1F("Y4S_pull.Y", "Pull - coord. Y", 100, -10, 10);
  m_h_pull->SetXTitle("(y_{reco}- y_{med})/#sigma_{y}^{reco}");
  m_h_pull->GetXaxis()->SetTitleOffset(1.2);
  m_h_pull->GetXaxis()->SetTitleSize(.04);
  m_h_y_risol = new TH1F("Y4S_Res.Y", "Resolution - coord. Y", 1000, -1, 1);
  m_h_y_risol->GetXaxis()->SetTitleOffset(1.2);
  m_h_y_risol->SetXTitle("(y_{reco}- y_{med}) [cm]");

  m_h_temp = new TH1F("Y4S_temp.Y", "temp - coord. Y", 2000, -2, 2);
  m_h_temp->SetXTitle("IP_coord. Y [cm]");

//   m_h_xx = new TH1F("Y4S_Prod.XX", "IP position - prod. XX", 1000, 0, 0.5);
//   m_h_xx->SetXTitle("IP_prod. XX [cm^{2} ]");
//   m_h_yy = new TH1F("Y4S_Prod.YY", "IP position - prod. YY", 1000, 0, 0.5);
//   m_h_yy->SetXTitle("IP_prod. YY [cm^{2} ]");
//   m_h_zz = new TH1F("Y4S_Prod.ZZ", "IP position - prod. ZZ", 2000, 0, 4);
//   m_h_zz->SetXTitle("IP_prod. ZZ [cm^{2} ]");
//   m_h_xy = new TH1F("Y4S_Prod.XY", "IP position - prod. XY", 1000, -1, 1);
//   m_h_xy->SetXTitle("IP_prod. XY [cm^{2} ]");
//   m_h_yz = new TH1F("Y4S_Prod.YZ", "IP position - prod. YZ", 1000, -1, 1);
//   m_h_yz->SetXTitle("IP_prod. YZ [cm^{2} ]");
//   m_h_xz = new TH1F("Y4S_Prod.XZ", "IP position - prod. XZ", 1000, -1, 1);
//   m_h_xz->SetXTitle("IP_prod. XZ [cm^{2} ]");

  auto m_h_cov_x_x = new TH1F("Var.X", "X Variance", 500, 0., 0.005);
  m_h_cov_x_x->SetXTitle("Var. X [cm^{2} ]");
  registerObject<TH1F>("Var.X", m_h_cov_x_x);

  auto m_h_cov_y_y = new TH1F("Var.Y", "Y Variance", 500, 0., 0.005);
  m_h_cov_y_y->SetXTitle("Var. Y [cm^{2} ]");
  registerObject<TH1F>("Var.Y", m_h_cov_y_y);

  auto m_h_cov_z_z = new TH1F("Var.Z", "Z Variance", 500, 0., 0.005);
  m_h_cov_z_z->SetXTitle("Var. Z [cm^{2} ]");
  registerObject<TH1F>("Var.Z", m_h_cov_z_z);

//   m_h_cov_x_y = new TH1F("Covar.XY", "XY Covariance", 1000, -0.005, 0.005);
//   m_h_cov_x_y->SetXTitle("Var. XY [cm^{2} ]");
//   m_h_cov_x_z = new TH1F("Covar.XZ", "XZ Covariance", 1000, -0.005, 0.005);
//   m_h_cov_x_z->SetXTitle("Covar. XZ [cm^{2} ]");
//   m_h_cov_y_z = new TH1F("Covar.YZ", "YZ Covariance", 1000, -0.005, 0.005);
//   m_h_cov_y_z->SetXTitle("Covar. YZ [cm^{2} ]");

}

void BeamSpotCollectorModule::startRun()
{
  // Clear counters and private histograms
  // Ccollected (registered) histograms reset automatically by CAF
  m_h_pull->Reset();
  m_h_temp->Reset();
  m_h_y_risol->Reset();
  m_v_y.clear();
  m_err_y.clear();
  m_r = 0;
}


void BeamSpotCollectorModule::closeRun()
{
  auto m_h_x = getObjectPtr<TH1F>("Y4S_Vertex.X");
  m_h_x->GetXaxis()->SetRangeUser(m_h_x->GetMean(1) - 5 * m_h_x->GetRMS(1), m_h_x->GetMean(1) + 5 * m_h_x->GetRMS(1));

  auto m_h_y = getObjectPtr<TH1F>("Y4S_Vertex.Y");
  m_h_y->GetXaxis()->SetRangeUser(m_h_y->GetMean(1) - 5 * m_h_y->GetRMS(1), m_h_y->GetMean(1) + 5 * m_h_y->GetRMS(1));

  auto m_h_z = getObjectPtr<TH1F>("Y4S_Vertex.Z");
  m_h_z->GetXaxis()->SetRangeUser(m_h_z->GetMean(1) - 5 * m_h_z->GetRMS(1), m_h_z->GetMean(1) + 5 * m_h_z->GetRMS(1));

//   m_h_xy->GetXaxis()->SetRangeUser(m_h_xy->GetMean(1) - 5 * m_h_xy->GetRMS(1), m_h_xy->GetMean(1) + 5 * m_h_xy->GetRMS(1));
//   m_h_xz->GetXaxis()->SetRangeUser(m_h_xz->GetMean(1) - 5 * m_h_xz->GetRMS(1), m_h_xz->GetMean(1) + 5 * m_h_xz->GetRMS(1));
//   m_h_yz->GetXaxis()->SetRangeUser(m_h_yz->GetMean(1) - 5 * m_h_yz->GetRMS(1), m_h_yz->GetMean(1) + 5 * m_h_yz->GetRMS(1));
//   m_h_xx->GetXaxis()->SetRangeUser(m_h_xx->GetMean(1) - 5 * m_h_xx->GetRMS(1), m_h_xx->GetMean(1) + 5 * m_h_xx->GetRMS(1));
//   m_h_yy->GetXaxis()->SetRangeUser(m_h_yy->GetMean(1) - 5 * m_h_yy->GetRMS(1), m_h_yy->GetMean(1) + 5 * m_h_yy->GetRMS(1));
//   m_h_zz->GetXaxis()->SetRangeUser(m_h_zz->GetMean(1) - 5 * m_h_zz->GetRMS(1), m_h_zz->GetMean(1) + 5 * m_h_zz->GetRMS(1));


  m_h_pull->GetXaxis()->SetRangeUser(m_h_pull->GetMean(1) - 5 * m_h_pull->GetRMS(1), m_h_pull->GetMean(1) + 5 * m_h_pull->GetRMS(1));
  m_h_y_risol->GetXaxis()->SetRangeUser(m_h_y_risol->GetMean(1) - 5 * m_h_y_risol->GetRMS(1),
                                        m_h_y_risol->GetMean(1) + 5 * m_h_y_risol->GetRMS(1));

  auto m_h_cov_x_x = getObjectPtr<TH1F>("Var.X");
  m_h_cov_x_x->GetXaxis()->SetRangeUser(m_h_cov_x_x->GetMean(1) - 5 * m_h_cov_x_x->GetRMS(1),
                                        m_h_cov_x_x->GetMean(1) + 5 * m_h_cov_x_x->GetRMS(1));

  auto m_h_cov_y_y = getObjectPtr<TH1F>("Var.Y");
  m_h_cov_y_y->GetXaxis()->SetRangeUser(m_h_cov_y_y->GetMean(1) - 5 * m_h_cov_y_y->GetRMS(1),
                                        m_h_cov_y_y->GetMean(1) + 5 * m_h_cov_y_y->GetRMS(1));

  auto m_h_cov_z_z = getObjectPtr<TH1F>("Var.Z");
  m_h_cov_z_z->GetXaxis()->SetRangeUser(m_h_cov_z_z->GetMean(1) - 5 * m_h_cov_z_z->GetRMS(1),
                                        m_h_cov_z_z->GetMean(1) + 5 * m_h_cov_z_z->GetRMS(1));

//   m_h_cov_x_z->GetXaxis()->SetRangeUser(m_h_cov_x_z->GetMean(1) - 5 * m_h_cov_x_z->GetRMS(1),
//                                         m_h_cov_x_z->GetMean(1) + 5 * m_h_cov_x_z->GetRMS(1));
//   m_h_cov_x_y->GetXaxis()->SetRangeUser(m_h_cov_x_y->GetMean(1) - 5 * m_h_cov_x_y->GetRMS(1),
//                                         m_h_cov_x_y->GetMean(1) + 5 * m_h_cov_x_y->GetRMS(1));
//   m_h_cov_y_z->GetXaxis()->SetRangeUser(m_h_cov_y_z->GetMean(1) - 5 * m_h_cov_y_z->GetRMS(1),
//                                         m_h_cov_y_z->GetMean(1) + 5 * m_h_cov_y_z->GetRMS(1));
}

void BeamSpotCollectorModule::finish()
{
}

void BeamSpotCollectorModule::collect()
{

  StoreObjPtr<ParticleList> Y4SParticles(m_Y4SPListName);
  const auto& frame = ReferenceFrame::GetCurrent();


  if (Y4SParticles.isValid() && abs(Y4SParticles->getPDGCode()) == 300553) {

    for (unsigned int i = 0; i < Y4SParticles->getListSize(); i++) {

      Particle* Y4S = Y4SParticles->getParticle(i);
      TVector3 IPVertex = frame.getVertex(Y4S);
      const auto& errMatrix = Y4S->getVertexErrorMatrix();

      getObjectPtr<TH1F>("Y4S_Vertex.X")->Fill(IPVertex.X());
      getObjectPtr<TH1F>("Y4S_Vertex.Y")->Fill(IPVertex.Y());
      getObjectPtr<TH1F>("Y4S_Vertex.Z")->Fill(IPVertex.Z());

//       m_h_xy->Fill(IPVertex.X()*IPVertex.Y());
//       m_h_xz->Fill(IPVertex.X()*IPVertex.Z());
//       m_h_yz->Fill(IPVertex.Y()*IPVertex.Z());
//       m_h_xx->Fill(IPVertex.X()*IPVertex.X());
//       m_h_yy->Fill(IPVertex.Y()*IPVertex.Y());
//       m_h_zz->Fill(IPVertex.Z()*IPVertex.Z());

      m_h_temp->Fill(IPVertex.Y());
      getObjectPtr<TH1F>("Var.X")->Fill(errMatrix(0, 0));
      getObjectPtr<TH1F>("Var.Y")->Fill(errMatrix(1, 1));
      getObjectPtr<TH1F>("Var.Z")->Fill(errMatrix(2, 2));

//       m_h_cov_x_y->Fill(errMatrix(0, 1));
//       m_h_cov_x_z->Fill(errMatrix(0, 2));
//       m_h_cov_y_z->Fill(errMatrix(1, 2));
//       m_h_px->Fill(frame.getMomentum(Y4S).Px());
//       m_h_py->Fill(frame.getMomentum(Y4S).Py());
//       m_h_pz->Fill(frame.getMomentum(Y4S).Pz());
//       m_h_E->Fill(frame.getMomentum(Y4S).E());
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
