/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <dqm/analysis/modules/DQMHistAnalysisMiraBelle.h>

#include <TROOT.h>
#include <TF1.h>

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
  addParam("scale_dst", m_scale_dst, "Scale factor signal/sideband", 0.09375);
}

DQMHistAnalysisMiraBelleModule::~DQMHistAnalysisMiraBelleModule()
{
}

void DQMHistAnalysisMiraBelleModule::initialize()
{
  gROOT->cd();

  //mon_mumu = new Belle2::MonitoringObject("mumu");
  mon_mumu = getMonitoringObject("mumu");
  mon_dst = getMonitoringObject("dst");
  //bhabha,hadrons
  mon_bhabha = getMonitoringObject("bhabha");
  mon_hadron = getMonitoringObject("hadronb2");

  // make cavases to be added to MonitoringObject
  mumu_main = new TCanvas("mumu_main", "mumu_main", 0, 0, 800, 600);
  mumu_resolution = new TCanvas("mumu_resolution", "mumu_resolution", 0, 0, 800, 600);
  mumu_muon_val = new TCanvas("mumu_muon_val", "mumu_muon_val", 0, 0, 400, 400);
  dst_mass = new TCanvas("dst_mass", "dst_mass", 0, 0, 1200, 400);
  dst_pi_val = new TCanvas("dst_pi_val", "dst_pi_val", 0, 0, 800, 400);
  dst_k_val = new TCanvas("dst_k_val", "dst_k_val", 0, 0, 800, 400);
  //bhabha,hadrons
  bhabha_main = new TCanvas("bhabha_main", "bhabha_main", 0, 0, 800, 600);
  bhabha_resolution = new TCanvas("bhabha_resolution", "bhabha_resolution", 0, 0, 800, 600);
  hadron_main = new TCanvas("hadron_main", "hadron_main", 0, 0, 800, 600);

  // add canvases to MonitoringObject
  mon_mumu->addCanvas(mumu_main);
  mon_mumu->addCanvas(mumu_resolution);
  mon_mumu->addCanvas(mumu_muon_val);
  mon_dst->addCanvas(dst_mass);
  mon_dst->addCanvas(dst_pi_val);
  mon_dst->addCanvas(dst_k_val);
  //bhabha,hadrons
  mon_bhabha->addCanvas(bhabha_main);
  mon_bhabha->addCanvas(bhabha_resolution);
  mon_hadron->addCanvas(hadron_main);

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
  // ========== mumutight
  // get existing histograms produced by DQM modules
  TH1* hist_npxd = findHist("PhysicsObjectsMiraBelle/hist_npxd");
  TH1* hist_nsvd = findHist("PhysicsObjectsMiraBelle/hist_nsvd");
  TH1* hist_ncdc = findHist("PhysicsObjectsMiraBelle/hist_ncdc");
  TH1* hist_topdig = findHist("PhysicsObjectsMiraBelle/hist_topdig");
  TH1* hist_DetPhotonARICH = findHist("PhysicsObjectsMiraBelle/hist_DetPhotonARICH");
  TH1* hist_klmTotalHits = findHist("PhysicsObjectsMiraBelle/hist_klmTotalHits");
  TH1* hist_klmClusterLayers = findHist("PhysicsObjectsMiraBelle/hist_klmClusterLayers");
  TH1* hist_dD0 = findHist("PhysicsObjectsMiraBelle/hist_dD0");
  TH1* hist_dZ0 = findHist("PhysicsObjectsMiraBelle/hist_dZ0");
  TH1* hist_dPtcms = findHist("PhysicsObjectsMiraBelle/hist_dPtcms");
  TH1* hist_Pval = findHist("PhysicsObjectsMiraBelle/hist_Pval");
  TH1* hist_nExtraCDCHits = findHist("PhysicsObjectsMiraBelle/hist_nExtraCDCHits");
  TH1* hist_nECLClusters = findHist("PhysicsObjectsMiraBelle/hist_nECLClusters");
  TH1* hist_muid = findHist("PhysicsObjectsMiraBelle/hist_muid");
  TH1* hist_inv_p = findHist("PhysicsObjectsMiraBelle/hist_inv_p");
  TH1* hist_ndf = findHist("PhysicsObjectsMiraBelle/hist_ndf");
  TH1* hist_D0 = findHist("PhysicsObjectsMiraBelle/hist_D0");
  TH1* hist_Z0 = findHist("PhysicsObjectsMiraBelle/hist_Z0");
  TH1* hist_theta = findHist("PhysicsObjectsMiraBelle/hist_theta");
  TH1* hist_Phi0 = findHist("PhysicsObjectsMiraBelle/hist_Phi0");
  TH1* hist_Pt = findHist("PhysicsObjectsMiraBelle/hist_Pt");
  TH1* hist_Mom = findHist("PhysicsObjectsMiraBelle/hist_Mom");
  TH1* hist_klmTotalBarrelHits = findHist("PhysicsObjectsMiraBelle/hist_klmTotalBarrelHits");
  TH1* hist_klmTotalEndcapHits = findHist("PhysicsObjectsMiraBelle/hist_klmTotalEndcapHits");
  TH1* hist_dPhicms = findHist("PhysicsObjectsMiraBelle/hist_dPhicms");

  // Make TCanvases
  // --- Mumu_Main
  mumu_main->Divide(4, 3);
  mumu_main->cd(1);  hist_theta->Draw();
  mumu_main->cd(2);  hist_Phi0->Draw();
  mumu_main->cd(3);  hist_Mom->Draw();
  mumu_main->cd(4);  hist_Pt->Draw();
  mumu_main->cd(5);  hist_npxd->Draw();
  mumu_main->cd(6);  hist_nsvd->Draw();
  mumu_main->cd(7);  hist_ncdc->Draw();
  mumu_main->cd(8);  hist_topdig->Draw();
  mumu_main->cd(9);  hist_DetPhotonARICH->Draw();
  mumu_main->cd(10);  hist_klmClusterLayers->Draw();
  mumu_main->cd(11);  hist_nExtraCDCHits->Draw();
  mumu_main->cd(12);  hist_nECLClusters->Draw();
  // --- Mumu_Resolution
  mumu_resolution->Divide(3, 3);
  mumu_resolution->cd(1);  hist_inv_p->Draw();
  mumu_resolution->cd(2);  hist_dD0->Draw();
  mumu_resolution->cd(3);  hist_dZ0->Draw();
  mumu_resolution->cd(4);  hist_dPtcms->Draw();
  mumu_resolution->cd(5);  hist_dPhicms->Draw();
  mumu_resolution->cd(6);  hist_nECLClusters->Draw();
  mumu_resolution->cd(7);  hist_nExtraCDCHits->Draw();
  mumu_resolution->cd(8);  hist_ndf->Draw();
  mumu_muon_val->Divide(2, 2);
  // --- Muon variables
  mumu_muon_val->cd(1);  hist_muid->Draw();
  mumu_muon_val->cd(2);  hist_Pval->Draw();
  mumu_muon_val->cd(3);  hist_theta->Draw();
  mumu_muon_val->cd(4);  hist_Phi0->Draw();

  // calculate the values of monitoring variables
  float mean_npxd = hist_npxd->GetMean();
  float mean_nsvd = hist_nsvd->GetMean();
  float mean_ncdc = hist_ncdc->GetMean();
  float mean_nklmlayer = hist_klmClusterLayers->GetMean();
  float mean_nklm = hist_klmTotalHits->GetMean();
  float mean_nbklm = hist_klmTotalBarrelHits->GetMean();
  float mean_neklm = hist_klmTotalEndcapHits->GetMean();
  float mean_topdig = hist_topdig->GetMean();
  float mean_parich = hist_DetPhotonARICH->GetMean();
  float mean_ncdc_ex = hist_nExtraCDCHits->GetMean();
  float mean_necl = hist_nECLClusters->GetMean();
  float mean_muid = hist_muid->GetMean();
  float mean_d0 = hist_D0->GetMean();
  float mean_z0 = hist_Z0->GetMean();
  float mean_pval = hist_Pval->GetMean();
  float mean_ndf = hist_ndf->GetMean();
  float dif_ndf_ncdc = mean_ndf - mean_ncdc;
  float mean_dd0 = hist_dD0->GetMean();
  float mean_dz0 = hist_dZ0->GetMean();
  float mean_dpt = hist_dPtcms->GetMean();
  float rms_dd0 = hist_dD0->GetRMS();
  float rms_dz0 = hist_dZ0->GetRMS();
  float rms_dpt = hist_dPtcms->GetRMS();
  float sigma68_dd0 = getSigma68(hist_dD0);
  float sigma68_dz0 = getSigma68(hist_dZ0);
  float sigma68_dpt = getSigma68(hist_dPtcms);
  int ntot = hist_nsvd->GetEntries();
  float neve_mumu = ntot;
  float goodmu_frac = hist_muid->GetBinContent(20) / (float)ntot;
  float goodmu_o_badmu = hist_muid->GetBinContent(20) / (float)hist_muid->GetBinContent(1);
  float pval_more95 = 0;
  float pval_less05 = 0;
  for (int i = 95; i < 100; i++) pval_more95 += hist_Pval->GetBinContent(i + 1);
  for (int i = 0; i < 5; i++) pval_less05 += hist_Pval->GetBinContent(i + 1);
  float pval_frac_0 = pval_less05 / (float)ntot;
  float pval_frac_1 = pval_more95 / (float)ntot;
  float nocdc_frac = hist_ncdc->GetBinContent(1) / (float)ntot;
  float notop_frac = hist_topdig->GetBinContent(1) / (float)ntot;
  float noarich_frac = hist_DetPhotonARICH->GetBinContent(1) / (float)ntot;
  //Calculate M(mumu)
  float peak_mumu = hist_inv_p->GetXaxis()->GetBinCenter(hist_inv_p->GetMaximumBin());
  TF1* f_mumuInvM = new TF1("f_mumuInvM", "gaus", peak_mumu - 0.04, peak_mumu + 0.04);
  f_mumuInvM->SetParameters(hist_inv_p->GetMaximum(), peak_mumu, 0.045);
  f_mumuInvM->SetParLimits(1, peak_mumu - 0.04, peak_mumu + 0.04);
  f_mumuInvM->SetParLimits(2, 0.01, 0.06);
  hist_inv_p->Fit(f_mumuInvM, "R");
  float fit_mumumass = f_mumuInvM->GetParameter(1);
  if (fit_mumumass < 9) fit_mumumass = 9;
  if (fit_mumumass > 12) fit_mumumass = 12;

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
  mon_mumu->setVariable("sigma68_dd0", sigma68_dd0);
  mon_mumu->setVariable("sigma68_dz0", sigma68_dz0);
  mon_mumu->setVariable("sigma68_dpt", sigma68_dpt);
  mon_mumu->setVariable("neve_mumu", neve_mumu);
  mon_mumu->setVariable("goodmu_frac", goodmu_frac);
  mon_mumu->setVariable("goodmu_o_badmu", goodmu_o_badmu);
  mon_mumu->setVariable("pval_frac_0", pval_frac_0);
  mon_mumu->setVariable("pval_frac_1", pval_frac_1);
  mon_mumu->setVariable("nocdc_frac", nocdc_frac);
  mon_mumu->setVariable("notop_frac", notop_frac);
  mon_mumu->setVariable("noarich_frac", noarich_frac);
  mon_mumu->setVariable("fit_mumumass", fit_mumumass);

  // ========== D*
  // get existing histograms produced by DQM modules
  TH1* hist_D0_InvM = findHist("PhysicsObjectsMiraBelleDst/hist_D0_InvM");
  TH1* hist_delta_m = findHist("PhysicsObjectsMiraBelleDst/hist_delta_m");
  TH1* hist_D0_softpi_PID_ALL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_ALL_pion");
  TH1* hist_D0_softpi_PID_SVD_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_SVD_pion");
  TH1* hist_D0_softpi_PID_CDC_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_CDC_pion");
  TH1* hist_D0_softpi_PID_TOP_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_TOP_pion");
  TH1* hist_D0_softpi_PID_ARICH_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_ARICH_pion");
  TH1* hist_D0_softpi_PID_ECL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_ECL_pion");
  TH1* hist_D0_softpi_PID_KLM_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_KLM_pion");
  TH1* hist_D0_pi_PID_ALL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_ALL_pion");
  TH1* hist_D0_pi_PID_SVD_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_SVD_pion");
  TH1* hist_D0_pi_PID_CDC_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_CDC_pion");
  TH1* hist_D0_pi_PID_TOP_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_TOP_pion");
  TH1* hist_D0_pi_PID_ARICH_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_ARICH_pion");
  TH1* hist_D0_pi_PID_ECL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_ECL_pion");
  TH1* hist_D0_pi_PID_KLM_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_KLM_pion");
  TH1* hist_D0_K_PID_ALL_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_ALL_kaon");
  TH1* hist_D0_K_PID_SVD_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_SVD_kaon");
  TH1* hist_D0_K_PID_CDC_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_CDC_kaon");
  TH1* hist_D0_K_PID_TOP_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_TOP_kaon");
  TH1* hist_D0_K_PID_ARICH_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_ARICH_kaon");
  TH1* hist_D0_K_PID_ECL_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_ECL_kaon");
  TH1* hist_D0_K_PID_KLM_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_KLM_kaon");
  TH1* hist_sideband_D0_softpi_PID_ALL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_ALL_pion");
  TH1* hist_sideband_D0_softpi_PID_SVD_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_SVD_pion");
  TH1* hist_sideband_D0_softpi_PID_CDC_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_CDC_pion");
  TH1* hist_sideband_D0_softpi_PID_TOP_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_TOP_pion");
  TH1* hist_sideband_D0_softpi_PID_ARICH_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_ARICH_pion");
  TH1* hist_sideband_D0_softpi_PID_ECL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_ECL_pion");
  TH1* hist_sideband_D0_softpi_PID_KLM_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_KLM_pion");
  TH1* hist_sideband_D0_pi_PID_ALL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_ALL_pion");
  TH1* hist_sideband_D0_pi_PID_SVD_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_SVD_pion");
  TH1* hist_sideband_D0_pi_PID_CDC_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_CDC_pion");
  TH1* hist_sideband_D0_pi_PID_TOP_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_TOP_pion");
  TH1* hist_sideband_D0_pi_PID_ARICH_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_ARICH_pion");
  TH1* hist_sideband_D0_pi_PID_ECL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_ECL_pion");
  TH1* hist_sideband_D0_pi_PID_KLM_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_KLM_pion");
  TH1* hist_sideband_D0_K_PID_ALL_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_ALL_kaon");
  TH1* hist_sideband_D0_K_PID_SVD_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_SVD_kaon");
  TH1* hist_sideband_D0_K_PID_CDC_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_CDC_kaon");
  TH1* hist_sideband_D0_K_PID_TOP_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_TOP_kaon");
  TH1* hist_sideband_D0_K_PID_ARICH_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_ARICH_kaon");
  TH1* hist_sideband_D0_K_PID_ECL_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_ECL_kaon");
  TH1* hist_sideband_D0_K_PID_KLM_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_KLM_kaon");
  TH1* hist_D0_pi0_InvM = findHist("PhysicsObjectsMiraBelleDst2/hist_D0_pi0_InvM");

  // Fit mass distributions for scale factor
  TF1* f_InvM = new TF1("f_InvM", "[0]*TMath::Gaus(x,[1],[2])+[3]*pow(x-[4],2)+[5]", 1.81, 1.95);
  f_InvM->SetParNames("Height", "#mu", "#sigma", "a", "b", "c");
  f_InvM->SetParameters(hist_D0_InvM->GetMaximum(), 1.86, 5e-3, 0., 1.86, 0.);
  hist_D0_InvM->Fit(f_InvM, "", "", 1.81, 1.95);
  f_InvM->SetLineColor(kRed);

  TF1* f_delta_m = new TF1("f_delta_m", "[0]*TMath::Gaus(x,[1],[2])+[3]*pow(x-[4],2)+[5]", 0.14, 0.16);
  f_delta_m->SetParNames("Height", "#mu", "#sigma", "a", "b", "c");
  f_delta_m->SetParameters(hist_delta_m->GetMaximum(), 0.145, 5e-4, 0., 0.145, 0.);
  hist_delta_m->Fit(f_delta_m, "", "", 0.14, 0.16);
  f_delta_m->SetLineColor(kRed);

  TF1* f_pi0_InvM = new TF1("f_pi0_InvM", "[0]*TMath::Gaus(x,[1],[2])+[3]*pow(x-[4],2)+[5]", 0.09, 0.17);
  f_pi0_InvM->SetParNames("Height", "#mu", "#sigma", "a", "b", "c");
  f_pi0_InvM->SetParameters(hist_D0_pi0_InvM->GetMaximum(), 0.13, 5e-3, 0., 0.13, 0.);
  hist_D0_pi0_InvM->Fit(f_pi0_InvM, "", "", 0.09, 0.17);
  f_pi0_InvM->SetLineColor(kRed);

  // D->KPi and D->KPiPi0 yields
  TF1* f_gaus_InvM = new TF1("f_gaus_InvM", "gaus", 1.81, 1.95);
  f_gaus_InvM->SetParameters(f_InvM->GetParameter(0), f_InvM->GetParameter(1), f_InvM->GetParameter(2));
  float InvM_bin_width = hist_D0_InvM->GetXaxis()->GetBinWidth(1);
  float neve_dst = f_gaus_InvM->Integral(1.81, 1.95) / InvM_bin_width;

  TF1* f_gaus_pi0_InvM = new TF1("f_gaus_pi0_InvM", "gaus", 0.09, 0.17);
  f_gaus_pi0_InvM->SetParameters(f_pi0_InvM->GetParameter(0), f_pi0_InvM->GetParameter(1), f_pi0_InvM->GetParameter(2));
  float pi0_InvM_bin_width = hist_D0_pi0_InvM->GetXaxis()->GetBinWidth(1);
  float neve_pi0 = f_gaus_pi0_InvM->Integral(0.09, 0.17) / pi0_InvM_bin_width;

  // Sumw2
  hist_D0_softpi_PID_ALL_pion->Sumw2();
  hist_D0_softpi_PID_SVD_pion->Sumw2();
  hist_D0_softpi_PID_CDC_pion->Sumw2();
  hist_D0_softpi_PID_TOP_pion->Sumw2();
  hist_D0_softpi_PID_ARICH_pion->Sumw2();
  hist_D0_softpi_PID_ECL_pion->Sumw2();
  hist_D0_softpi_PID_KLM_pion->Sumw2();
  hist_D0_pi_PID_ALL_pion->Sumw2();
  hist_D0_pi_PID_SVD_pion->Sumw2();
  hist_D0_pi_PID_CDC_pion->Sumw2();
  hist_D0_pi_PID_TOP_pion->Sumw2();
  hist_D0_pi_PID_ARICH_pion->Sumw2();
  hist_D0_pi_PID_ECL_pion->Sumw2();
  hist_D0_pi_PID_KLM_pion->Sumw2();
  hist_D0_K_PID_ALL_kaon->Sumw2();
  hist_D0_K_PID_SVD_kaon->Sumw2();
  hist_D0_K_PID_CDC_kaon->Sumw2();
  hist_D0_K_PID_TOP_kaon->Sumw2();
  hist_D0_K_PID_ARICH_kaon->Sumw2();
  hist_D0_K_PID_ECL_kaon->Sumw2();
  hist_D0_K_PID_KLM_kaon->Sumw2();
  hist_sideband_D0_softpi_PID_ALL_pion->Sumw2();
  hist_sideband_D0_softpi_PID_SVD_pion->Sumw2();
  hist_sideband_D0_softpi_PID_CDC_pion->Sumw2();
  hist_sideband_D0_softpi_PID_TOP_pion->Sumw2();
  hist_sideband_D0_softpi_PID_ARICH_pion->Sumw2();
  hist_sideband_D0_softpi_PID_ECL_pion->Sumw2();
  hist_sideband_D0_softpi_PID_KLM_pion->Sumw2();
  hist_sideband_D0_pi_PID_ALL_pion->Sumw2();
  hist_sideband_D0_pi_PID_SVD_pion->Sumw2();
  hist_sideband_D0_pi_PID_CDC_pion->Sumw2();
  hist_sideband_D0_pi_PID_TOP_pion->Sumw2();
  hist_sideband_D0_pi_PID_ARICH_pion->Sumw2();
  hist_sideband_D0_pi_PID_ECL_pion->Sumw2();
  hist_sideband_D0_pi_PID_KLM_pion->Sumw2();
  hist_sideband_D0_K_PID_ALL_kaon->Sumw2();
  hist_sideband_D0_K_PID_SVD_kaon->Sumw2();
  hist_sideband_D0_K_PID_CDC_kaon->Sumw2();
  hist_sideband_D0_K_PID_TOP_kaon->Sumw2();
  hist_sideband_D0_K_PID_ARICH_kaon->Sumw2();
  hist_sideband_D0_K_PID_ECL_kaon->Sumw2();
  hist_sideband_D0_K_PID_KLM_kaon->Sumw2();

  // Scale the distributions in sideband
  hist_sideband_D0_softpi_PID_ALL_pion->Scale(m_scale_dst);
  hist_sideband_D0_softpi_PID_SVD_pion->Scale(m_scale_dst);
  hist_sideband_D0_softpi_PID_CDC_pion->Scale(m_scale_dst);
  hist_sideband_D0_softpi_PID_TOP_pion->Scale(m_scale_dst);
  hist_sideband_D0_softpi_PID_ARICH_pion->Scale(m_scale_dst);
  hist_sideband_D0_softpi_PID_ECL_pion->Scale(m_scale_dst);
  hist_sideband_D0_softpi_PID_KLM_pion->Scale(m_scale_dst);
  hist_sideband_D0_pi_PID_ALL_pion->Scale(m_scale_dst);
  hist_sideband_D0_pi_PID_SVD_pion->Scale(m_scale_dst);
  hist_sideband_D0_pi_PID_CDC_pion->Scale(m_scale_dst);
  hist_sideband_D0_pi_PID_TOP_pion->Scale(m_scale_dst);
  hist_sideband_D0_pi_PID_ARICH_pion->Scale(m_scale_dst);
  hist_sideband_D0_pi_PID_ECL_pion->Scale(m_scale_dst);
  hist_sideband_D0_pi_PID_KLM_pion->Scale(m_scale_dst);
  hist_sideband_D0_K_PID_ALL_kaon->Scale(m_scale_dst);
  hist_sideband_D0_K_PID_SVD_kaon->Scale(m_scale_dst);
  hist_sideband_D0_K_PID_CDC_kaon->Scale(m_scale_dst);
  hist_sideband_D0_K_PID_TOP_kaon->Scale(m_scale_dst);
  hist_sideband_D0_K_PID_ARICH_kaon->Scale(m_scale_dst);
  hist_sideband_D0_K_PID_ECL_kaon->Scale(m_scale_dst);
  hist_sideband_D0_K_PID_KLM_kaon->Scale(m_scale_dst);

  // BG subtraction
  hist_D0_softpi_PID_ALL_pion->Add(hist_sideband_D0_softpi_PID_ALL_pion, -1);
  hist_D0_softpi_PID_SVD_pion->Add(hist_sideband_D0_softpi_PID_SVD_pion, -1);
  hist_D0_softpi_PID_CDC_pion->Add(hist_sideband_D0_softpi_PID_CDC_pion, -1);
  hist_D0_softpi_PID_TOP_pion->Add(hist_sideband_D0_softpi_PID_TOP_pion, -1);
  hist_D0_softpi_PID_ARICH_pion->Add(hist_sideband_D0_softpi_PID_ARICH_pion, -1);
  hist_D0_softpi_PID_ECL_pion->Add(hist_sideband_D0_softpi_PID_ECL_pion, -1);
  hist_D0_softpi_PID_KLM_pion->Add(hist_sideband_D0_softpi_PID_KLM_pion, -1);
  hist_D0_pi_PID_ALL_pion->Add(hist_sideband_D0_pi_PID_ALL_pion, -1);
  hist_D0_pi_PID_SVD_pion->Add(hist_sideband_D0_pi_PID_SVD_pion, -1);
  hist_D0_pi_PID_CDC_pion->Add(hist_sideband_D0_pi_PID_CDC_pion, -1);
  hist_D0_pi_PID_TOP_pion->Add(hist_sideband_D0_pi_PID_TOP_pion, -1);
  hist_D0_pi_PID_ARICH_pion->Add(hist_sideband_D0_pi_PID_ARICH_pion, -1);
  hist_D0_pi_PID_ECL_pion->Add(hist_sideband_D0_pi_PID_ECL_pion, -1);
  hist_D0_pi_PID_KLM_pion->Add(hist_sideband_D0_pi_PID_KLM_pion, -1);
  hist_D0_K_PID_ALL_kaon->Add(hist_sideband_D0_K_PID_ALL_kaon, -1);
  hist_D0_K_PID_SVD_kaon->Add(hist_sideband_D0_K_PID_SVD_kaon, -1);
  hist_D0_K_PID_CDC_kaon->Add(hist_sideband_D0_K_PID_CDC_kaon, -1);
  hist_D0_K_PID_TOP_kaon->Add(hist_sideband_D0_K_PID_TOP_kaon, -1);
  hist_D0_K_PID_ARICH_kaon->Add(hist_sideband_D0_K_PID_ARICH_kaon, -1);
  hist_D0_K_PID_ECL_kaon->Add(hist_sideband_D0_K_PID_ECL_kaon, -1);
  hist_D0_K_PID_KLM_kaon->Add(hist_sideband_D0_K_PID_KLM_kaon, -1);

  // Make TCanvases
  // --- D* -> D0pi mass resolution
  dst_mass->Divide(3, 1);
  dst_mass->cd(1);
  hist_D0_InvM->Draw();
  f_InvM->Draw("SAME");
  dst_mass->cd(2);
  hist_delta_m->Draw();
  f_delta_m->Draw("SAME");
  dst_mass->cd(3);
  hist_D0_pi0_InvM->Draw();
  f_pi0_InvM->Draw("SAME");
  // --- pi variables for D0 -> K pi
  dst_pi_val->Divide(4, 2);
  dst_pi_val->cd(1);  hist_D0_pi_PID_ALL_pion->Draw();
  dst_pi_val->cd(2);  hist_D0_pi_PID_SVD_pion->Draw();
  dst_pi_val->cd(3);  hist_D0_pi_PID_CDC_pion->Draw();
  dst_pi_val->cd(4);  hist_D0_pi_PID_TOP_pion->Draw();
  dst_pi_val->cd(5);  hist_D0_pi_PID_ARICH_pion->Draw();
  dst_pi_val->cd(6);  hist_D0_pi_PID_ECL_pion->Draw();
  dst_pi_val->cd(7);  hist_D0_pi_PID_KLM_pion->Draw();
  // --- K variables for D0 -> K pi
  dst_k_val->Divide(4, 2);
  dst_k_val->cd(1);  hist_D0_K_PID_ALL_kaon->Draw();
  dst_k_val->cd(2);  hist_D0_K_PID_SVD_kaon->Draw();
  dst_k_val->cd(3);  hist_D0_K_PID_CDC_kaon->Draw();
  dst_k_val->cd(4);  hist_D0_K_PID_TOP_kaon->Draw();
  dst_k_val->cd(5);  hist_D0_K_PID_ARICH_kaon->Draw();
  dst_k_val->cd(6);  hist_D0_K_PID_ECL_kaon->Draw();
  dst_k_val->cd(7);  hist_D0_K_PID_KLM_kaon->Draw();

  // calculate the values of monitoring variables
  float mean_D0_InvM = f_InvM->GetParameter(1);
  float width_D0_InvM = f_InvM->GetParameter(2);
  float mean_delta_m = f_delta_m->GetParameter(1);
  float width_delta_m = f_delta_m->GetParameter(2);
  float mean_pi0_InvM = f_pi0_InvM->GetParameter(1);
  float width_pi0_InvM = f_pi0_InvM->GetParameter(2);
  float mean_D0_softpi_PID_ALL_pion = hist_D0_softpi_PID_ALL_pion->GetMean();
  float mean_D0_softpi_PID_SVD_pion = hist_D0_softpi_PID_SVD_pion->GetMean();
  float mean_D0_softpi_PID_CDC_pion = hist_D0_softpi_PID_CDC_pion->GetMean();
  float mean_D0_softpi_PID_TOP_pion = hist_D0_softpi_PID_TOP_pion->GetMean();
  float mean_D0_softpi_PID_ARICH_pion = hist_D0_softpi_PID_ARICH_pion->GetMean();
  float mean_D0_softpi_PID_ECL_pion = hist_D0_softpi_PID_ECL_pion->GetMean();
  float mean_D0_softpi_PID_KLM_pion = hist_D0_softpi_PID_KLM_pion->GetMean();
  float mean_D0_pi_PID_ALL_pion = hist_D0_pi_PID_ALL_pion->GetMean();
  float mean_D0_pi_PID_SVD_pion = hist_D0_pi_PID_SVD_pion->GetMean();
  float mean_D0_pi_PID_CDC_pion = hist_D0_pi_PID_CDC_pion->GetMean();
  float mean_D0_pi_PID_TOP_pion = hist_D0_pi_PID_TOP_pion->GetMean();
  float mean_D0_pi_PID_ARICH_pion = hist_D0_pi_PID_ARICH_pion->GetMean();
  float mean_D0_pi_PID_ECL_pion = hist_D0_pi_PID_ECL_pion->GetMean();
  float mean_D0_pi_PID_KLM_pion = hist_D0_pi_PID_KLM_pion->GetMean();
  float mean_D0_K_PID_ALL_kaon = hist_D0_K_PID_ALL_kaon->GetMean();
  float mean_D0_K_PID_SVD_kaon = hist_D0_K_PID_SVD_kaon->GetMean();
  float mean_D0_K_PID_CDC_kaon = hist_D0_K_PID_CDC_kaon->GetMean();
  float mean_D0_K_PID_TOP_kaon = hist_D0_K_PID_TOP_kaon->GetMean();
  float mean_D0_K_PID_ARICH_kaon = hist_D0_K_PID_ARICH_kaon->GetMean();
  float mean_D0_K_PID_ECL_kaon = hist_D0_K_PID_ECL_kaon->GetMean();
  float mean_D0_K_PID_KLM_kaon = hist_D0_K_PID_KLM_kaon->GetMean();

  // set values
  mon_dst->setVariable("neve_dst", neve_dst);
  mon_dst->setVariable("neve_pi0", neve_pi0);
  mon_dst->setVariable("mean_D0_InvM", mean_D0_InvM);
  mon_dst->setVariable("width_D0_InvM", width_D0_InvM);
  mon_dst->setVariable("mean_delta_m", mean_delta_m);
  mon_dst->setVariable("width_delta_m", width_delta_m);
  mon_dst->setVariable("mean_pi0_InvM", mean_pi0_InvM);
  mon_dst->setVariable("width_pi0_InvM", width_pi0_InvM);
  mon_dst->setVariable("mean_D0_softpi_PID_ALL_pion", mean_D0_softpi_PID_ALL_pion);
  mon_dst->setVariable("mean_D0_softpi_PID_SVD_pion", mean_D0_softpi_PID_SVD_pion);
  mon_dst->setVariable("mean_D0_softpi_PID_CDC_pion", mean_D0_softpi_PID_CDC_pion);
  mon_dst->setVariable("mean_D0_softpi_PID_TOP_pion", mean_D0_softpi_PID_TOP_pion);
  mon_dst->setVariable("mean_D0_softpi_PID_ARICH_pion", mean_D0_softpi_PID_ARICH_pion);
  mon_dst->setVariable("mean_D0_softpi_PID_ECL_pion", mean_D0_softpi_PID_ECL_pion);
  mon_dst->setVariable("mean_D0_softpi_PID_KLM_pion", mean_D0_softpi_PID_KLM_pion);
  mon_dst->setVariable("mean_D0_pi_PID_ALL_pion", mean_D0_pi_PID_ALL_pion);
  mon_dst->setVariable("mean_D0_pi_PID_SVD_pion", mean_D0_pi_PID_SVD_pion);
  mon_dst->setVariable("mean_D0_pi_PID_CDC_pion", mean_D0_pi_PID_CDC_pion);
  mon_dst->setVariable("mean_D0_pi_PID_TOP_pion", mean_D0_pi_PID_TOP_pion);
  mon_dst->setVariable("mean_D0_pi_PID_ARICH_pion", mean_D0_pi_PID_ARICH_pion);
  mon_dst->setVariable("mean_D0_pi_PID_ECL_pion", mean_D0_pi_PID_ECL_pion);
  mon_dst->setVariable("mean_D0_pi_PID_KLM_pion", mean_D0_pi_PID_KLM_pion);
  mon_dst->setVariable("mean_D0_K_PID_ALL_kaon", mean_D0_K_PID_ALL_kaon);
  mon_dst->setVariable("mean_D0_K_PID_SVD_kaon", mean_D0_K_PID_SVD_kaon);
  mon_dst->setVariable("mean_D0_K_PID_CDC_kaon", mean_D0_K_PID_CDC_kaon);
  mon_dst->setVariable("mean_D0_K_PID_TOP_kaon", mean_D0_K_PID_TOP_kaon);
  mon_dst->setVariable("mean_D0_K_PID_ARICH_kaon", mean_D0_K_PID_ARICH_kaon);
  mon_dst->setVariable("mean_D0_K_PID_ECL_kaon", mean_D0_K_PID_ECL_kaon);
  mon_dst->setVariable("mean_D0_K_PID_KLM_kaon", mean_D0_K_PID_KLM_kaon);

  //bhabha,hadrons
  // ========== bhabha_all
  // get existing histograms produced by DQM modules
  TH1* histbh_npxd = findHist("PhysicsObjectsMiraBelleBhabha/hist_npxd");
  TH1* histbh_nsvd = findHist("PhysicsObjectsMiraBelleBhabha/hist_nsvd");
  TH1* histbh_ncdc = findHist("PhysicsObjectsMiraBelleBhabha/hist_ncdc");
  TH1* histbh_topdig = findHist("PhysicsObjectsMiraBelleBhabha/hist_topdig");
  TH1* histbh_DetPhotonARICH = findHist("PhysicsObjectsMiraBelleBhabha/hist_DetPhotonARICH");
  TH1* histbh_dD0 = findHist("PhysicsObjectsMiraBelleBhabha/hist_dD0");
  TH1* histbh_dZ0 = findHist("PhysicsObjectsMiraBelleBhabha/hist_dZ0");
  TH1* histbh_dPtcms = findHist("PhysicsObjectsMiraBelleBhabha/hist_dPtcms");
  TH1* histbh_Pval = findHist("PhysicsObjectsMiraBelleBhabha/hist_Pval");
  TH1* histbh_nExtraCDCHits = findHist("PhysicsObjectsMiraBelleBhabha/hist_nExtraCDCHits");
  TH1* histbh_nECLClusters = findHist("PhysicsObjectsMiraBelleBhabha/hist_nECLClusters");
  TH1* histbh_electronid = findHist("PhysicsObjectsMiraBelleBhabha/hist_electronid");
  TH1* histbh_inv_p = findHist("PhysicsObjectsMiraBelleBhabha/hist_inv_p");
  TH1* histbh_ndf = findHist("PhysicsObjectsMiraBelleBhabha/hist_ndf");
  TH1* histbh_D0 = findHist("PhysicsObjectsMiraBelleBhabha/hist_D0");
  TH1* histbh_Z0 = findHist("PhysicsObjectsMiraBelleBhabha/hist_Z0");
  TH1* histbh_theta = findHist("PhysicsObjectsMiraBelleBhabha/hist_theta");
  TH1* histbh_Phi0 = findHist("PhysicsObjectsMiraBelleBhabha/hist_Phi0");
  TH1* histbh_Pt = findHist("PhysicsObjectsMiraBelleBhabha/hist_Pt");
  TH1* histbh_Mom = findHist("PhysicsObjectsMiraBelleBhabha/hist_Mom");
  TH1* histbh_dPhicms = findHist("PhysicsObjectsMiraBelleBhabha/hist_dPhicms");

  // Make TCanvases
  // --- bhabha_Main
  bhabha_main->Divide(4, 3);
  bhabha_main->cd(1);  histbh_theta->Draw();
  bhabha_main->cd(2);  histbh_Phi0->Draw();
  bhabha_main->cd(3);  histbh_Mom->Draw();
  bhabha_main->cd(4);  histbh_Pt->Draw();
  bhabha_main->cd(5);  histbh_npxd->Draw();
  bhabha_main->cd(6);  histbh_nsvd->Draw();
  bhabha_main->cd(7);  histbh_ncdc->Draw();
  bhabha_main->cd(8);  histbh_topdig->Draw();
  bhabha_main->cd(9);  histbh_DetPhotonARICH->Draw();
  bhabha_main->cd(10);  histbh_nExtraCDCHits->Draw();
  bhabha_main->cd(11);  histbh_nECLClusters->Draw();
  // --- bhabha_Resolution
  bhabha_resolution->Divide(3, 3);
  bhabha_resolution->cd(1);  histbh_inv_p->Draw();
  bhabha_resolution->cd(2);  histbh_dD0->Draw();
  bhabha_resolution->cd(3);  histbh_dZ0->Draw();
  bhabha_resolution->cd(4);  histbh_dPtcms->Draw();
  bhabha_resolution->cd(5);  histbh_dPhicms->Draw();
  bhabha_resolution->cd(6);  histbh_nECLClusters->Draw();
  bhabha_resolution->cd(7);  histbh_nExtraCDCHits->Draw();
  bhabha_resolution->cd(8);  histbh_ndf->Draw();

  // calculate the values of monitoring variables
  float bh_mean_npxd = histbh_npxd->GetMean();
  float bh_mean_nsvd = histbh_nsvd->GetMean();
  float bh_mean_ncdc = histbh_ncdc->GetMean();
  float bh_mean_topdig = histbh_topdig->GetMean();
  float bh_mean_parich = histbh_DetPhotonARICH->GetMean();
  float bh_mean_ncdc_ex = histbh_nExtraCDCHits->GetMean();
  float bh_mean_necl = histbh_nECLClusters->GetMean();
  float bh_mean_electronid = histbh_electronid->GetMean();
  float bh_mean_d0 = histbh_D0->GetMean();
  float bh_mean_z0 = histbh_Z0->GetMean();
  float bh_mean_pval = histbh_Pval->GetMean();
  float bh_mean_ndf = histbh_ndf->GetMean();
  float bh_dif_ndf_ncdc = mean_ndf - mean_ncdc;
  float bh_mean_dd0 = histbh_dD0->GetMean();
  float bh_mean_dz0 = histbh_dZ0->GetMean();
  float bh_mean_dpt = histbh_dPtcms->GetMean();
  float bh_rms_dd0 = histbh_dD0->GetRMS();
  float bh_rms_dz0 = histbh_dZ0->GetRMS();
  float bh_rms_dpt = histbh_dPtcms->GetRMS();
  float bh_sigma68_dd0 = getSigma68(histbh_dD0);
  float bh_sigma68_dz0 = getSigma68(histbh_dZ0);
  float bh_sigma68_dpt = getSigma68(histbh_dPtcms);
  int bh_ntot = histbh_nsvd->GetEntries();
  float bh_neve_bhabha = bh_ntot;
  float bh_goode_frac = histbh_electronid->GetBinContent(20) / (float)bh_ntot;
  float bh_goode_o_bade = histbh_electronid->GetBinContent(20) / (float)histbh_electronid->GetBinContent(1);
  float bh_pval_more95 = 0;
  float bh_pval_less05 = 0;
  for (int i = 95; i < 100; i++) bh_pval_more95 += histbh_Pval->GetBinContent(i + 1);
  for (int i = 0; i < 5; i++) bh_pval_less05 += histbh_Pval->GetBinContent(i + 1);
  float bh_pval_frac_0 = bh_pval_less05 / (float)bh_ntot;
  float bh_pval_frac_1 = bh_pval_more95 / (float)bh_ntot;
  float bh_nocdc_frac = histbh_ncdc->GetBinContent(1) / (float)bh_ntot;
  float bh_notop_frac = histbh_topdig->GetBinContent(1) / (float)bh_ntot;
  float bh_noarich_frac = histbh_DetPhotonARICH->GetBinContent(1) / (float)bh_ntot;
  // set values
  mon_bhabha->setVariable("bh_mean_npxd", bh_mean_npxd);
  mon_bhabha->setVariable("bh_mean_nsvd", bh_mean_nsvd);
  mon_bhabha->setVariable("bh_mean_ncdc", bh_mean_ncdc);
  mon_bhabha->setVariable("bh_mean_topdig", bh_mean_topdig);
  mon_bhabha->setVariable("bh_mean_parich", bh_mean_parich);
  mon_bhabha->setVariable("bh_mean_ncdc_ex", bh_mean_ncdc_ex);
  mon_bhabha->setVariable("bh_mean_necl", bh_mean_necl);
  mon_bhabha->setVariable("bh_mean_electronid", bh_mean_electronid);
  mon_bhabha->setVariable("bh_mean_d0", bh_mean_d0);
  mon_bhabha->setVariable("bh_mean_z0", bh_mean_z0);
  mon_bhabha->setVariable("bh_mean_pval", bh_mean_pval);
  mon_bhabha->setVariable("bh_mean_ndf", bh_mean_ndf);
  mon_bhabha->setVariable("bh_dif_ndf_ncdc", bh_dif_ndf_ncdc);
  mon_bhabha->setVariable("bh_mean_dd0", bh_mean_dd0);
  mon_bhabha->setVariable("bh_mean_dz0", bh_mean_dz0);
  mon_bhabha->setVariable("bh_mean_dpt", bh_mean_dpt);
  mon_bhabha->setVariable("bh_rms_dd0", bh_rms_dd0);
  mon_bhabha->setVariable("bh_rms_dz0", bh_rms_dz0);
  mon_bhabha->setVariable("bh_rms_dpt", bh_rms_dpt);
  mon_bhabha->setVariable("bh_sigma68_dd0", bh_sigma68_dd0);
  mon_bhabha->setVariable("bh_sigma68_dz0", bh_sigma68_dz0);
  mon_bhabha->setVariable("bh_sigma68_dpt", bh_sigma68_dpt);
  mon_bhabha->setVariable("bh_neve_bhabha", bh_neve_bhabha);
  mon_bhabha->setVariable("bh_goodmu_frac", bh_goode_frac);
  mon_bhabha->setVariable("bh_goodmu_o_badmu", bh_goode_o_bade);
  mon_bhabha->setVariable("bh_pval_frac_0", bh_pval_frac_0);
  mon_bhabha->setVariable("bh_pval_frac_1", bh_pval_frac_1);
  mon_bhabha->setVariable("bh_nocdc_frac", bh_nocdc_frac);
  mon_bhabha->setVariable("bh_notop_frac", bh_notop_frac);
  mon_bhabha->setVariable("bh_noarich_frac", bh_noarich_frac);
  // ========== hadronb2 + tight
  // get existing histograms produced by DQM modules
  TH1* histhad_nECLClusters = findHist("PhysicsObjectsMiraBelleBhabha/hist_nECLClusters");
  TH1* histhad_visibleEnergyCMSnorm = findHist("PhysicsObjectsMiraBelleBhabha/hist_visibleEnergyCMSnorm");
  TH1* histhad_EsumCMSnorm = findHist("PhysicsObjectsMiraBelleBhabha/hist_EsumCMSnorm");
  TH1* histhad_R2 = findHist("PhysicsObjectsMiraBelleBhabha/hist_R2");
  // Make TCanvases
  // --- hadron_Main
  hadron_main->Divide(2, 2);
  hadron_main->cd(1);  histhad_nECLClusters->Draw();
  hadron_main->cd(2);  histhad_visibleEnergyCMSnorm->Draw();
  hadron_main->cd(3);  histhad_EsumCMSnorm->Draw();
  hadron_main->cd(4);  histhad_R2->Draw();
  // calculate the values of monitoring variables
  float had_ntot = histhad_R2->GetEntries();

  float ratio_hadron_bhabha = -1.;
  if (bh_ntot != 0) {
    ratio_hadron_bhabha = bh_pval_less05 / (float)bh_ntot;
  } else {
    ratio_hadron_bhabha = 0;
  }
  // set values
  mon_bhabha->setVariable("had_ntot", had_ntot);
  mon_hadron->setVariable("ratio_hadron_bhabha", ratio_hadron_bhabha);

  B2DEBUG(20, "DQMHistAnalysisMiraBelle : endRun called");
}

void DQMHistAnalysisMiraBelleModule::terminate()
{

  B2DEBUG(20, "terminate called");
}
