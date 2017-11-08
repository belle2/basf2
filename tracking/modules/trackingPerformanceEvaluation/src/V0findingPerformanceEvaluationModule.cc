/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackingPerformanceEvaluation/V0findingPerformanceEvaluationModule.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/RelationsObject.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>

#include <framework/geometry/BFieldManager.h>

#include <vxd/geometry/GeoCache.h>

#include <root/TTree.h>
#include <root/TAxis.h>
#include <root/TObject.h>

#include <boost/foreach.hpp>
#include <vector>
#include <utility>

#include <typeinfo>
#include <cxxabi.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(V0findingPerformanceEvaluation)

V0findingPerformanceEvaluationModule::V0findingPerformanceEvaluationModule() :
  Module()
{

  setDescription("This module evaluates the V0 finding package performance");

  addParam("outputFileName", m_rootFileName, "Name of output root file.",
           std::string("V0findingPerformanceEvaluation_output.root"));
  addParam("V0sName", m_V0sName, "Name of V0 collection.", std::string("V0ValidationVertexs"));
  addParam("MCParticlesName", m_MCParticlesName, "Name of MC Particle collection.", std::string(""));

}

V0findingPerformanceEvaluationModule::~V0findingPerformanceEvaluationModule()
{

}

void V0findingPerformanceEvaluationModule::initialize()
{
  StoreArray<MCParticle>::required(m_MCParticlesName);
  StoreArray<V0ValidationVertex>::required(m_V0sName);

  //create list of histograms to be saved in the rootfile
  m_histoList = new TList;
  m_histoList_multiplicity = new TList;
  m_histoList_efficiency = new TList;
  m_histoList_purity = new TList;
  m_histoList_trkQuality = new TList;

  //set the ROOT File
  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //now create histograms

  //multiplicity histograms
  m_multiplicityV0s = createHistogram1D("h1nV0", "number of V0s per MC Particle", 8, -0.5, 7.5, "# V0s", m_histoList_multiplicity);

  m_multiplicityMCParticles = createHistogram1D("h1nMCPrtcl", "number of MCParticles per V0s", 5, -0.5, 4.5,
                                                "# MCParticles", m_histoList_multiplicity);

  m_MCParticlePDGcode = createHistogram1D("h1PDGcode", "PDG code of MCParticles", 6244, -3122, 3122,
                                          "PDG code", m_histoList_multiplicity);


  //vertex and momentum parameters errors
  m_h1_vtxX_err = createHistogram1D("h1vtxXerr", "vtxX error", 100, 0, 0.1, "#sigma_{vtxX} (cm)", m_histoList);
  m_h1_vtxY_err = createHistogram1D("h1vtxYerr", "vtxY error", 100, 0, 0.1, "#sigma_{vtxY} (cm)", m_histoList);
  m_h1_vtxZ_err = createHistogram1D("h1vtxZerr", "vtxZ error", 100, 0, 0.3, "#sigma_{vtxZ} (cm)", m_histoList);
  m_h2_vtxTvsR_err = createHistogram2D("h2vtxTerrVsR", "vtxT error vs R", 100, 0, 100, "R (cm)", 100, 0, 0.3, "#sigma_{vtxT} (cm)",
                                       m_histoList);
  //  m_h1_mom_err = createHistogram1D("h1momerr", "mom error", 100, 0, 0.1, "#sigma_{p} (GeV/c)", m_histoList);
  //  m_h1_mass_err = createHistogram1D("h1masserr", "mass error", 100, 0, 1, "#sigma_{m} (GeV/c2)", m_histoList);
  //vertex and momentum parameters residuals
  m_h1_vtxX_res = createHistogram1D("h1vtxXres", "vtxX resid", 100, -0.2, 0.2, "vtxX resid (cm)", m_histoList);
  m_h1_vtxY_res = createHistogram1D("h1vtxYres", "vtxY resid", 100, -0.2, 0.2, "vtxY resid (cm)", m_histoList);
  m_h1_vtxZ_res = createHistogram1D("h1vtxZres", "vtxZ resid", 100, -0.5, 0.5, "vtxZ resid (cm)", m_histoList);
  m_h1_mom_res = createHistogram1D("h1momres", "mom resid", 1000, -0.5, 0.5, "mom resid (GeV/c)", m_histoList);
  m_h1_mass_res = createHistogram1D("h1massres", "mass resid", 500, -0.3, 0.3, "mass resid (GeV/c)", m_histoList);

  //vertex and momentum parameters pulls
  m_h1_vtxX_pll = createHistogram1D("h1vtxXpll", "vtxX pull", 100, -5, 5, "vtxX pull", m_histoList);
  m_h1_vtxY_pll = createHistogram1D("h1vtxYpll", "vtxY pull", 100, -5, 5, "vtxY pull", m_histoList);
  m_h1_vtxZ_pll = createHistogram1D("h1vtxZpll", "vtxZ pull", 100, -5, 5, "vtxZ pull", m_histoList);
  //  m_h1_mom_pll = createHistogram1D("h1mompll", "mom pull", 100, -5, 5, "momX pull", m_histoList);
  //  m_h1_mass_pll = createHistogram1D("h1masspll", "mass pull", 100, -5, 5, "momY pull", m_histoList);


  m_h1_ChiSquare = createHistogram1D("h1Chi2", "Chi2 of the fit", 100, 0, 20, "Chi2", m_histoList_trkQuality);

  m_h1_nMatchedDau = createHistogram1D("h1nMatchedDau", "Number of Matched MCParticle Daughters", 3, -0.5, 2.5, "# matched dau",
                                       m_histoList);


  m_h2_mom = createHistogram2D("h2mom", "reco VS true momentum", 100, 0, 3, "V0 mom (GeV/c)", 100, 0, 3, "MC mom (GeV/c)",
                               m_histoList);
  m_h2_mass = createHistogram2D("h2mass", "reco VS true mass", 100, 0, 1.5, "V0 mass (GeV/c2)", 100, 0, 1.5, "MC mass (GeV/c)",
                                m_histoList);

  //histograms to produce efficiency plots
  Double_t bins_pt[9 + 1] = {0, 0.05, 0.1, 0.15, 0.2, 0.3, 0.5, 1, 2, 3.5}; //GeV/c
  Double_t bins_theta[10 + 1] = {0, 0.25, 0.5, 0.75, 0.75 + 0.32, 0.75 + 2 * 0.32, 0.75 + 3 * 0.32, 0.75 + 4 * 0.32, 0.75 + 5 * 0.32, 2.65, TMath::Pi()};
  Double_t bins_phi[14 + 1];
  Double_t width_phi = 2 * TMath::Pi() / 14;
  for (int bin = 0; bin < 14 + 1; bin++)
    bins_phi[bin] = - TMath::Pi() + bin * width_phi;

  m_h1_MCParticle_R = createHistogram1D("h1nMCParticleVSr", "entry per MCParticles", 50, 0, 20, "transverse L", m_histoList);

  m_h1_V0sPerMCParticle_R = (TH1F*)duplicateHistogram("h1nV0perMCvsR", "entry per V0 related to a MCParticle",   m_h1_MCParticle_R,
                                                      m_histoList);


  m_h3_MCParticle = createHistogram3D("h3MCParticle", "entry per MCParticle",
                                      9, bins_pt, "p_{t} (GeV/c)",
                                      10, bins_theta, "#theta",
                                      14, bins_phi, "#phi" /*, m_histoList*/);

  m_h3_V0sPerMCParticle = (TH3F*)duplicateHistogram("h3V0sPerMCParticle",
                                                    "entry per V0 connected to a MCParticle",
                                                    m_h3_MCParticle /*, m_histoList*/);

  m_h3_V0s = (TH3F*)duplicateHistogram("h3V0s", "entry per V0",
                                       m_h3_MCParticle /*, m_histoList*/);

  //histograms to produce purity plots
  m_h3_MCParticlesPerV0 = (TH3F*)duplicateHistogram("h3MCParticlesPerV0",
                                                    "entry per MCParticle connected to a V0",
                                                    m_h3_MCParticle /*, m_histoList*/);
}

void V0findingPerformanceEvaluationModule::beginRun()
{

}

void V0findingPerformanceEvaluationModule::event()
{

  StoreArray<MCParticle> mcParticles(m_MCParticlesName);

  B2Vector3D magField = BFieldManager::getField(0, 0, 0) / Unit::T;

  B2DEBUG(99, "+++++ 1. loop on MCParticles");
  BOOST_FOREACH(MCParticle & mcParticle, mcParticles) {

    if (! isV0(mcParticle))
      continue;

    int nMatchedDau =  nMatchedDaughters(mcParticle);
    m_h1_nMatchedDau->Fill(nMatchedDau);

    //proceed only in case the MCParticle daughters have both one associated reconstructed track:
    if (nMatchedDau != 2)
      continue;

    int pdgCode = mcParticle.getPDG();
    B2DEBUG(99, "MCParticle has PDG code " << pdgCode);
    m_MCParticlePDGcode->Fill(mcParticle.getPDG());

    MCParticleInfo mcParticleInfo(mcParticle, magField);

    TVector3 MC_prodvtx = mcParticle.getVertex();
    TVector3 MC_vtx = mcParticle.getDecayVertex();
    float MC_mom = mcParticle.getMomentum().Mag();
    float MC_mass = mcParticle.getMass();
    TVector3 MC_FL = MC_vtx - MC_prodvtx;
    float flightR = sqrt(MC_FL.X() * MC_FL.X() + MC_FL.Y() * MC_FL.Y());
    m_h1_MCParticle_R->Fill(flightR);

    m_h3_MCParticle->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());

    //1. retrieve all the V0s related to the MCParticle

    //1.0 check if there is a V0
    RelationVector<V0ValidationVertex> V0s_toMCParticle =
      DataStore::getRelationsWithObj<V0ValidationVertex>(&mcParticle, m_V0sName);

    m_multiplicityV0s->Fill(V0s_toMCParticle.size());

    if (V0s_toMCParticle.size() > 0)
      m_h3_V0sPerMCParticle->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());

    for (int v0 = 0; v0 < (int)V0s_toMCParticle.size(); v0++) {

      TVector3 V0_vtx = V0s_toMCParticle[v0]->getVertexPosition();
      float V0_mom = V0s_toMCParticle[v0]->getFittedMomentum();
      float V0_chi2 = V0s_toMCParticle[v0]->getVertexChi2();
      float V0_mass = V0s_toMCParticle[v0]->getFittedInvariantMass();
      TMatrixDSym V0_cov = V0s_toMCParticle[v0]->getVertexPositionCovariance();

      m_h1_vtxX_err->Fill(sqrt(V0_cov[0][0]));
      m_h1_vtxY_err->Fill(sqrt(V0_cov[1][1]));
      m_h1_vtxZ_err->Fill(sqrt(V0_cov[2][2]));
      m_h2_vtxTvsR_err->Fill(flightR, sqrt(V0_cov[0][0] + V0_cov[1][1]));

      m_h1_V0sPerMCParticle_R->Fill(flightR);

      m_h1_vtxX_res->Fill(V0_vtx.X() -  MC_vtx.X());
      m_h1_vtxY_res->Fill(V0_vtx.Y() -  MC_vtx.Y());
      m_h1_vtxZ_res->Fill(V0_vtx.Z() -  MC_vtx.Z());

      m_h1_mom_res->Fill(V0_mom -  MC_mom);
      m_h2_mom->Fill(V0_mom, MC_mom);
      m_h1_mass_res->Fill(V0_mass -  MC_mass);
      m_h2_mass->Fill(V0_mass, MC_mass);

      m_h1_vtxX_pll->Fill((V0_vtx.X() -  MC_vtx.X()) / sqrt(V0_cov[0][0]));
      m_h1_vtxY_pll->Fill((V0_vtx.Y() -  MC_vtx.Y()) / sqrt(V0_cov[1][1]));
      m_h1_vtxZ_pll->Fill((V0_vtx.Z() -  MC_vtx.Z()) / sqrt(V0_cov[2][2]));

      m_h1_ChiSquare->Fill(V0_chi2);

    }


  }


  B2DEBUG(99, "+++++ 2. loop on V0s");


  StoreArray<V0ValidationVertex> V0s(m_V0sName);


  BOOST_FOREACH(V0ValidationVertex & v0, V0s) {

    int nMCParticles = 0;

    //    m_h3_V0s>Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
    //2. retrieve all the MCParticles related to the V0s
    RelationVector<MCParticle> MCParticles_fromV0 =
      DataStore::getRelationsWithObj<MCParticle>(&v0, m_MCParticlesName);

    nMCParticles = MCParticles_fromV0.size();

    if (nMCParticles == 0)
      continue;

    MCParticleInfo mcParticleInfo(* MCParticles_fromV0[0], magField);
    m_h3_MCParticlesPerV0->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
    m_multiplicityMCParticles->Fill(nMCParticles);

  }

}

void V0findingPerformanceEvaluationModule::endRun()
{

  double num = 0;
  double den = 0;

  for (int bin = 1; bin < m_multiplicityV0s->GetNbinsX(); bin ++)
    num += m_multiplicityV0s->GetBinContent(bin + 1);
  den = m_multiplicityV0s->GetEntries();
  double efficiency = num / den ;
  double efficiencyErr =  sqrt(efficiency * (1 - efficiency)) / sqrt(den);

  double nMCParticles = 0;
  for (int bin = 1; bin < m_multiplicityMCParticles->GetNbinsX(); bin ++)
    nMCParticles += m_multiplicityMCParticles->GetBinContent(bin + 1);
  double purity = nMCParticles / m_multiplicityMCParticles->GetEntries();
  double purityErr =  sqrt(purity * (1 - purity)) / sqrt(m_multiplicityMCParticles->GetEntries());

  B2INFO("");
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  B2INFO("~ V0 Finding Performance Evaluation ~ SHORT SUMMARY ~");
  B2INFO("");
  B2INFO(" + overall:");
  B2INFO("   efficiency = (" << efficiency * 100 << " +/- " << efficiencyErr * 100 << ")% ");
  B2INFO("       purity = (" << purity * 100 << " +/- " << purityErr * 100 << ")% ");
  B2INFO("");
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

void V0findingPerformanceEvaluationModule::terminate()
{

  TH1F* h_eff_R   = (TH1F*)duplicateHistogram("h_eff_R", "efficiency vs R", m_h1_MCParticle_R, m_histoList_efficiency);

  for (int bin = 0; bin < h_eff_R->GetXaxis()->GetNbins(); bin++) {
    float num = m_h1_V0sPerMCParticle_R->GetBinContent(bin + 1);
    float den = m_h1_MCParticle_R->GetBinContent(bin + 1);
    double eff = 0;
    double err = 0;

    if (den > 0) {
      eff = (double)num / den;
      err = sqrt(eff * (1 - eff)) / sqrt(den);
    }

    h_eff_R->SetBinContent(bin + 1, eff);
    h_eff_R->SetBinError(bin + 1, err);
  }

  addEfficiencyPlots(m_histoList_efficiency, m_h3_V0sPerMCParticle, m_h3_MCParticle);

  addInefficiencyPlots(m_histoList_efficiency, m_h3_V0sPerMCParticle, m_h3_MCParticle);

  //  addPurityPlots(m_histoList_purity, m_h3_MCParticlesPerV0, m_h3_V0s);

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TDirectory* oldDir = gDirectory;

    TDirectory* dir_multiplicity = oldDir->mkdir("multiplicity");
    dir_multiplicity->cd();
    TIter nextH_multiplicity(m_histoList_multiplicity);
    TObject* obj;
    while ((obj = nextH_multiplicity()))
      obj->Write();

    TDirectory* dir_efficiency = oldDir->mkdir("efficiency");
    dir_efficiency->cd();
    TIter nextH_efficiency(m_histoList_efficiency);
    while ((obj = nextH_efficiency()))
      obj->Write();

    TDirectory* dir_purity = oldDir->mkdir("purity");
    dir_purity->cd();
    TIter nextH_purity(m_histoList_purity);
    while ((obj = nextH_purity()))
      obj->Write();

    TDirectory* dir_trkQuality = oldDir->mkdir("trkQuality");
    dir_trkQuality->cd();
    TIter nextH_trkQuality(m_histoList_trkQuality);
    while ((obj = nextH_trkQuality()))
      obj->Write();

    TIter nextH(m_histoList);
    while ((obj = nextH()))
      obj->Write();


    m_rootFilePtr->Close();
  }

}


bool V0findingPerformanceEvaluationModule::isV0(const MCParticle& the_mcParticle)
{

  bool isGamma = false;
  if (abs(the_mcParticle.getPDG()) == 22)
    isGamma = true;

  bool isK_S0 = false;
  if (abs(the_mcParticle.getPDG()) == 310)
    isK_S0 = true;

  bool isK_0 = false;
  if (abs(the_mcParticle.getPDG()) == 311)
    isK_0 = true;

  bool isLambda = false;
  if (abs(the_mcParticle.getPDG()) == 3122)
    isLambda = true;

  bool twoProngs = false;
  bool twoChargedProngs = false;

  if (the_mcParticle.getDaughters().size() == 2)
    twoProngs = true;

  if (twoProngs)
    if (the_mcParticle.getDaughters()[0]->getCharge() *  the_mcParticle.getDaughters()[1]->getCharge() < 0)
      twoChargedProngs = true;

  return ((isGamma || isK_S0 || isK_0 || isLambda) && twoChargedProngs);

}

int V0findingPerformanceEvaluationModule::nMatchedDaughters(const MCParticle& the_mcParticle)
{

  int nMatchedDau = 0;

  std::vector< MCParticle* > MCPart_dau = the_mcParticle.getDaughters();

  bool first = false;
  bool second = false;

  RelationVector<Track> Tracks_fromMCParticle_0 = DataStore::getRelationsWithObj<Track>(MCPart_dau[0]);
  if (Tracks_fromMCParticle_0.size() > 0)
    first = true;

  RelationVector<Track> Tracks_fromMCParticle_1 = DataStore::getRelationsWithObj<Track>(MCPart_dau[1]);
  if (Tracks_fromMCParticle_1.size() > 0)
    second = true;


  if (first)
    nMatchedDau++;

  if (second)
    nMatchedDau++;


  return nMatchedDau;

}
