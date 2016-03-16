/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/V0findingPerformanceEvaluation/V0findingPerformanceEvaluationModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>

#include <geometry/bfieldmap/BFieldMap.h>

#include <vxd/geometry/GeoCache.h>


#include <root/TTree.h>
#include <root/TAxis.h>
#include <root/TObject.h>

#include <boost/foreach.hpp>

#include <typeinfo>
#include <cxxabi.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(V0findingPerformanceEvaluation)

V0findingPerformanceEvaluationModule::V0findingPerformanceEvaluationModule() :
  Module()
  , m_rootFilePtr(NULL)
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

  //set the ROOT File
  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //now create histograms

  //multiplicity histograms
  m_multiplicityV0s = createHistogram1D("h1nV0", "number of V0s per MC Particle", 8, -0.5, 7.5, "# V0s", m_histoList);

  m_multiplicityMCParticles = createHistogram1D("h1nMCPrtcl", "number of MCParticles per V0s", 5, -0.5, 4.5,
                                                "# MCParticles", m_histoList);

  //vertex and momentum parameters errors
  m_h1_vtxX_err = createHistogram1D("h1vtxXerr", "vtxX error", 100, 0, 0.1, "#sigma_{vtxX} (cm)", m_histoList);
  m_h1_vtxY_err = createHistogram1D("h1vtxYerr", "vtxY error", 100, 0, 0.1, "#sigma_{vtxY} (cm)", m_histoList);
  m_h1_vtxZ_err = createHistogram1D("h1vtxZerr", "vtxZ error", 100, 0, 0.1, "#sigma_{vtxZ} (cm)", m_histoList);
  m_h1_momX_err = createHistogram1D("h1momXerr", "momX error", 100, 0, 0.1, "#sigma_{momX} (cm)", m_histoList);
  m_h1_momY_err = createHistogram1D("h1momYerr", "momY error", 100, 0, 0.1, "#sigma_{momY} (cm)", m_histoList);
  m_h1_momZ_err = createHistogram1D("h1momZerr", "momZ error", 100, 0, 0.1, "#sigma_{momZ} (cm)", m_histoList);
  //vertex and momentum parameters residuals
  m_h1_vtxX_res = createHistogram1D("h1vtxXres", "vtxX resol", 100, -0.2, 0.2, "vtxX resid (cm)", m_histoList);
  m_h1_vtxY_res = createHistogram1D("h1vtxYres", "vtxY resol", 100, -0.2, 0.2, "vtxY resid (cm)", m_histoList);
  m_h1_vtxZ_res = createHistogram1D("h1vtxZres", "vtxZ resol", 100, -0.5, 0.5, "vtxZ resid (cm)", m_histoList);
  m_h1_momX_res = createHistogram1D("h1momXres", "momX resol", 100, -1, 1, "momX resid (GeV/c)", m_histoList);
  m_h1_momY_res = createHistogram1D("h1momYres", "momY resol", 100, -1, 1, "momY resid (GeV/c)", m_histoList);
  m_h1_momZ_res = createHistogram1D("h1momZres", "momZ resol", 100, -1, 1, "momZ resid (GeV/c)", m_histoList);

  //vertex and momentum parameters pulls
  m_h1_vtxX_pll = createHistogram1D("h1vtxXpll", "vtxX pllol", 100, -5, 5, "vtxX pull", m_histoList);
  m_h1_vtxY_pll = createHistogram1D("h1vtxYpll", "vtxY pllol", 100, -5, 5, "vtxY pull", m_histoList);
  m_h1_vtxZ_pll = createHistogram1D("h1vtxZpll", "vtxZ pllol", 100, -5, 5, "vtxZ pull", m_histoList);
  m_h1_momX_pll = createHistogram1D("h1momXpll", "momX pllol", 100, -5, 5, "momX pull", m_histoList);
  m_h1_momY_pll = createHistogram1D("h1momYpll", "momY pllol", 100, -5, 5, "momY pull", m_histoList);
  m_h1_momZ_pll = createHistogram1D("h1momZpll", "momZ pllol", 100, -5, 5, "momZ pull", m_histoList);


  m_h1_pValue = createHistogram1D("h1Chi2", "Chi2 of the fit", 100, 0, 20, "Chi2", m_histoList);


  //histograms to produce efficiency plots
  Double_t bins_pt[9 + 1] = {0, 0.05, 0.1, 0.15, 0.2, 0.3, 0.5, 1, 2, 3.5}; //GeV/c
  Double_t bins_theta[10 + 1] = {0, 0.25, 0.5, 0.75, 0.75 + 0.32, 0.75 + 2 * 0.32, 0.75 + 3 * 0.32, 0.75 + 4 * 0.32, 0.75 + 5 * 0.32, 2.65, TMath::Pi()};
  Double_t bins_phi[14 + 1];
  Double_t width_phi = 2 * TMath::Pi() / 14;
  for (int bin = 0; bin < 14 + 1; bin++)
    bins_phi[bin] = - TMath::Pi() + bin * width_phi;


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

  BFieldMap& bfieldMap = BFieldMap::Instance();
  TVector3 magField = bfieldMap.getBField(TVector3(0, 0, 0));

  bool hasV0 = false;

  B2DEBUG(99, "+++++ 1. loop on MCParticles");
  BOOST_FOREACH(MCParticle & mcParticle, mcParticles) {

    if (! isV0(mcParticle))
      continue;

    int pdgCode = mcParticle.getPDG();
    B2DEBUG(99, "MCParticle has PDG code " << pdgCode);

    int nV0s = 0;

    MCParticleInfo mcParticleInfo(mcParticle, magField);

    TVector3 MC_vtx = mcParticle.getDecayVertex();
    float MC_mom = mcParticle.getMomentum().Mag();
    float MC_mass = mcParticle.getMass();


    hasV0 = false;

    m_h3_MCParticle->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());

    //1. retrieve all the V0s related to the MCParticle

    //1.0 check if there is a V0
    RelationVector<V0ValidationVertex> V0s_toMCParticle =
      DataStore::getRelationsToObj<V0ValidationVertex>(&mcParticle, m_V0sName);

    if (V0s_toMCParticle.size() > 0)
      m_h3_V0sPerMCParticle->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());

    for (int v0 = 0; v0 < (int)V0s_toMCParticle.size(); v0++) {

      nV0s++;

      TVector3 V0_vtx = V0s_toMCParticle[v0]->getVertexPosition();
      float V0_mom = V0s_toMCParticle[v0]->getFittedMomentum();
      float V0_chi2 = V0s_toMCParticle[v0]->getVertexChi2();
      float V0_mass = V0s_toMCParticle[v0]->getFittedInvariantMass();
      TMatrixDSym V0_cov = V0s_toMCParticle[v0]->getVertexPositionCovariance();


      m_h1_vtxX_err->Fill(sqrt(V0_cov[0][0]));
      m_h1_vtxY_err->Fill(sqrt(V0_cov[1][1]));
      m_h1_vtxZ_err->Fill(sqrt(V0_cov[2][2]));

      m_h1_vtxX_res->Fill(V0_vtx.X() -  MC_vtx.X());
      m_h1_vtxY_res->Fill(V0_vtx.Y() -  MC_vtx.Y());
      m_h1_vtxZ_res->Fill(V0_vtx.Z() -  MC_vtx.Z());

      m_h1_vtxX_pll->Fill((V0_vtx.X() -  MC_vtx.X()) / sqrt(V0_cov[0][0]));
      m_h1_vtxY_pll->Fill((V0_vtx.Y() -  MC_vtx.Y()) / sqrt(V0_cov[1][1]));
      m_h1_vtxZ_pll->Fill((V0_vtx.Z() -  MC_vtx.Z()) / sqrt(V0_cov[2][2]));


    }


    //    fillTrackParams1DHistograms(fitResult, mcParticleInfo);


    m_multiplicityV0s->Fill(nV0s);

  }


  B2DEBUG(99, "+++++ 2. loop on V0s");

  //2. retrieve all the MCParticles related to the V0s
  StoreArray<V0ValidationVertex> V0s(m_V0sName);

  BOOST_FOREACH(V0ValidationVertex & v0, V0s) {

    int nMCParticles = 0;

    //check if the track has been fitted
    RelationVector<MCParticle> MCParticles_fromV0 =
      DataStore::getRelationsFromObj<MCParticle>(&v0, m_MCParticlesName);

    nMCParticles = MCParticles_fromV0.size();

    if (nMCParticles == 0)
      continue;

    m_h1_pValue->Fill(v0.getVertexChi2());

    //    m_h3_V0->Fill(momentum.Pt(), momentum.Theta(), momentum.Phi());

    //    const MCParticle* mcParticle = DataStore::getRelatedFromObj<MCParticle>(&v0);
    //    if (mcParticle == NULL)

    /*
    TVector3 momentum = fitResult->getMomentum();
    m_h3_Tracks->Fill(momentum.Pt(), momentum.Theta(), momentum.Phi());

    */
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


  addEfficiencyPlots(m_histoList);

  addInefficiencyPlots(m_histoList);

  addPurityPlots(m_histoList);

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TIter nextH(m_histoList);
    TObject* obj;
    while ((obj = nextH()))
      obj->Write();


    m_rootFilePtr->Close();
  }

}

TH1F* V0findingPerformanceEvaluationModule::createHistogram1D(const char* name, const char* title,
    Int_t nbins, Double_t min, Double_t max,
    const char* xtitle, TList* histoList)
{

  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);

  return h;
}


TH2F* V0findingPerformanceEvaluationModule::createHistogram2D(const char* name, const char* title,
    Int_t nbinsX, Double_t minX, Double_t maxX,
    const char* titleX,
    Int_t nbinsY, Double_t minY, Double_t maxY,
    const char* titleY, TList* histoList)
{

  TH2F* h = new TH2F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH3F* V0findingPerformanceEvaluationModule::createHistogram3D(const char* name, const char* title,
    Int_t nbinsX, Double_t minX, Double_t maxX,
    const char* titleX,
    Int_t nbinsY, Double_t minY, Double_t maxY,
    const char* titleY,
    Int_t nbinsZ, Double_t minZ, Double_t maxZ,
    const char* titleZ,
    TList* histoList)
{

  TH3F* h = new TH3F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY, nbinsZ, minZ, maxZ);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);
  h->GetZaxis()->SetTitle(titleZ);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH3F* V0findingPerformanceEvaluationModule::createHistogram3D(const char* name, const char* title,
    Int_t nbinsX, Double_t* binsX,
    const char* titleX,
    Int_t nbinsY, Double_t* binsY,
    const char* titleY,
    Int_t nbinsZ, Double_t* binsZ,
    const char* titleZ,
    TList* histoList)
{

  TH3F* h = new TH3F(name, title, nbinsX, binsX, nbinsY, binsY, nbinsZ, binsZ);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);
  h->GetZaxis()->SetTitle(titleZ);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH1* V0findingPerformanceEvaluationModule::duplicateHistogram(const char* newname, const char* newtitle,
    TH1* h, TList* histoList)
{

  TH1F* h1 =  dynamic_cast<TH1F*>(h);
  TH2F* h2 =  dynamic_cast<TH2F*>(h);
  TH3F* h3 =  dynamic_cast<TH3F*>(h);

  TH1* newh = 0;

  if (h1)
    newh = new TH1F(*h1);
  if (h2)
    newh = new TH2F(*h2);
  if (h3)
    newh = new TH3F(*h3);

  newh->SetName(newname);
  newh->SetTitle(newtitle);

  if (histoList)
    histoList->Add(newh);


  return newh;
}

TH1F* V0findingPerformanceEvaluationModule::createHistogramsRatio(const char* name, const char* title,
    TH1* hNum, TH1* hDen, bool isEffPlot,
    int axisRef)
{

  TH1F* h1den =  dynamic_cast<TH1F*>(hDen);
  TH1F* h1num =  dynamic_cast<TH1F*>(hNum);
  TH2F* h2den =  dynamic_cast<TH2F*>(hDen);
  TH2F* h2num =  dynamic_cast<TH2F*>(hNum);
  TH3F* h3den =  dynamic_cast<TH3F*>(hDen);
  TH3F* h3num =  dynamic_cast<TH3F*>(hNum);

  TH1* hden = 0;
  TH1* hnum = 0;

  if (h1den) {
    hden = new TH1F(*h1den);
    hnum = new TH1F(*h1num);
  }
  if (h2den) {
    hden = new TH2F(*h2den);
    hnum = new TH2F(*h2num);
  }
  if (h3den) {
    hden = new TH3F(*h3den);
    hnum = new TH3F(*h3num);
  }

  TAxis* the_axis;
  TAxis* the_other1;
  TAxis* the_other2;

  if (axisRef == 0) {
    the_axis = hden->GetXaxis();
    the_other1 = hden->GetYaxis();
    the_other2 = hden->GetZaxis();
  } else if (axisRef == 1) {
    the_axis = hden->GetYaxis();
    the_other1 = hden->GetXaxis();
    the_other2 = hden->GetZaxis();
  } else if (axisRef == 2) {
    the_axis = hden->GetZaxis();
    the_other1 = hden->GetXaxis();
    the_other2 = hden->GetYaxis();
  } else
    return NULL;


  TH1F* h;
  if (the_axis->GetXbins()->GetSize())
    h = new TH1F(name, title, the_axis->GetNbins(), (the_axis->GetXbins())->GetArray());
  else
    h = new TH1F(name, title, the_axis->GetNbins(), the_axis->GetXmin(), the_axis->GetXmax());
  h->GetXaxis()->SetTitle(the_axis->GetTitle());

  h->GetYaxis()->SetRangeUser(0.00001, 1);

  double num = 0;
  double den = 0;
  Int_t bin = 0;
  Int_t nBins = 0;


  for (int the_bin = 1; the_bin < the_axis->GetNbins() + 1; the_bin++) {

    num = 0;
    den = 0 ;

    for (int other1_bin = 1; other1_bin < the_other1->GetNbins() + 1; other1_bin++)
      for (int other2_bin = 1; other2_bin < the_other2->GetNbins() + 1; other2_bin++) {

        if (axisRef == 0) bin = hden->GetBin(the_bin, other1_bin, other2_bin);
        else if (axisRef == 1) bin = hden->GetBin(other1_bin, the_bin, other2_bin);
        else if (axisRef == 2) bin = hden->GetBin(other1_bin, other2_bin, the_bin);

        if (hden->IsBinUnderflow(bin))
          B2INFO("  bin = " << bin << "(" << the_bin << "," << other1_bin << "," << other2_bin << "), UNDERFLOW");
        if (hden->IsBinOverflow(bin))
          B2INFO("  bin = " << bin << "(" << the_bin << "," << other1_bin << "," << other2_bin << "), OVERFLOW");

        num += hnum->GetBinContent(bin);
        den += hden->GetBinContent(bin);

        nBins++;

      }
    double eff = 0;
    double err = 0;

    if (den > 0) {
      eff = (double)num / den;
      err = sqrt(eff * (1 - eff)) / sqrt(den);
    }

    if (isEffPlot) {
      h->SetBinContent(the_bin, eff);
      h->SetBinError(the_bin, err);
    } else {
      h->SetBinContent(the_bin, 1 - eff);
      h->SetBinError(the_bin, err);
    }

  }

  return h;

}

void  V0findingPerformanceEvaluationModule::fillTrackParams1DHistograms(const TrackFitResult* fitResult,
    MCParticleInfo mcParticleInfo)
{

  /*
  //track parameters errors
  double d0_err = sqrt((fitResult->getCovariance5())[0][0]);
  double phi_err =  sqrt((fitResult->getCovariance5())[1][1]);
  double omega_err = sqrt((fitResult->getCovariance5())[2][2]);
  double z0_err =  sqrt((fitResult->getCovariance5())[3][3]);
  double cotTheta_err = sqrt((fitResult->getCovariance5())[4][4]);

  //track parameters residuals:
  double d0_res = fitResult->getD0() - mcParticleInfo.getD0();

  //  double phi_res = fmod(fitResult->getPhi() - mcParticleInfo.getPhi() + 2*TMath::Pi(), 2 * TMath::Pi()); //giulia
  double phi_res = TMath::ASin(TMath::Sin(fitResult->getPhi() - mcParticleInfo.getPhi()));   //giulia

  double omega_res =  fitResult->getOmega() - mcParticleInfo.getOmega();
  double z0_res = fitResult->getZ0() - mcParticleInfo.getZ0();
  double cotTheta_res = fitResult->getCotTheta() - mcParticleInfo.getCotTheta();

  m_h1_d0_err->Fill(d0_err);
  m_h1_phi_err->Fill(phi_err);
  m_h1_omega_err->Fill(omega_err);
  m_h1_z0_err->Fill(z0_err);
  m_h1_cotTheta_err->Fill(cotTheta_err);

  m_h1_d0_res->Fill(d0_res);
  m_h1_phi_res->Fill(phi_res);
  m_h1_omega_res->Fill(omega_res);
  m_h1_z0_res->Fill(z0_res);
  m_h1_cotTheta_res->Fill(cotTheta_res);

  m_h1_d0_pll->Fill(d0_res / d0_err);
  m_h1_phi_pll->Fill(phi_res / phi_err);
  m_h1_omega_pll->Fill(omega_res / omega_err);
  m_h1_z0_pll->Fill(z0_res / z0_err);
  m_h1_cotTheta_pll->Fill(cotTheta_res / cotTheta_err);


  m_h2_OmegaerrOmegaVSpt->Fill(fitResult->getMomentum().Pt(), omega_err / mcParticleInfo.getOmega());
  */

}


void V0findingPerformanceEvaluationModule::addInefficiencyPlots(TList* histoList)
{

  //normalized to MCParticles
  TH1F* h_ineff_pt = createHistogramsRatio("hineffpt", "inefficiency VS pt, normalized to MCParticles", m_h3_V0sPerMCParticle,
                                           m_h3_MCParticle, false, 0);
  histoList->Add(h_ineff_pt);

  TH1F* h_ineff_theta = createHistogramsRatio("hinefftheta", "inefficiency VS #theta, normalized to MCParticles",
                                              m_h3_V0sPerMCParticle, m_h3_MCParticle, false, 1);
  histoList->Add(h_ineff_theta);

  TH1F* h_ineff_phi = createHistogramsRatio("hineffphi", "inefficiency VS #phi, normalized to MCParticles", m_h3_V0sPerMCParticle,
                                            m_h3_MCParticle, false, 2);
  histoList->Add(h_ineff_phi);

}

void V0findingPerformanceEvaluationModule::addEfficiencyPlots(TList* histoList)
{

  //normalized to MCParticles
  TH1F* h_eff_pt = createHistogramsRatio("heffpt", "efficiency VS pt, normalized to MCParticles", m_h3_V0sPerMCParticle,
                                         m_h3_MCParticle, true, 0);
  histoList->Add(h_eff_pt);

  TH1F* h_eff_theta = createHistogramsRatio("hefftheta", "efficiency VS #theta, normalized to MCParticles", m_h3_V0sPerMCParticle,
                                            m_h3_MCParticle, true, 1);
  histoList->Add(h_eff_theta);

  TH1F* h_eff_phi = createHistogramsRatio("heffphi", "efficiency VS #phi, normalized to MCParticles", m_h3_V0sPerMCParticle,
                                          m_h3_MCParticle, true, 2);
  histoList->Add(h_eff_phi);

}



void V0findingPerformanceEvaluationModule::addPurityPlots(TList* histoList)
{

//purity histograms
  TH1F* h_pur_pt = createHistogramsRatio("hpurpt", "purity VS pt", m_h3_MCParticlesPerV0, m_h3_V0s, true, 0);
  histoList->Add(h_pur_pt);

  TH1F* h_pur_theta = createHistogramsRatio("hpurtheta", "purity VS #theta", m_h3_MCParticlesPerV0, m_h3_V0s, true, 1);
  histoList->Add(h_pur_theta);

  TH1F* h_pur_phi = createHistogramsRatio("hpurphi", "purity VS #phi", m_h3_MCParticlesPerV0, m_h3_V0s, true, 2);
  histoList->Add(h_pur_phi);

}


bool V0findingPerformanceEvaluationModule::isV0(const MCParticle& the_mcParticle)
{

  bool isK_S0 = false;
  if (abs(the_mcParticle.getPDG()) == 310)
    isK_S0 = true;

  bool isK_L0 = false;
  if (abs(the_mcParticle.getPDG()) == 130)
    isK_L0 = true;

  bool isK_0 = false;
  if (abs(the_mcParticle.getPDG()) == 311)
    isK_0 = true;

  bool isLambda = false;
  if (abs(the_mcParticle.getPDG()) == 31122)
    isLambda = true;

  bool twoChargedProngs = false;

  if (the_mcParticle.getDaughters().size() == 2)
    twoChargedProngs = true;

  if (twoChargedProngs)
    if (the_mcParticle.getDaughters()[0]->getCharge() == 0)
      twoChargedProngs = false;

  return ((isK_S0 || isK_L0 || isK_0 || isLambda) && twoChargedProngs);

}
