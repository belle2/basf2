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
  mon_bhabha->addCanvas(hadron_main);

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
  auto* hist_npxd = findHist("PhysicsObjectsMiraBelle/hist_npxd");
  auto* hist_nsvd = findHist("PhysicsObjectsMiraBelle/hist_nsvd");
  auto* hist_ncdc = findHist("PhysicsObjectsMiraBelle/hist_ncdc");
  auto* hist_topdig = findHist("PhysicsObjectsMiraBelle/hist_topdig");
  auto* hist_DetPhotonARICH = findHist("PhysicsObjectsMiraBelle/hist_DetPhotonARICH");
  auto* hist_klmTotalHits = findHist("PhysicsObjectsMiraBelle/hist_klmTotalHits");
  auto* hist_klmClusterLayers = findHist("PhysicsObjectsMiraBelle/hist_klmClusterLayers");
  auto* hist_dD0 = findHist("PhysicsObjectsMiraBelle/hist_dD0");
  auto* hist_dZ0 = findHist("PhysicsObjectsMiraBelle/hist_dZ0");
  auto* hist_dPtcms = findHist("PhysicsObjectsMiraBelle/hist_dPtcms");
  auto* hist_Pval = findHist("PhysicsObjectsMiraBelle/hist_Pval");
  auto* hist_nExtraCDCHits = findHist("PhysicsObjectsMiraBelle/hist_nExtraCDCHits");
  auto* hist_nECLClusters = findHist("PhysicsObjectsMiraBelle/hist_nECLClusters");
  auto* hist_muid = findHist("PhysicsObjectsMiraBelle/hist_muid");
  auto* hist_inv_p = findHist("PhysicsObjectsMiraBelle/hist_inv_p");
  auto* hist_ndf = findHist("PhysicsObjectsMiraBelle/hist_ndf");
  auto* hist_D0 = findHist("PhysicsObjectsMiraBelle/hist_D0");
  auto* hist_Z0 = findHist("PhysicsObjectsMiraBelle/hist_Z0");
  auto* hist_theta = findHist("PhysicsObjectsMiraBelle/hist_theta");
  auto* hist_Phi0 = findHist("PhysicsObjectsMiraBelle/hist_Phi0");
  auto* hist_Pt = findHist("PhysicsObjectsMiraBelle/hist_Pt");
  auto* hist_Mom = findHist("PhysicsObjectsMiraBelle/hist_Mom");
  auto* hist_klmTotalBarrelHits = findHist("PhysicsObjectsMiraBelle/hist_klmTotalBarrelHits");
  auto* hist_klmTotalEndcapHits = findHist("PhysicsObjectsMiraBelle/hist_klmTotalEndcapHits");
  auto* hist_dPhicms = findHist("PhysicsObjectsMiraBelle/hist_dPhicms");

  if (hist_npxd == nullptr) {
    B2ERROR("Can not find the hist_npxd histogram!");
    return;
  }
  if (hist_nsvd == nullptr) {
    B2ERROR("Can not find the hist_nsvd histogram!");
    return;
  }
  if (hist_ncdc == nullptr) {
    B2ERROR("Can not find the hist_ncdc histogram!");
    return;
  }
  if (hist_topdig == nullptr) {
    B2ERROR("Can not find the hist_topdig histogram!");
    return;
  }
  if (hist_DetPhotonARICH == nullptr) {
    B2ERROR("Can not find the hist_DetPhotonARICH histogram!");
    return;
  }
  if (hist_klmTotalHits == nullptr) {
    B2ERROR("Can not find the hist_klmTotalHits histogram!");
    return;
  }
  if (hist_klmClusterLayers == nullptr) {
    B2ERROR("Can not find the hist_klmClusterLayers histogram!");
    return;
  }
  if (hist_dD0 == nullptr) {
    B2ERROR("Can not find the hist_dD0 histogram!");
    return;
  }
  if (hist_dZ0 == nullptr) {
    B2ERROR("Can not find the hist_dZ0 histogram!");
    return;
  }
  if (hist_dPtcms == nullptr) {
    B2ERROR("Can not find the hist_dPtcms histogram!");
    return;
  }
  if (hist_Pval == nullptr) {
    B2ERROR("Can not find the hist_Pval histogram!");
    return;
  }
  if (hist_nExtraCDCHits == nullptr) {
    B2ERROR("Can not find the hist_nExtraCDCHits histogram!");
    return;
  }
  if (hist_nECLClusters == nullptr) {
    B2ERROR("Can not find the hist_nECLClusters histogram!");
    return;
  }
  if (hist_muid == nullptr) {
    B2ERROR("Can not find the hist_muid histogram!");
    return;
  }
  if (hist_inv_p == nullptr) {
    B2ERROR("Can not find the hist_inv_p histogram!");
    return;
  }
  if (hist_ndf == nullptr) {
    B2ERROR("Can not find the hist_ndf histogram!");
    return;
  }
  if (hist_D0 == nullptr) {
    B2ERROR("Can not find the hist_D0 histogram!");
    return;
  }
  if (hist_Z0 == nullptr) {
    B2ERROR("Can not find the hist_Z0 histogram!");
    return;
  }
  if (hist_theta == nullptr) {
    B2ERROR("Can not find the hist_theta histogram!");
    return;
  }
  if (hist_Phi0 == nullptr) {
    B2ERROR("Can not find the hist_Phi0 histogram!");
    return;
  }
  if (hist_Pt == nullptr) {
    B2ERROR("Can not find the hist_Pt histogram!");
    return;
  }
  if (hist_Mom == nullptr) {
    B2ERROR("Can not find the hist_Mom histogram!");
    return;
  }
  if (hist_klmTotalBarrelHits == nullptr) {
    B2ERROR("Can not find the hist_klmTotalBarrelHits histogram!");
    return;
  }
  if (hist_klmTotalEndcapHits == nullptr) {
    B2ERROR("Can not find the hist_klmTotalEndcapHits histogram!");
    return;
  }
  if (hist_dPhicms == nullptr) {
    B2ERROR("Can not find the hist_dPhicms histogram!");
    return;
  }

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
  double mean_npxd = hist_npxd->GetMean();
  double mean_nsvd = hist_nsvd->GetMean();
  double mean_ncdc = hist_ncdc->GetMean();
  double mean_nklmlayer = hist_klmClusterLayers->GetMean();
  double mean_nklm = hist_klmTotalHits->GetMean();
  double mean_nbklm = hist_klmTotalBarrelHits->GetMean();
  double mean_neklm = hist_klmTotalEndcapHits->GetMean();
  double mean_topdig = hist_topdig->GetMean();
  double mean_parich = hist_DetPhotonARICH->GetMean();
  double mean_ncdc_ex = hist_nExtraCDCHits->GetMean();
  double mean_necl = hist_nECLClusters->GetMean();
  double mean_muid = hist_muid->GetMean();
  double mean_d0 = hist_D0->GetMean();
  double mean_z0 = hist_Z0->GetMean();
  double mean_pval = hist_Pval->GetMean();
  double mean_ndf = hist_ndf->GetMean();
  double dif_ndf_ncdc = mean_ndf - mean_ncdc;
  double mean_dd0 = hist_dD0->GetMean();
  double mean_dz0 = hist_dZ0->GetMean();
  double mean_dpt = hist_dPtcms->GetMean();
  double rms_dd0 = hist_dD0->GetRMS();
  double rms_dz0 = hist_dZ0->GetRMS();
  double rms_dpt = hist_dPtcms->GetRMS();
  double sigma68_dd0 = getSigma68(hist_dD0);
  double sigma68_dz0 = getSigma68(hist_dZ0);
  double sigma68_dpt = getSigma68(hist_dPtcms);
  int ntot = hist_nsvd->GetEntries();
  double neve_mumu = ntot;
  double goodmu_frac = -1.;
  double pval_frac_0 = -1.;
  double pval_frac_1 = -1.;
  double nocdc_frac = -1.;
  double notop_frac = -1.;
  double noarich_frac = -1.;
  double muidcontent = hist_muid->GetBinContent(1);
  double goodmu_o_badmu = -1.;
  if (muidcontent != 0) {
    goodmu_o_badmu = hist_muid->GetBinContent(20) / muidcontent;
  } else {
    goodmu_o_badmu = 0.0;
  }
  double pval_more95 = 0.0;
  double pval_less05 = 0.0;
  for (int i = 95; i < 100; i++) pval_more95 += hist_Pval->GetBinContent(i + 1);
  for (int i = 0; i < 5; i++) pval_less05 += hist_Pval->GetBinContent(i + 1);
  if (ntot != 0) {
    goodmu_frac = hist_muid->GetBinContent(20) / neve_mumu;
    pval_frac_0 = pval_less05 / neve_mumu;
    pval_frac_1 = pval_more95 / neve_mumu;
    nocdc_frac = hist_ncdc->GetBinContent(1) / neve_mumu;
    notop_frac = hist_topdig->GetBinContent(1) / neve_mumu;
    noarich_frac = hist_DetPhotonARICH->GetBinContent(1) / neve_mumu;
  } else {
    goodmu_frac = 0.0;
    pval_frac_0 = 0.0;
    pval_frac_1 = 0.0;
    nocdc_frac = 0.0;
    notop_frac = 0.0;
    noarich_frac = 0.0;
  }
  //Calculate M(mumu)
  double peak_mumu = hist_inv_p->GetXaxis()->GetBinCenter(hist_inv_p->GetMaximumBin());
  TF1* f_mumuInvM = new TF1("f_mumuInvM", "gaus", peak_mumu - 0.05, peak_mumu + 0.05);
  f_mumuInvM->SetParameters(hist_inv_p->GetMaximum(), peak_mumu, 0.045);
  f_mumuInvM->SetParLimits(1, peak_mumu - 0.05, peak_mumu + 0.05);
  f_mumuInvM->SetParLimits(2, 0.01, 0.08);
  hist_inv_p->Fit(f_mumuInvM, "R");
  double fit_mumumass = f_mumuInvM->GetParameter(1);
  if (fit_mumumass < 9.) fit_mumumass = 9.;
  if (fit_mumumass > 12.) fit_mumumass = 12.;
  double fit_mumumass_error = f_mumuInvM->GetParError(1);
  double mumumass_reference = 10.568;
  double pull_mumumass = (fit_mumumass - mumumass_reference) / fit_mumumass_error;
  double fit_sigma_mumu = f_mumuInvM->GetParameter(2);

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
  mon_mumu->setVariable("fit_mumumass_error", fit_mumumass_error);
  mon_mumu->setVariable("pull_mumumass", pull_mumumass);
  mon_mumu->setVariable("sigma_mumumass", fit_sigma_mumu);
  mon_mumu->setVariable("fit_mumumass_ref", mumumass_reference);

  // ========== D*
  // get existing histograms produced by DQM modules
  auto* hist_D0_InvM = findHist("PhysicsObjectsMiraBelleDst/hist_D0_InvM");
  auto* hist_delta_m = findHist("PhysicsObjectsMiraBelleDst/hist_delta_m");
  auto* hist_D0_softpi_PID_ALL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_ALL_pion");
  auto* hist_D0_softpi_PID_SVD_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_SVD_pion");
  auto* hist_D0_softpi_PID_CDC_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_CDC_pion");
  auto* hist_D0_softpi_PID_TOP_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_TOP_pion");
  auto* hist_D0_softpi_PID_ARICH_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_ARICH_pion");
  auto* hist_D0_softpi_PID_ECL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_ECL_pion");
  auto* hist_D0_softpi_PID_KLM_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_softpi_PID_KLM_pion");
  auto* hist_D0_pi_PID_ALL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_ALL_pion");
  auto* hist_D0_pi_PID_SVD_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_SVD_pion");
  auto* hist_D0_pi_PID_CDC_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_CDC_pion");
  auto* hist_D0_pi_PID_TOP_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_TOP_pion");
  auto* hist_D0_pi_PID_ARICH_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_ARICH_pion");
  auto* hist_D0_pi_PID_ECL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_ECL_pion");
  auto* hist_D0_pi_PID_KLM_pion = findHist("PhysicsObjectsMiraBelleDst/hist_D0_pi_PID_KLM_pion");
  auto* hist_D0_K_PID_ALL_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_ALL_kaon");
  auto* hist_D0_K_PID_SVD_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_SVD_kaon");
  auto* hist_D0_K_PID_CDC_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_CDC_kaon");
  auto* hist_D0_K_PID_TOP_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_TOP_kaon");
  auto* hist_D0_K_PID_ARICH_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_ARICH_kaon");
  auto* hist_D0_K_PID_ECL_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_ECL_kaon");
  auto* hist_D0_K_PID_KLM_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_D0_K_PID_KLM_kaon");
  auto* hist_sideband_D0_softpi_PID_ALL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_ALL_pion");
  auto* hist_sideband_D0_softpi_PID_SVD_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_SVD_pion");
  auto* hist_sideband_D0_softpi_PID_CDC_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_CDC_pion");
  auto* hist_sideband_D0_softpi_PID_TOP_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_TOP_pion");
  auto* hist_sideband_D0_softpi_PID_ARICH_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_ARICH_pion");
  auto* hist_sideband_D0_softpi_PID_ECL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_ECL_pion");
  auto* hist_sideband_D0_softpi_PID_KLM_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_softpi_PID_KLM_pion");
  auto* hist_sideband_D0_pi_PID_ALL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_ALL_pion");
  auto* hist_sideband_D0_pi_PID_SVD_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_SVD_pion");
  auto* hist_sideband_D0_pi_PID_CDC_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_CDC_pion");
  auto* hist_sideband_D0_pi_PID_TOP_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_TOP_pion");
  auto* hist_sideband_D0_pi_PID_ARICH_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_ARICH_pion");
  auto* hist_sideband_D0_pi_PID_ECL_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_ECL_pion");
  auto* hist_sideband_D0_pi_PID_KLM_pion = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_pi_PID_KLM_pion");
  auto* hist_sideband_D0_K_PID_ALL_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_ALL_kaon");
  auto* hist_sideband_D0_K_PID_SVD_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_SVD_kaon");
  auto* hist_sideband_D0_K_PID_CDC_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_CDC_kaon");
  auto* hist_sideband_D0_K_PID_TOP_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_TOP_kaon");
  auto* hist_sideband_D0_K_PID_ARICH_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_ARICH_kaon");
  auto* hist_sideband_D0_K_PID_ECL_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_ECL_kaon");
  auto* hist_sideband_D0_K_PID_KLM_kaon = findHist("PhysicsObjectsMiraBelleDst/hist_sideband_D0_K_PID_KLM_kaon");
  auto* hist_D0_pi0_InvM = findHist("PhysicsObjectsMiraBelleDst2/hist_D0_pi0_InvM");
  if (hist_D0_InvM == nullptr) {
    B2ERROR("Can not find the hist_D0_InvM histogram!");
    return;
  }
  if (hist_delta_m == nullptr) {
    B2ERROR("Can not find the hist_delta_m histogram!");
    return;
  }
  if (hist_D0_softpi_PID_ALL_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_softpi_PID_ALL_pion histogram!");
    return;
  }
  if (hist_D0_softpi_PID_SVD_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_softpi_PID_SVD_pion histogram!");
    return;
  }
  if (hist_D0_softpi_PID_CDC_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_softpi_PID_CDC_pion histogram!");
    return;
  }
  if (hist_D0_softpi_PID_TOP_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_softpi_PID_TOP_pion histogram!");
    return;
  }
  if (hist_D0_softpi_PID_ARICH_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_softpi_PID_ARICH_pion histogram!");
    return;
  }
  if (hist_D0_softpi_PID_ECL_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_softpi_PID_ECL_pion histogram!");
    return;
  }
  if (hist_D0_softpi_PID_KLM_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_softpi_PID_KLM_pion histogram!");
    return;
  }
  if (hist_D0_pi_PID_ALL_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_pi_PID_ALL_pion histogram!");
    return;
  }
  if (hist_D0_pi_PID_SVD_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_pi_PID_SVD_pion histogram!");
    return;
  }
  if (hist_D0_pi_PID_CDC_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_pi_PID_CDC_pion histogram!");
    return;
  }
  if (hist_D0_pi_PID_TOP_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_pi_PID_TOP_pion histogram!");
    return;
  }
  if (hist_D0_pi_PID_ARICH_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_pi_PID_ARICH_pion histogram!");
    return;
  }
  if (hist_D0_pi_PID_ECL_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_pi_PID_ECL_pion histogram!");
    return;
  }
  if (hist_D0_pi_PID_KLM_pion == nullptr) {
    B2ERROR("Can not find the hist_D0_pi_PID_KLM_pion histogram!");
    return;
  }
  if (hist_D0_K_PID_ALL_kaon == nullptr) {
    B2ERROR("Can not find the hist_D0_K_PID_ALL_kaon histogram!");
    return;
  }
  if (hist_D0_K_PID_SVD_kaon == nullptr) {
    B2ERROR("Can not find the hist_D0_K_PID_SVD_kaon histogram!");
    return;
  }
  if (hist_D0_K_PID_CDC_kaon == nullptr) {
    B2ERROR("Can not find the hist_D0_K_PID_CDC_kaon histogram!");
    return;
  }
  if (hist_D0_K_PID_TOP_kaon == nullptr) {
    B2ERROR("Can not find the hist_D0_K_PID_TOP_kaon histogram!");
    return;
  }
  if (hist_D0_K_PID_ARICH_kaon == nullptr) {
    B2ERROR("Can not find the hist_D0_K_PID_ARICH_kaon histogram!");
    return;
  }
  if (hist_D0_K_PID_ECL_kaon == nullptr) {
    B2ERROR("Can not find the hist_D0_K_PID_ECL_kaon histogram!");
    return;
  }
  if (hist_D0_K_PID_KLM_kaon == nullptr) {
    B2ERROR("Can not find the hist_D0_K_PID_KLM_kaon histogram!");
    return;
  }
  if (hist_sideband_D0_softpi_PID_ALL_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_softpi_PID_ALL_pion histogram!");
    return;
  }
  if (hist_sideband_D0_softpi_PID_SVD_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_softpi_PID_SVD_pion histogram!");
    return;
  }
  if (hist_sideband_D0_softpi_PID_CDC_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_softpi_PID_CDC_pion histogram!");
    return;
  }
  if (hist_sideband_D0_softpi_PID_TOP_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_softpi_PID_TOP_pion histogram!");
    return;
  }
  if (hist_sideband_D0_softpi_PID_ARICH_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_softpi_PID_ARICH_pion histogram!");
    return;
  }
  if (hist_sideband_D0_softpi_PID_ECL_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_softpi_PID_ECL_pion histogram!");
    return;
  }
  if (hist_sideband_D0_softpi_PID_KLM_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_softpi_PID_KLM_pion histogram!");
    return;
  }
  if (hist_sideband_D0_pi_PID_ALL_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_pi_PID_ALL_pion histogram!");
    return;
  }
  if (hist_sideband_D0_pi_PID_SVD_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_pi_PID_SVD_pion histogram!");
    return;
  }
  if (hist_sideband_D0_pi_PID_CDC_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_pi_PID_CDC_pion histogram!");
    return;
  }
  if (hist_sideband_D0_pi_PID_TOP_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_pi_PID_TOP_pion histogram!");
    return;
  }
  if (hist_sideband_D0_pi_PID_ARICH_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_pi_PID_ARICH_pion histogram!");
    return;
  }
  if (hist_sideband_D0_pi_PID_ECL_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_pi_PID_ECL_pion histogram!");
    return;
  }
  if (hist_sideband_D0_pi_PID_KLM_pion == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_pi_PID_KLM_pion histogram!");
    return;
  }
  if (hist_sideband_D0_K_PID_ALL_kaon == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_K_PID_ALL_kaon histogram!");
    return;
  }
  if (hist_sideband_D0_K_PID_SVD_kaon == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_K_PID_SVD_kaon histogram!");
    return;
  }
  if (hist_sideband_D0_K_PID_CDC_kaon == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_K_PID_CDC_kaon histogram!");
    return;
  }
  if (hist_sideband_D0_K_PID_TOP_kaon == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_K_PID_TOP_kaon histogram!");
    return;
  }
  if (hist_sideband_D0_K_PID_ARICH_kaon == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_K_PID_ARICH_kaon histogram!");
    return;
  }
  if (hist_sideband_D0_K_PID_ECL_kaon == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_K_PID_ECL_kaon histogram!");
    return;
  }
  if (hist_sideband_D0_K_PID_KLM_kaon == nullptr) {
    B2ERROR("Can not find the hist_sideband_D0_K_PID_KLM_kaon histogram!");
    return;
  }
  if (hist_D0_pi0_InvM == nullptr) {
    B2ERROR("Can not find the hist_D0_pi0_InvM histogram!");
    return;
  }
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
  double InvM_bin_width = hist_D0_InvM->GetXaxis()->GetBinWidth(1);
  double neve_dst = f_gaus_InvM->Integral(1.81, 1.95) / InvM_bin_width;

  TF1* f_gaus_pi0_InvM = new TF1("f_gaus_pi0_InvM", "gaus", 0.09, 0.17);
  f_gaus_pi0_InvM->SetParameters(f_pi0_InvM->GetParameter(0), f_pi0_InvM->GetParameter(1), f_pi0_InvM->GetParameter(2));
  double pi0_InvM_bin_width = hist_D0_pi0_InvM->GetXaxis()->GetBinWidth(1);
  double neve_pi0 = f_gaus_pi0_InvM->Integral(0.09, 0.17) / pi0_InvM_bin_width;

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
  double mean_D0_InvM = f_InvM->GetParameter(1);
  double width_D0_InvM = f_InvM->GetParameter(2);
  double mean_delta_m = f_delta_m->GetParameter(1);
  double width_delta_m = f_delta_m->GetParameter(2);
  double mean_pi0_InvM = f_pi0_InvM->GetParameter(1);
  double width_pi0_InvM = f_pi0_InvM->GetParameter(2);
  double mean_D0_softpi_PID_ALL_pion = hist_D0_softpi_PID_ALL_pion->GetMean();
  double mean_D0_softpi_PID_SVD_pion = hist_D0_softpi_PID_SVD_pion->GetMean();
  double mean_D0_softpi_PID_CDC_pion = hist_D0_softpi_PID_CDC_pion->GetMean();
  double mean_D0_softpi_PID_TOP_pion = hist_D0_softpi_PID_TOP_pion->GetMean();
  double mean_D0_softpi_PID_ARICH_pion = hist_D0_softpi_PID_ARICH_pion->GetMean();
  double mean_D0_softpi_PID_ECL_pion = hist_D0_softpi_PID_ECL_pion->GetMean();
  double mean_D0_softpi_PID_KLM_pion = hist_D0_softpi_PID_KLM_pion->GetMean();
  double mean_D0_pi_PID_ALL_pion = hist_D0_pi_PID_ALL_pion->GetMean();
  double mean_D0_pi_PID_SVD_pion = hist_D0_pi_PID_SVD_pion->GetMean();
  double mean_D0_pi_PID_CDC_pion = hist_D0_pi_PID_CDC_pion->GetMean();
  double mean_D0_pi_PID_TOP_pion = hist_D0_pi_PID_TOP_pion->GetMean();
  double mean_D0_pi_PID_ARICH_pion = hist_D0_pi_PID_ARICH_pion->GetMean();
  double mean_D0_pi_PID_ECL_pion = hist_D0_pi_PID_ECL_pion->GetMean();
  double mean_D0_pi_PID_KLM_pion = hist_D0_pi_PID_KLM_pion->GetMean();
  double mean_D0_K_PID_ALL_kaon = hist_D0_K_PID_ALL_kaon->GetMean();
  double mean_D0_K_PID_SVD_kaon = hist_D0_K_PID_SVD_kaon->GetMean();
  double mean_D0_K_PID_CDC_kaon = hist_D0_K_PID_CDC_kaon->GetMean();
  double mean_D0_K_PID_TOP_kaon = hist_D0_K_PID_TOP_kaon->GetMean();
  double mean_D0_K_PID_ARICH_kaon = hist_D0_K_PID_ARICH_kaon->GetMean();
  double mean_D0_K_PID_ECL_kaon = hist_D0_K_PID_ECL_kaon->GetMean();
  double mean_D0_K_PID_KLM_kaon = hist_D0_K_PID_KLM_kaon->GetMean();

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
  auto* histbh_npxd = findHist("PhysicsObjectsMiraBelleBhabha/hist_npxd");
  auto* histbh_nsvd = findHist("PhysicsObjectsMiraBelleBhabha/hist_nsvd");
  auto* histbh_ncdc = findHist("PhysicsObjectsMiraBelleBhabha/hist_ncdc");
  auto* histbh_topdig = findHist("PhysicsObjectsMiraBelleBhabha/hist_topdig");
  auto* histbh_DetPhotonARICH = findHist("PhysicsObjectsMiraBelleBhabha/hist_DetPhotonARICH");
  auto* histbh_dD0 = findHist("PhysicsObjectsMiraBelleBhabha/hist_dD0");
  auto* histbh_dZ0 = findHist("PhysicsObjectsMiraBelleBhabha/hist_dZ0");
  auto* histbh_dPtcms = findHist("PhysicsObjectsMiraBelleBhabha/hist_dPtcms");
  auto* histbh_Pval = findHist("PhysicsObjectsMiraBelleBhabha/hist_Pval");
  auto* histbh_nExtraCDCHits = findHist("PhysicsObjectsMiraBelleBhabha/hist_nExtraCDCHits");
  auto* histbh_nECLClusters = findHist("PhysicsObjectsMiraBelleBhabha/hist_nECLClusters");
  auto* histbh_electronid = findHist("PhysicsObjectsMiraBelleBhabha/hist_electronid");
  auto* histbh_inv_p = findHist("PhysicsObjectsMiraBelleBhabha/hist_inv_p");
  auto* histbh_ndf = findHist("PhysicsObjectsMiraBelleBhabha/hist_ndf");
  auto* histbh_D0 = findHist("PhysicsObjectsMiraBelleBhabha/hist_D0");
  auto* histbh_Z0 = findHist("PhysicsObjectsMiraBelleBhabha/hist_Z0");
  auto* histbh_theta = findHist("PhysicsObjectsMiraBelleBhabha/hist_theta");
  auto* histbh_Phi0 = findHist("PhysicsObjectsMiraBelleBhabha/hist_Phi0");
  auto* histbh_Pt = findHist("PhysicsObjectsMiraBelleBhabha/hist_Pt");
  auto* histbh_Mom = findHist("PhysicsObjectsMiraBelleBhabha/hist_Mom");
  auto* histbh_dPhicms = findHist("PhysicsObjectsMiraBelleBhabha/hist_dPhicms");

  if (histbh_npxd == nullptr) {
    B2ERROR("Can not find the histbh_npxd histogram!");
    return;
  }
  if (histbh_nsvd == nullptr) {
    B2ERROR("Can not find the histbh_nsvd histogram!");
    return;
  }
  if (histbh_ncdc == nullptr) {
    B2ERROR("Can not find the histbh_ncdc histogram!");
    return;
  }
  if (histbh_topdig == nullptr) {
    B2ERROR("Can not find the histbh_topdig histogram!");
    return;
  }
  if (histbh_DetPhotonARICH == nullptr) {
    B2ERROR("Can not find the histbh_DetPhotonARICH histogram!");
    return;
  }
  if (histbh_dD0 == nullptr) {
    B2ERROR("Can not find the histbh_dD0 histogram!");
    return;
  }
  if (histbh_dZ0 == nullptr) {
    B2ERROR("Can not find the histbh_dZ0 histogram!");
    return;
  }
  if (histbh_dPtcms == nullptr) {
    B2ERROR("Can not find the histbh_dPtcms histogram!");
    return;
  }
  if (histbh_Pval == nullptr) {
    B2ERROR("Can not find the histbh_Pval histogram!");
    return;
  }
  if (histbh_nExtraCDCHits == nullptr) {
    B2ERROR("Can not find the histbh_nExtraCDCHits histogram!");
    return;
  }
  if (histbh_nECLClusters == nullptr) {
    B2ERROR("Can not find the histbh_nECLClusters histogram!");
    return;
  }
  if (histbh_electronid == nullptr) {
    B2ERROR("Can not find the histbh_electronid histogram!");
    return;
  }
  if (histbh_inv_p == nullptr) {
    B2ERROR("Can not find the histbh_inv_p histogram!");
    return;
  }
  if (histbh_ndf == nullptr) {
    B2ERROR("Can not find the histbh_ndf histogram!");
    return;
  }
  if (histbh_D0 == nullptr) {
    B2ERROR("Can not find the histbh_D0 histogram!");
    return;
  }
  if (histbh_Z0 == nullptr) {
    B2ERROR("Can not find the histbh_Z0 histogram!");
    return;
  }
  if (histbh_theta == nullptr) {
    B2ERROR("Can not find the histbh_theta histogram!");
    return;
  }
  if (histbh_Phi0 == nullptr) {
    B2ERROR("Can not find the histbh_Phi0 histogram!");
    return;
  }
  if (histbh_Pt == nullptr) {
    B2ERROR("Can not find the histbh_Pt histogram!");
    return;
  }
  if (histbh_Mom == nullptr) {
    B2ERROR("Can not find the histbh_Mom histogram!");
    return;
  }
  if (histbh_dPhicms == nullptr) {
    B2ERROR("Can not find the histbh_dPhicms histogram!");
    return;
  }

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
  double bh_mean_npxd = histbh_npxd->GetMean();
  double bh_mean_nsvd = histbh_nsvd->GetMean();
  double bh_mean_ncdc = histbh_ncdc->GetMean();
  double bh_mean_topdig = histbh_topdig->GetMean();
  double bh_mean_parich = histbh_DetPhotonARICH->GetMean();
  double bh_mean_ncdc_ex = histbh_nExtraCDCHits->GetMean();
  double bh_mean_necl = histbh_nECLClusters->GetMean();
  double bh_mean_electronid = histbh_electronid->GetMean();
  double bh_mean_d0 = histbh_D0->GetMean();
  double bh_mean_z0 = histbh_Z0->GetMean();
  double bh_mean_pval = histbh_Pval->GetMean();
  double bh_mean_ndf = histbh_ndf->GetMean();
  double bh_dif_ndf_ncdc = mean_ndf - mean_ncdc;
  double bh_mean_dd0 = histbh_dD0->GetMean();
  double bh_mean_dz0 = histbh_dZ0->GetMean();
  double bh_mean_dpt = histbh_dPtcms->GetMean();
  double bh_rms_dd0 = histbh_dD0->GetRMS();
  double bh_rms_dz0 = histbh_dZ0->GetRMS();
  double bh_rms_dpt = histbh_dPtcms->GetRMS();
  double bh_sigma68_dd0 = getSigma68(histbh_dD0);
  double bh_sigma68_dz0 = getSigma68(histbh_dZ0);
  double bh_sigma68_dpt = getSigma68(histbh_dPtcms);
  int bh_ntot = histbh_nECLClusters->GetEntries();
  double bh_neve_bhabha = bh_ntot;
  int bh_ntot_sign = histbh_nsvd->GetEntries();
  double bh_neve_bhabha_sign = bh_ntot_sign;
  double bh_goode_frac = -1.;
  double bh_pval_frac_0 = -1.;
  double bh_pval_frac_1 = -1.;
  double bh_nocdc_frac = -1.;
  double bh_notop_frac = -1.;
  double bh_noarich_frac = -1.;
  double eidcontent = histbh_electronid->GetBinContent(1);
  double bh_goode_o_bade;
  if (eidcontent != 0) {
    bh_goode_o_bade = histbh_electronid->GetBinContent(20) / eidcontent;
  } else {
    bh_goode_o_bade = 0.0;
  }
  double bh_pval_more95 = 0.0;
  double bh_pval_less05 = 0.0;
  for (int i = 95; i < 100; i++) bh_pval_more95 += histbh_Pval->GetBinContent(i + 1);
  for (int i = 0; i < 5; i++) bh_pval_less05 += histbh_Pval->GetBinContent(i + 1);
  if (bh_neve_bhabha_sign != 0) {
    bh_goode_frac = histbh_electronid->GetBinContent(20) / bh_neve_bhabha_sign;
    bh_pval_frac_0 = bh_pval_less05 / bh_neve_bhabha_sign;
    bh_pval_frac_1 = bh_pval_more95 / bh_neve_bhabha_sign;
    bh_nocdc_frac = histbh_ncdc->GetBinContent(1) / bh_neve_bhabha_sign;
    bh_notop_frac = histbh_topdig->GetBinContent(1) / bh_neve_bhabha_sign;
    bh_noarich_frac = histbh_DetPhotonARICH->GetBinContent(1) / bh_neve_bhabha_sign;
  } else {
    bh_goode_frac = 0.0;
    bh_pval_frac_0 = 0.0;
    bh_pval_frac_1 = 0.0;
    bh_nocdc_frac = 0.0;
    bh_notop_frac = 0.0;
    bh_noarich_frac = 0.0;
  }
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
  mon_bhabha->setVariable("bh_neve_bhabha_sign", bh_neve_bhabha_sign);
  mon_bhabha->setVariable("bh_goode_frac", bh_goode_frac);
  mon_bhabha->setVariable("bh_goode_o_bade", bh_goode_o_bade);
  mon_bhabha->setVariable("bh_pval_frac_0", bh_pval_frac_0);
  mon_bhabha->setVariable("bh_pval_frac_1", bh_pval_frac_1);
  mon_bhabha->setVariable("bh_nocdc_frac", bh_nocdc_frac);
  mon_bhabha->setVariable("bh_notop_frac", bh_notop_frac);
  mon_bhabha->setVariable("bh_noarich_frac", bh_noarich_frac);
  // ========== hadronb2 + tight
  // get existing histograms produced by DQM modules
  auto* histhad_nECLClusters = findHist("PhysicsObjectsMiraBelleHadron/hist_nECLClusters");
  auto* histhad_visibleEnergyCMSnorm = findHist("PhysicsObjectsMiraBelleHadron/hist_visibleEnergyCMSnorm");
  auto* histhad_EsumCMSnorm = findHist("PhysicsObjectsMiraBelleHadron/hist_EsumCMSnorm");
  auto* histhad_R2 = findHist("PhysicsObjectsMiraBelleHadron/hist_R2");
  auto* histhad_physicsresultsH = findHist("PhysicsObjectsMiraBelleHadron/hist_physicsresultsH");

  if (histhad_nECLClusters == nullptr) {
    B2ERROR("Can not find the histhad_nECLClusters histogram!");
    return;
  }
  if (histhad_visibleEnergyCMSnorm == nullptr) {
    B2ERROR("Can not find the histhad_visibleEnergyCMSnorm histogram!");
    return;
  }
  if (histhad_EsumCMSnorm == nullptr) {
    B2ERROR("Can not find the histhad_EsumCMSnorm histogram!");
    return;
  }
  if (histhad_R2 == nullptr) {
    B2ERROR("Can not find the histhad_R2 histogram!");
    return;
  }
  if (histhad_physicsresultsH == nullptr) {
    B2ERROR("Can not find the histhad_physicsresultsH histogram!");
    return;
  }

  // Make TCanvases
  // --- hadron_Main
  hadron_main->Divide(2, 2);
  hadron_main->cd(1);  histhad_nECLClusters->Draw();
  hadron_main->cd(2);  histhad_visibleEnergyCMSnorm->Draw();
  hadron_main->cd(3);  histhad_EsumCMSnorm->Draw();
  hadron_main->cd(4);  histhad_R2->Draw();
  // calculate the values of monitoring variables
  double had_ntot = histhad_physicsresultsH->GetBinContent(3);
  double ratio_hadron_bhabha = 0.;
  //pull
  double ratio_pull_hadBhabha = -10.;
  double error_ratio = -10.;
  double ratio_reference = 0.193;
  if (bh_ntot != 0) {
    ratio_hadron_bhabha = had_ntot / bh_neve_bhabha;
    //pull
    error_ratio = ratio_hadron_bhabha * sqrt((1 / had_ntot) + (1 / bh_neve_bhabha));
    ratio_pull_hadBhabha = (ratio_hadron_bhabha - ratio_reference) / error_ratio;
  }
  // set values
  mon_bhabha->setVariable("had_ntot", had_ntot);
  mon_hadron->setVariable("ratio_hadron_bhabha", ratio_hadron_bhabha);
  mon_hadron->setVariable("error_ratio", error_ratio);
  mon_hadron->setVariable("reference_ratio", ratio_reference);
  mon_hadron->setVariable("ratio_pull_hadBhabha", ratio_pull_hadBhabha);

  B2DEBUG(20, "DQMHistAnalysisMiraBelle : endRun called");
}

void DQMHistAnalysisMiraBelleModule::terminate()
{

  B2DEBUG(20, "terminate called");
}
