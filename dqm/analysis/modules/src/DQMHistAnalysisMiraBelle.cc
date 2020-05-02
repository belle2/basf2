/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kindo Haruki, Luka Santelj                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <dqm/analysis/modules/DQMHistAnalysisMiraBelle.h>

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TClass.h>
#include <TROOT.h>

#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisMiraBelle);

DQMHistAnalysisMiraBelleModule::DQMHistAnalysisMiraBelleModule()
  : DQMHistAnalysisModule()
{
  // set module description (e.g. insert text)
  setDescription("Modify and analyze the data quality histograms of MiraBelle");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("debug", m_debug, "debug mode", false);
  addParam("alert", m_enableAlert, "Enable color alert", true);
}

DQMHistAnalysisMiraBelleModule::~DQMHistAnalysisMiraBelleModule()
{
}

void DQMHistAnalysisMiraBelleModule::initialize()
{
  gROOT->cd();

  //mon_mumu = new Belle2::MonitoringObject("mumu");
  mon_mumu = getMonitoringObject("mumu");

  // make cavases to be added to MonitoringObject
  main = new TCanvas("main", "main", 0, 0, 800, 600);
  resolution = new TCanvas("resolution", "resolution", 0, 0, 800, 600);
  muon_val = new TCanvas("muon_val", "muon_val", 0, 0, 400, 400);

  // add canvases to MonitoringObject
  mon_mumu->addCanvas(main);
  mon_mumu->addCanvas(resolution);
  mon_mumu->addCanvas(muon_val);

  B2DEBUG(20, "DQMHistAnalysisMiraBelle: initialized.");
}

void DQMHistAnalysisMiraBelleModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisMiraBelle: beginRun called.");
}

void DQMHistAnalysisMiraBelleModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisOutputMiraBelle: event called.");
}

void DQMHistAnalysisMiraBelleModule::endRun()
{
  // get existing histograms produced by DQM modules
  TH1* m_h_npxd = findHist("PhysicsObjectsMiraBelle/m_h_npxd");
  TH1* m_h_nsvd = findHist("PhysicsObjectsMiraBelle/m_h_nsvd");
  TH1* m_h_ncdc = findHist("PhysicsObjectsMiraBelle/m_h_ncdc");
  TH1* m_h_topdig = findHist("PhysicsObjectsMiraBelle/m_h_topdig");
  TH1* m_h_DetPhotonARICH = findHist("PhysicsObjectsMiraBelle/m_h_DetPhotonARICH");
  TH1* m_h_klmTotalHits = findHist("PhysicsObjectsMiraBelle/m_h_klmTotalHits");
  TH1* m_h_klmClusterLayers = findHist("PhysicsObjectsMiraBelle/m_h_klmClusterLayers");
  TH1* m_h_dD0 = findHist("PhysicsObjectsMiraBelle/m_h_dD0");
  TH1* m_h_dZ0 = findHist("PhysicsObjectsMiraBelle/m_h_dZ0");
  TH1* m_h_dPtcms = findHist("PhysicsObjectsMiraBelle/m_h_dPtcms");
  TH1* m_h_Pval = findHist("PhysicsObjectsMiraBelle/m_h_Pval");
  TH1* m_h_nExtraCDCHits = findHist("PhysicsObjectsMiraBelle/m_h_nExtraCDCHits");
  TH1* m_h_nECLClusters = findHist("PhysicsObjectsMiraBelle/m_h_nECLClusters");
  TH1* m_h_muid = findHist("PhysicsObjectsMiraBelle/m_h_muid");
  TH1* m_h_inv_p = findHist("PhysicsObjectsMiraBelle/m_h_inv_p");
  TH1* m_h_ndf = findHist("PhysicsObjectsMiraBelle/m_h_ndf");
  TH1* m_h_D0 = findHist("PhysicsObjectsMiraBelle/m_h_D0");
  TH1* m_h_Z0 = findHist("PhysicsObjectsMiraBelle/m_h_Z0");
  TH1* m_h_theta = findHist("PhysicsObjectsMiraBelle/m_h_theta");
  TH1* m_h_Phi0 = findHist("PhysicsObjectsMiraBelle/m_h_Phi0");
  TH1* m_h_Pt = findHist("PhysicsObjectsMiraBelle/m_h_Pt");
  TH1* m_h_Mom = findHist("PhysicsObjectsMiraBelle/m_h_Mom");
  TH1* m_h_klmTotalBarrelHits = findHist("PhysicsObjectsMiraBelle/m_h_klmTotalBarrelHits");
  TH1* m_h_klmTotalEndcapHits = findHist("PhysicsObjectsMiraBelle/m_h_klmTotalEndcapHits");
  TH1* m_h_dPhicms = findHist("PhysicsObjectsMiraBelle/m_h_dPhicms");

  // Make TCanvases
  // --- Main
  main->Divide(4, 3);
  main->cd(1);  m_h_theta->Draw();
  main->cd(2);  m_h_Phi0->Draw();
  main->cd(3);  m_h_Mom->Draw();
  main->cd(4);  m_h_Pt->Draw();
  main->cd(5);  m_h_npxd->Draw();
  main->cd(6);  m_h_nsvd->Draw();
  main->cd(7);  m_h_ncdc->Draw();
  main->cd(8);  m_h_topdig->Draw();
  main->cd(9);  m_h_DetPhotonARICH->Draw();
  main->cd(10);  m_h_klmClusterLayers->Draw();
  main->cd(11);  m_h_nExtraCDCHits->Draw();
  main->cd(12);  m_h_nECLClusters->Draw();
  // --- Resolution
  resolution->Divide(3, 3);
  resolution->cd(1);  m_h_inv_p->Draw();
  resolution->cd(2);  m_h_dD0->Draw();
  resolution->cd(3);  m_h_dZ0->Draw();
  resolution->cd(4);  m_h_dPtcms->Draw();
  resolution->cd(5);  m_h_dPhicms->Draw();
  resolution->cd(6);  m_h_nECLClusters->Draw();
  resolution->cd(7);  m_h_nExtraCDCHits->Draw();
  resolution->cd(8);  m_h_ndf->Draw();
  muon_val->Divide(2, 2);
  // --- Muon variables
  muon_val->cd(1);  m_h_muid->Draw();
  muon_val->cd(2);  m_h_Pval->Draw();
  muon_val->cd(3);  m_h_theta->Draw();
  muon_val->cd(4);  m_h_Phi0->Draw();

  // calculate the values of monitoring variables
  float mean_npxd = m_h_npxd->GetMean();
  float mean_nsvd = m_h_nsvd->GetMean();
  float mean_ncdc = m_h_ncdc->GetMean();
  float mean_nklmlayer = m_h_klmClusterLayers->GetMean();
  float mean_nklm = m_h_klmTotalHits->GetMean();
  float mean_nbklm = m_h_klmTotalBarrelHits->GetMean();
  float mean_neklm = m_h_klmTotalEndcapHits->GetMean();
  float mean_topdig = m_h_topdig->GetMean();
  float mean_parich = m_h_DetPhotonARICH->GetMean();
  float mean_ncdc_ex = m_h_nExtraCDCHits->GetMean();
  float mean_necl = m_h_nECLClusters->GetMean();
  float mean_muid = m_h_muid->GetMean();
  float mean_d0 = m_h_D0->GetMean();
  float mean_z0 = m_h_Z0->GetMean();
  float mean_pval = m_h_Pval->GetMean();
  float mean_ndf = m_h_ndf->GetMean();
  float dif_ndf_ncdc = mean_ndf - mean_ncdc;
  float mean_dd0 = m_h_dD0->GetMean();
  float mean_dz0 = m_h_dZ0->GetMean();
  float mean_dpt = m_h_dPtcms->GetMean();
  float rms_dd0 = m_h_dD0->GetRMS();
  float rms_dz0 = m_h_dZ0->GetRMS();
  float rms_dpt = m_h_dPtcms->GetRMS();
  int ntot = m_h_nsvd->GetEntries();
  float nevt_o_lum = ntot;
  float goodmu_frac = m_h_muid->GetBinContent(20) / (float)ntot;
  float goodmu_o_badmu = m_h_muid->GetBinContent(20) / (float)m_h_muid->GetBinContent(1);
  float pval_more95 = 0;
  float pval_less05 = 0;
  for (int i = 95; i < 100; i++) pval_more95 += m_h_Pval->GetBinContent(i + 1);
  for (int i = 0; i < 5; i++) pval_less05 += m_h_Pval->GetBinContent(i + 1);
  float pval_frac_0 = pval_less05 / (float)ntot;
  float pval_frac_1 = pval_more95 / (float)ntot;
  float nocdc_frac = m_h_ncdc->GetBinContent(1) / (float)ntot;
  float notop_frac = m_h_topdig->GetBinContent(1) / (float)ntot;
  float noarich_frac = m_h_DetPhotonARICH->GetBinContent(1) / (float)ntot;

  // set values
  mon_mumu->setVariable("mean_npxd", mean_npxd);
  mon_mumu->setVariable("mean_nsvd", mean_nsvd);
  mon_mumu->setVariable("mean_ncdc", mean_ncdc);
  mon_mumu->setVariable("mean_nklmlayer", mean_nklmlayer);
  mon_mumu->setVariable("mean_nklm", mean_nklm);
  mon_mumu->setVariable("mean_nbklm", mean_nbklm);
  mon_mumu->setVariable("mean_neklm", mean_neklm);
  mon_mumu->setVariable("mean_topdig", mean_topdig);
  mon_mumu->setVariable("mean_parich", mean_parich);
  mon_mumu->setVariable("mean_ncdc_ex", mean_ncdc_ex);
  mon_mumu->setVariable("mean_necl", mean_necl);
  mon_mumu->setVariable("mean_muid", mean_muid);
  mon_mumu->setVariable("mean_d0", mean_d0);
  mon_mumu->setVariable("mean_z0", mean_z0);
  mon_mumu->setVariable("mean_pval", mean_pval);
  mon_mumu->setVariable("mean_ndf", mean_ndf);
  mon_mumu->setVariable("dif_ndf_ncdc", dif_ndf_ncdc);
  mon_mumu->setVariable("mean_dd0", mean_dd0);
  mon_mumu->setVariable("mean_dz0", mean_dz0);
  mon_mumu->setVariable("mean_dpt", mean_dpt);
  mon_mumu->setVariable("rms_dd0", rms_dd0);
  mon_mumu->setVariable("rms_dz0", rms_dz0);
  mon_mumu->setVariable("rms_dpt", rms_dpt);
  mon_mumu->setVariable("nevt_o_lum", nevt_o_lum);
  mon_mumu->setVariable("goodmu_frac", goodmu_frac);
  mon_mumu->setVariable("goodmu_o_badmu", goodmu_o_badmu);
  mon_mumu->setVariable("pval_frac_0", pval_frac_0);
  mon_mumu->setVariable("pval_frac_1", pval_frac_1);
  mon_mumu->setVariable("nocdc_frac", nocdc_frac);
  mon_mumu->setVariable("notop_frac", notop_frac);
  mon_mumu->setVariable("noarich_frac", noarich_frac);

  B2DEBUG(20, "DQMHistAnalysisMiraBelle : endRun called");
}

void DQMHistAnalysisMiraBelleModule::terminate()
{

  B2DEBUG(20, "terminate called");
}

TCanvas* DQMHistAnalysisMiraBelleModule::find_canvas(TString canvas_name)
{
  TIter nextckey(gROOT->GetListOfCanvases());
  TObject* cobj = NULL;

  while ((cobj = (TObject*)nextckey())) {
    if (cobj->IsA()->InheritsFrom("TCanvas")) {
      if (cobj->GetName() == canvas_name)
        break;
    }
  }
  return (TCanvas*)cobj;
}