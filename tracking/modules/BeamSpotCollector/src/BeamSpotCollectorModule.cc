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

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/ReferenceFrame.h>
#include <mdst/dataobjects/TrackFitResult.h>

using namespace Belle2;
using namespace std;

#include <iostream>

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
  cout << "Radek init of the trees" << endl;
  //describeProcess("CaTest::prepare");
  std::string objectName = "tracks";
  //Data object creation --------------------------------------------------
  TTree* tree = new TTree(objectName.c_str(), "");

  tree->Branch<int>("event", &m_evt);
  tree->Branch<int>("exp", &m_exp);
  tree->Branch<int>("run", &m_run);
  tree->Branch<double>("time", &m_time);

  tree->Branch<double>("mu0_d0", &m_mu0_d0);
  tree->Branch<double>("mu0_z0", &m_mu0_z0);
  tree->Branch<double>("mu0_phi0", &m_mu0_phi0);
  tree->Branch<double>("mu0_tanlambda", &m_mu0_tanlambda);

  tree->Branch<double>("mu1_d0", &m_mu1_d0);
  tree->Branch<double>("mu1_z0", &m_mu1_z0);
  tree->Branch<double>("mu1_phi0", &m_mu1_phi0);
  tree->Branch<double>("mu1_tanlambda", &m_mu1_tanlambda);


  // We register the objects so that our framework knows about them.
  // Don't try and hold onto the pointers or fill these objects directly
  // Use the getObjectPtr functions to access collector objects
  registerObject<TTree>(objectName, tree);
}

void BeamSpotCollectorModule::startRun()
{
  // Clear counters and private histograms
  // Ccollected (registered) histograms reset automatically by CAF
}


void BeamSpotCollectorModule::closeRun()
{
  cout << "Closing run Radek " << __LINE__ << endl;

}

void BeamSpotCollectorModule::collect()
{
  //describeProcess("CaTest::collect()");
  m_evt  = m_emd->getEvent();
  m_run  = m_emd->getRun();
  m_exp  = m_emd->getExperiment();
  m_time = m_emd->getTime();


  StoreObjPtr<ParticleList> Y4SParticles(m_Y4SPListName);
  const auto& frame = ReferenceFrame::GetCurrent();

  //for(int i = 0; i < Y4SParticles.getNDaughters(); ++i)

  if (!Y4SParticles.isValid() || abs(Y4SParticles->getPDGCode()) != 300553)
    return;
  //cout << "Radek is here " << __LINE__ << endl;
  //cout << "Size of the Y4SParticles " << Y4SParticles->getListSize() << endl;
  if (Y4SParticles->getListSize() !=  1)
    return;


  std::vector<int> indxes =  Y4SParticles->getParticle(0)->getDaughterIndices();
  if (indxes.size() != 2) return;

  const Particle* part0 = Y4SParticles->getParticle(0)->getDaughter(0);
  const Particle* part1 = Y4SParticles->getParticle(0)->getDaughter(1);
  const TrackFitResult* tr0 = part0->getTrackFitResult();
  const TrackFitResult* tr1 = part1->getTrackFitResult();

  m_mu0_d0        = tr0->getD0();
  m_mu0_z0        = tr0->getZ0();
  m_mu0_phi0      = tr0->getPhi0();
  m_mu0_tanlambda = tr0->getTanLambda();
  m_mu0_omega     = tr0->getOmega();


  m_mu1_d0        = tr1->getD0();
  m_mu1_z0        = tr1->getZ0();
  m_mu1_phi0      = tr1->getPhi0();
  m_mu1_tanlambda = tr1->getTanLambda();
  m_mu1_omega     = tr1->getOmega();


  getObjectPtr<TTree>("tracks")->Fill();


  /*
  if (Y4SParticles.isValid() && abs(Y4SParticles->getPDGCode()) == 300553) {

    if (Y4SParticles->getListSize() > 1)
      return;

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
  */

}
