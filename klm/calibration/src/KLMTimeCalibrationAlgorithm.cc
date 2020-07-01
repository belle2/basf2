/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/calibration/KLMTimeCalibrationAlgorithm.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <framework/core/Environment.h>

/* ROOT headers. */
#include <TFile.h>
#include <TFitResult.h>
#include <TString.h>
#include <TTree.h>
#include <TVector3.h>
#include <Math/MinimizerOptions.h>

using namespace Belle2;
using namespace ROOT::Math;

KLMTimeCalibrationAlgorithm::KLMTimeCalibrationAlgorithm() :
  CalibrationAlgorithm("KLMTimeCalibrationCollector")
{
  m_debug = false;
  m_useEventT0 = true;
  m_lower_limit_counts = 50;

  m_mc = Environment::Instance().isMC();
  m_elementNum = &(KLMElementNumbers::Instance());
  m_minimizerOptions = ROOT::Math::MinimizerOptions();

  m_time_channelAvg_rpc = 0.0;
  m_etime_channelAvg_rpc = 0.0;
  m_time_channelAvg_scint = 0.0;
  m_etime_channelAvg_scint = 0.0;
  m_time_channelAvg_scint_end = 0.0;
  m_etime_channelAvg_scint_end = 0.0;

  h_calibrated = nullptr;
  h_diff = nullptr;
  gre_time_channel_rpc = nullptr;
  gre_time_channel_scint = nullptr;
  gre_time_channel_scint_end = nullptr;

  hprf_rpc_phi_effC = nullptr;
  hprf_rpc_z_effC = nullptr;
  hprf_scint_phi_effC = nullptr;
  hprf_scint_z_effC = nullptr;
  hprf_scint_plane1_effC_end = nullptr;
  hprf_scint_plane2_effC_end = nullptr;
  h_time_rpc_tc = nullptr;
  h_time_scint_tc = nullptr;
  h_time_scint_tc_end = nullptr;

  h_time_rpc = nullptr;
  h_time_scint = nullptr;
  h_time_scint_end = nullptr;
  hc_time_rpc = nullptr;
  hc_time_scint = nullptr;
  hc_time_scint_end = nullptr;

  for (int ia = 0; ia < 2; ++ia) {
    h_timeF_rpc[ia] = nullptr;
    h_timeF_scint[ia] = nullptr;
    h_timeF_scint_end[ia] = nullptr;
    hc_timeF_rpc[ia] = nullptr;
    hc_timeF_scint[ia] = nullptr;
    hc_timeF_scint_end[ia] = nullptr;
    h2_timeF_rpc[ia] = nullptr;
    h2_timeF_scint[ia] = nullptr;
    h2_timeF_scint_end[ia] = nullptr;
    h2c_timeF_rpc[ia] = nullptr;
    h2c_timeF_scint[ia] = nullptr;
    h2c_timeF_scint_end[ia] = nullptr;
    for (int ib = 0; ib < 8; ++ib) {
      h_timeFS_rpc[ia][ib] = nullptr;
      h_timeFS_scint[ia][ib] = nullptr;
      hc_timeFS_rpc[ia][ib] = nullptr;
      hc_timeFS_scint[ia][ib] = nullptr;
      h2_timeFS[ia][ib] = nullptr;
      h2c_timeFS[ia][ib] = nullptr;
      for (int ic = 0; ic < 15; ++ic) {
        h_timeFSL[ia][ib][ic] = nullptr;
        hc_timeFSL[ia][ib][ic] = nullptr;
        for (int id = 0; id < 2; ++id) {
          h_timeFSLP[ia][ib][ic][id] = nullptr;
          hc_timeFSLP[ia][ib][ic][id] = nullptr;
          h2_timeFSLP[ia][ib][ic][id] = nullptr;
          h2c_timeFSLP[ia][ib][ic][id] = nullptr;
          for (int ie = 0; ie < 54; ++ie) {
            h_timeFSLPC_tc[ia][ib][ic][id][ie] = nullptr;
            h_timeFSLPC[ia][ib][ic][id][ie] = nullptr;
            hc_timeFSLPC[ia][ib][ic][id][ie] = nullptr;
          }
        }
      }
    }
    for (int ib = 0; ib < 4; ++ib) {
      h_timeFS_scint_end[ia][ib] = nullptr;
      hc_timeFS_scint_end[ia][ib] = nullptr;
      h2_timeFS_end[ia][ib] = nullptr;
      h2c_timeFS_end[ia][ib] = nullptr;
      for (int ic = 0; ic < 14; ++ic) {
        h_timeFSL_end[ia][ib][ic] = nullptr;
        hc_timeFSL_end[ia][ib][ic] = nullptr;
        for (int id = 0; id < 2; ++id) {
          h_timeFSLP_end[ia][ib][ic][id] = nullptr;
          hc_timeFSLP_end[ia][ib][ic][id] = nullptr;
          h2_timeFSLP_end[ia][ib][ic][id] = nullptr;
          h2c_timeFSLP_end[ia][ib][ic][id] = nullptr;
          for (int ie = 0; ie < 75; ++ie) {
            h_timeFSLPC_tc_end[ia][ib][ic][id][ie] = nullptr;
            h_timeFSLPC_end[ia][ib][ic][id][ie] = nullptr;
            hc_timeFSLPC_end[ia][ib][ic][id][ie] = nullptr;
          }
        }
      }
    }
  }
  fcn_pol1 = nullptr;
  fcn_const = nullptr;
  fcn_gaus = nullptr;
  fcn_land = nullptr;

  m_outFile = nullptr;
}

KLMTimeCalibrationAlgorithm::~KLMTimeCalibrationAlgorithm()
{
}

CalibrationAlgorithm::EResult KLMTimeCalibrationAlgorithm::calibrate()
{
  int channelId;
  double effSpeed_end, effSpeed, effSpeed_RPC;
  m_timeCableDelay = new KLMTimeCableDelay();
  m_timeConstants = new KLMTimeConstants();

  fcn_gaus = new TF1("fcn_gaus", "gaus");
  fcn_land = new TF1("fcn_land", "landau");
  fcn_pol1 = new TF1("fcn_pol1", "pol1");
  fcn_const = new TF1("fcn_const", "pol0");

  B2INFO("Read tree entries and seprate events by module id.");

  t_tin = getObjectPtr<TTree>("time_calibration_data");
  t_tin->SetBranchAddress("t0",           &ev.t0);
  t_tin->SetBranchAddress("flyTime",      &ev.flyTime);
  t_tin->SetBranchAddress("recTime",      &ev.recTime);
  t_tin->SetBranchAddress("dist",         &ev.dist);
  t_tin->SetBranchAddress("diffDistX",    &ev.diffDistX);
  t_tin->SetBranchAddress("diffDistY",    &ev.diffDistY);
  t_tin->SetBranchAddress("diffDistZ",    &ev.diffDistZ);
  t_tin->SetBranchAddress("eDep",         &ev.eDep);
  t_tin->SetBranchAddress("nPE",          &ev.nPE);
  t_tin->SetBranchAddress("channelId",    &ev.channelId);
  t_tin->SetBranchAddress("inRPC",        &ev.inRPC);
  t_tin->SetBranchAddress("isFlipped",    &ev.isFlipped);

  B2INFO(LogVar("Total number of digit event:", t_tin->GetEntries()));
  m_evts.clear();

  for (int i = 0; i < t_tin->GetEntries(); ++i) {
    t_tin->GetEntry(i);
    m_evts[ev.channelId].push_back(ev);
  }
  B2INFO("Events packing finish.");

  m_outFile = new TFile("histTimeCalibration.root", "recreate");
  h_diff = new TH1D("h_diff", "Position difference between bklmHit2d and extHit;position difference", 100, 0, 10);
  h_calibrated = new TH1I("h_calibrated_summary", "h_calibrated_summary;calibrated or not", 3, 0, 3);

  double lowEdge_rpc = 0.0;
  double upEdge_rpc = 10.0;
  double lowEdge_scint = 0.0;
  double upEdge_scint = 10.0;
  double lowEdge_scint_end = 0.0;
  double upEdge_scint_end = 10.0;

  B2INFO("Sample Type" << LogVar("data or mc", m_mc));
  //if (m_mc) {
  //lowEdge_scint = -20.0;
  //lowEdge_rpc = -20.0;
  //upEdge_scint = 30.0;
  //upEdge_rpc = 30.0;
  //} else {
  lowEdge_rpc = -800.0;
  upEdge_rpc = -600.0;
  lowEdge_scint = -4800.0;
  upEdge_scint = -4400.0;
  lowEdge_scint_end = -4950.0;
  upEdge_scint_end = -4650.0;
  //}
  int nBin = 200;
  int nBin_scint = 400;

  TString iFstring[2] = {"Backward", "Forward"};
  TString iPstring[2] = {"ZReadout", "PhiReadout"};
  TString hn, ht;

  gre_time_channel_scint = new TGraphErrors();
  gre_time_channel_rpc = new TGraphErrors();
  gre_time_channel_scint_end = new TGraphErrors();

  gr_timeShift_channel_scint = new TGraph();
  gr_timeShift_channel_rpc = new TGraph();
  gr_timeShift_channel_scint_end = new TGraph();

  hprf_rpc_phi_effC = new TProfile("hprf_rpc_phi_effC",
                                   "Time over propagation length for RPCs (Phi_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]", 400, 0.0,
                                   400.0);
  hprf_rpc_z_effC = new TProfile("hprf_rpc_z_effC",
                                 "Time over propagation length for RPCs (Z_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]", 400, 0.0,
                                 400.0);
  hprf_scint_phi_effC = new TProfile("hprf_scint_phi_effC",
                                     "Time over propagation length for scintillators (Phi_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]", 350,
                                     0.0, 350.0);
  hprf_scint_z_effC = new TProfile("hprf_scint_z_effC",
                                   "Time over propagation length for scintillators (Z_Readout); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                   350, 0.0, 350.0);
  hprf_scint_plane1_effC_end = new TProfile("hprf_scint_plane1_effC_end",
                                            "Time over propagation length for scintillators (plane1, Endcap); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                            350, 0.0, 350.0);
  hprf_scint_plane2_effC_end = new TProfile("hprf_scint_plane2_effC_end",
                                            "Time over propagation length for scintillators (plane2, Endcap); propagation distance[cm]; T_rec-T_0-T_fly-'T_calibration'[ns]",
                                            350, 0.0, 350.0);

  h_time_rpc_tc = new TH1D("h_time_rpc_tc", "time distribtution for RPC", nBin, lowEdge_rpc, upEdge_rpc);
  h_time_scint_tc = new TH1D("h_time_scint_tc", "time distribtution for Scintillator", nBin_scint, lowEdge_scint, upEdge_scint);
  h_time_scint_tc_end = new TH1D("h_time_scint_tc_end", "time distribtution for Scintillator (Endcap)", nBin_scint, lowEdge_scint_end,
                                 upEdge_scint_end);

  /** Hist declaration Global time distribution **/
  h_time_rpc = new TH1D("h_time_rpc", "time distribtution for RPC; T_rec-T_0-T_fly-T_propagation[ns]", nBin, lowEdge_rpc, upEdge_rpc);
  h_time_scint = new TH1D("h_time_scint", "time distribtution for Scintillator; T_rec-T_0-T_fly-T_propagation[ns]", nBin_scint,
                          lowEdge_scint, upEdge_scint);
  h_time_scint_end = new TH1D("h_time_scint_end", "time distribtution for Scintillator (Endcap); T_rec-T_0-T_fly-T_propagation[ns]",
                              nBin_scint, lowEdge_scint_end, upEdge_scint_end);

  hc_time_rpc = new TH1D("hc_time_rpc", "Calibrated time distribtution for RPC; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                         nBin, lowEdge_rpc, upEdge_rpc);
  hc_time_scint = new TH1D("hc_time_scint",
                           "Calibrated time distribtution for Scintillator; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", nBin_scint, lowEdge_scint,
                           upEdge_scint);
  hc_time_scint_end = new TH1D("hc_time_scint_end",
                               "Calibrated time distribtution for Scintillator (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", nBin_scint,
                               lowEdge_scint_end, upEdge_scint_end);

  for (int iF = 0; iF < 2; ++iF) {
    hn = Form("h_timeF%d_rpc", iF);
    ht = Form("Time distribtution for RPC of %s; T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h_timeF_rpc[iF] = new TH1D(hn.Data(), ht.Data(), nBin, lowEdge_rpc, upEdge_rpc);
    hn = Form("h_timeF%d_scint", iF);
    ht = Form("Time distribtution for Scintillator of %s; T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h_timeF_scint[iF] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint, upEdge_scint);
    hn = Form("h_timeF%d_scint_end", iF);
    ht = Form("Time distribtution for Scintillator of %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h_timeF_scint_end[iF] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint_end, upEdge_scint_end);

    hn = Form("h2_timeF%d_rpc", iF);
    ht = Form("Time distribtution for RPC of %s; Sector Index; T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h2_timeF_rpc[iF] = new TH2D(hn.Data(), ht.Data(), 8, 0, 8, nBin, lowEdge_rpc, upEdge_rpc);
    hn = Form("h2_timeF%d_scint", iF);
    ht = Form("Time distribtution for Scintillator of %s; Sector Index; T_rec-T_0-T_fly-T_propagation[ns]", iFstring[iF].Data());
    h2_timeF_scint[iF] = new TH2D(hn.Data(), ht.Data(), 8, 0, 8, nBin_scint, lowEdge_scint, upEdge_scint);
    hn = Form("h2_timeF%d_scint_end", iF);
    ht = Form("Time distribtution for Scintillator of %s (Endcap); Sector Index; T_rec-T_0-T_fly-T_propagation[ns]",
              iFstring[iF].Data());
    h2_timeF_scint_end[iF] = new TH2D(hn.Data(), ht.Data(), 4, 0, 4, nBin_scint, lowEdge_scint_end, upEdge_scint_end);

    hn = Form("hc_timeF%d_rpc", iF);
    ht = Form("Calibrated time distribtution for RPC of %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iFstring[iF].Data());
    hc_timeF_rpc[iF] = new TH1D(hn.Data(), ht.Data(), nBin, lowEdge_rpc, upEdge_rpc);
    hn = Form("hc_timeF%d_scint", iF);
    ht = Form("Calibrated time distribtution for Scintillator of %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    hc_timeF_scint[iF] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint, upEdge_scint);
    hn = Form("hc_timeF%d_scint_end", iF);
    ht = Form("Calibrated time distribtution for Scintillator of %s (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    hc_timeF_scint_end[iF] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint_end, upEdge_scint_end);

    hn = Form("h2c_timeF%d_rpc", iF);
    ht = Form("Calibrated time distribtution for RPC of %s; Sector Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    h2c_timeF_rpc[iF] = new TH2D(hn.Data(), ht.Data(), 8, 0, 8, nBin, lowEdge_rpc, upEdge_rpc);
    hn = Form("h2c_timeF%d_scint", iF);
    ht = Form("Calibrated time distribtution for Scintillator of %s; Sector Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    h2c_timeF_scint[iF] = new TH2D(hn.Data(), ht.Data(), 8, 0, 8, nBin_scint, lowEdge_scint, upEdge_scint);
    hn = Form("h2c_timeF%d_scint_end", iF);
    ht = Form("Calibrated time distribtution for Scintillator of %s (Endcap) ; Sector Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
              iFstring[iF].Data());
    h2c_timeF_scint_end[iF] = new TH2D(hn.Data(), ht.Data(), 4, 0, 4, nBin_scint, lowEdge_scint_end, upEdge_scint_end);

    for (int iS = 0; iS < 8; ++iS) {
      // Barrel parts
      hn = Form("h_timeF%d_S%d_scint", iF, iS);
      ht = Form("Time distribtution for Scintillator of Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iS, iFstring[iF].Data());
      h_timeFS_scint[iF][iS] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint, upEdge_scint);
      hn = Form("h_timeF%d_S%d_rpc", iF, iS);
      ht = Form("Time distribtution for RPC of Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iS, iFstring[iF].Data());
      h_timeFS_rpc[iF][iS] = new TH1D(hn.Data(), ht.Data(), nBin, lowEdge_rpc, upEdge_rpc);
      hn = Form("h2_timeF%d_S%d", iF, iS);
      ht = Form("Time distribtution of Sector%d, %s; Layer Index; T_rec-T_0-T_fly-T_propagation[ns]", iS, iFstring[iF].Data());
      h2_timeFS[iF][iS] = new TH2D(hn.Data(), ht.Data(), 15, 0, 15, nBin_scint, lowEdge_rpc, upEdge_scint);

      hn = Form("hc_timeF%d_S%d_scint", iF, iS);
      ht = Form("Calibrated time distribtution for Scintillator of Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iS,
                iFstring[iF].Data());
      hc_timeFS_scint[iF][iS] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint, upEdge_scint);
      hn = Form("hc_timeF%d_S%d_rpc", iF, iS);
      ht = Form("Calibrated time distribtution for RPC of Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iS,
                iFstring[iF].Data());
      hc_timeFS_rpc[iF][iS] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_rpc, upEdge_rpc);
      hn = Form("h2c_timeF%d_S%d", iF, iS);
      ht = Form("Calibrated time distribtution of Sector%d, %s; Layer Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iS,
                iFstring[iF].Data());
      h2c_timeFS[iF][iS] = new TH2D(hn.Data(), ht.Data(), 15, 0, 15, nBin_scint, lowEdge_rpc, upEdge_scint);

      // Inner 2 layers --> Scintillators
      for (int iL = 0; iL < 2; ++iL) {
        hn = Form("h_timeF%d_S%d_L%d", iF, iS, iL);
        ht = Form("Time distribtution for Scintillator of Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iL, iS,
                  iFstring[iF].Data());
        h_timeFSL[iF][iS][iL] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint, upEdge_scint);
        hn = Form("hc_timeF%d_S%d_L%d", iF, iS, iL);
        ht = Form("Calibrated time distribtution for Scintillator of Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                  iL, iS, iFstring[iF].Data());
        hc_timeFSL[iF][iS][iL] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint, upEdge_scint);

        for (int iP = 0; iP < 2; ++iP) {
          hn = Form("h_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Time distribtution for Scintillator of %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h_timeFSLP[iF][iS][iL][iP] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint, upEdge_scint);
          hn = Form("h2_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Time distribtution for Scintillator of %s, Layer%d, Sector%d, %s; Channel Index; T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2_timeFSLP[iF][iS][iL][iP] = new TH2D(hn.Data(), ht.Data(), 54, 0, 54, nBin_scint, lowEdge_scint, upEdge_scint);

          hn = Form("hc_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Calibrated time distribtution for Scintillator of %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          hc_timeFSLP[iF][iS][iL][iP] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint, upEdge_scint);
          hn = Form("h2c_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Calibrated time distribtution for Scintillator of %s, Layer%d, Sector%d, %s; Channel Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2c_timeFSLP[iF][iS][iL][iP] = new TH2D(hn.Data(), ht.Data(), 54, 0, 54, nBin_scint, lowEdge_scint, upEdge_scint);

          int nchannel_max = BKLMElementNumbers::getNStrips(iF, iS + 1, iL + 1, iP);
          for (int iC = 0; iC < nchannel_max; ++iC) {
            hn = Form("h_timeF%d_S%d_L%d_P%d_C%d_tc", iF, iS, iL, iP, iC);
            ht = Form("time distribtution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iC,
                      iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            h_timeFSLPC_tc[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint, upEdge_scint);

            hn = Form("h_timeF%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
            ht = Form("time distribtution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iC,
                      iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            h_timeFSLPC[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint, upEdge_scint);

            hn = Form("hc_timeF%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
            ht = Form("Calibrated time distribtution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                      iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            hc_timeFSLPC[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint, upEdge_scint);
          }
        }
      }

      for (int iL = 2; iL < 15; ++iL) {
        hn = Form("h_timeF%d_S%d_L%d", iF, iS, iL);
        ht = Form("time distribtution for RPC of Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iL, iS, iFstring[iF].Data());
        h_timeFSL[iF][iS][iL] = new TH1D(hn.Data(), ht.Data(), nBin, lowEdge_rpc, upEdge_rpc);

        hn = Form("hc_timeF%d_S%d_L%d", iF, iS, iL);
        ht = Form("Calibrated time distribtution for RPC of Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]", iL, iS,
                  iFstring[iF].Data());
        hc_timeFSL[iF][iS][iL] = new TH1D(hn.Data(), ht.Data(), nBin, lowEdge_rpc, upEdge_rpc);

        for (int iP = 0; iP < 2; ++iP) {
          hn = Form("h_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("time distribtution for RPC of %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iPstring[iP].Data(), iL, iS,
                    iFstring[iF].Data());
          h_timeFSLP[iF][iS][iL][iP] = new TH1D(hn.Data(), ht.Data(), nBin, lowEdge_rpc, upEdge_rpc);

          hn = Form("h2_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("time distribtution for RPC of %s, Layer%d, Sector%d, %s; Channel Index; T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2_timeFSLP[iF][iS][iL][iP] = new TH2D(hn.Data(), ht.Data(), 48, 0, 48, nBin, lowEdge_rpc, upEdge_rpc);

          hn = Form("hc_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Calibrated time distribtution for RPC of %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          hc_timeFSLP[iF][iS][iL][iP] = new TH1D(hn.Data(), ht.Data(), nBin, lowEdge_rpc, upEdge_rpc);

          hn = Form("h2c_timeF%d_S%d_L%d_P%d", iF, iS, iL, iP);
          ht = Form("Calibrated time distribtution for RPC of %s, Layer%d, Sector%d, %s; Channel Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2c_timeFSLP[iF][iS][iL][iP] = new TH2D(hn.Data(), ht.Data(), 48, 0, 48, nBin, lowEdge_rpc, upEdge_rpc);

          int nchannel_max = BKLMElementNumbers::getNStrips(iF, iS + 1, iL + 1, iP);
          for (int iC = 0; iC < nchannel_max; ++iC) {
            hn = Form("h_timeF%d_S%d_L%d_P%d_C%d_tc", iF, iS, iL, iP, iC);
            ht = Form("Time distribtution for RPC of Channel%d, %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iC,
                      iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            h_timeFSLPC_tc[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin, lowEdge_rpc, upEdge_rpc);

            hn = Form("h_timeF%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
            ht = Form("Time distribtution for RPC of Channel%d, %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation[ns]", iC,
                      iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            h_timeFSLPC[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin, lowEdge_rpc, upEdge_rpc);

            hn = Form("hc_timeF%d_S%d_L%d_P%d_C%d", iF, iS, iL, iP, iC);
            ht = Form("Calibrated time distribtution for RPC of Channel%d, %s, Layer%d, Sector%d, %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                      iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            hc_timeFSLPC[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin, lowEdge_rpc, upEdge_rpc);
          }
        }
      }
    }
    // Endcap part
    int maxLay = 12 + 2 * iF;
    for (int iS = 0; iS < 4; ++iS) {
      hn = Form("h_timeF%d_S%d_scint_end", iF, iS);
      ht = Form("Time distribtution for Scintillator of Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]", iS,
                iFstring[iF].Data());
      h_timeFS_scint_end[iF][iS] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint_end, upEdge_scint_end);
      hn = Form("h2_timeF%d_S%d_end", iF, iS);
      ht = Form("Time distribtution of Sector%d, %s (Endcap); Layer Index; T_rec-T_0-T_fly-T_propagation[ns]", iS, iFstring[iF].Data());
      h2_timeFS_end[iF][iS] = new TH2D(hn.Data(), ht.Data(), maxLay, 0, maxLay, nBin_scint, lowEdge_scint_end, upEdge_scint_end);
      hn = Form("hc_timeF%d_S%d_scint_end", iF, iS);
      ht = Form("Calibrated time distribtution for Scintillator of Sector%d (Endcap), %s; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                iS, iFstring[iF].Data());
      hc_timeFS_scint_end[iF][iS] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint_end, upEdge_scint_end);
      hn = Form("h2c_timeF%d_S%d_end", iF, iS);
      ht = Form("Calibrated time distribtution of Sector%d, %s (Endcap); Layer Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                iS, iFstring[iF].Data());
      h2c_timeFS_end[iF][iS] = new TH2D(hn.Data(), ht.Data(), maxLay, 0, maxLay, nBin_scint, lowEdge_scint_end, upEdge_scint_end);

      for (int iL = 0; iL < maxLay; ++iL) {
        hn = Form("h_timeF%d_S%d_L%d_end", iF, iS, iL);
        ht = Form("Time distribtution for Scintillator of Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]", iL, iS,
                  iFstring[iF].Data());
        h_timeFSL_end[iF][iS][iL] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint_end, upEdge_scint_end);
        hn = Form("hc_timeF%d_S%d_L%d_end", iF, iS, iL);
        ht = Form("Calibrated time distribtution for Scintillator of Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                  iL, iS, iFstring[iF].Data());
        hc_timeFSL_end[iF][iS][iL] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint_end, upEdge_scint_end);

        for (int iP = 0; iP < 2; ++iP) {
          hn = Form("h_timeF%d_S%d_L%d_P%d_end", iF, iS, iL, iP);
          ht = Form("Time distribtution for Scintillator of %s, Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h_timeFSLP_end[iF][iS][iL][iP] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint_end, upEdge_scint_end);

          hn = Form("h2_timeF%d_S%d_L%d_P%d_end", iF, iS, iL, iP);
          ht = Form("Time distribtution for Scintillator of %s, Layer%d, Sector%d, %s (Endcap); Channel Index; T_rec-T_0-T_fly-T_propagation[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2_timeFSLP_end[iF][iS][iL][iP] = new TH2D(hn.Data(), ht.Data(), 75, 0, 75, nBin_scint, lowEdge_scint_end, upEdge_scint_end);

          hn = Form("hc_timeF%d_S%d_L%d_P%d_end", iF, iS, iL, iP);
          ht = Form("Calibrated time distribtution for Scintillator of %s, Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          hc_timeFSLP_end[iF][iS][iL][iP] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint_end, upEdge_scint_end);

          hn = Form("h2c_timeF%d_S%d_L%d_P%d_end", iF, iS, iL, iP);
          ht = Form("Calibrated time distribtution for Scintillator of %s, Layer%d, Sector%d, %s (Endcap); Channel Index; T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                    iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
          h2c_timeFSLP_end[iF][iS][iL][iP] = new TH2D(hn.Data(), ht.Data(), 75, 0, 75, nBin_scint, lowEdge_scint_end, upEdge_scint_end);

          for (int iC = 0; iC < 75; ++iC) {
            hn = Form("h_timeF%d_S%d_L%d_P%d_C%d_tc_end", iF, iS, iL, iP, iC);
            ht = Form("Time distribtution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]",
                      iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            h_timeFSLPC_tc_end[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint_end, upEdge_scint_end);

            hn = Form("h_timeF%d_S%d_L%d_P%d_C%d_end", iF, iS, iL, iP, iC);
            ht = Form("Time distribtution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation[ns]",
                      iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            h_timeFSLPC_end[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint_end, upEdge_scint_end);

            hn = Form("hc_timeF%d_S%d_L%d_P%d_C%d_end", iF, iS, iL, iP, iC);
            ht = Form("Calibrated time distribtution for Scintillator of Channel%d, %s, Layer%d, Sector%d, %s (Endcap); T_rec-T_0-T_fly-T_propagation-T_calibration[ns]",
                      iC, iPstring[iP].Data(), iL, iS, iFstring[iF].Data());
            hc_timeFSLPC_end[iF][iS][iL][iP][iC] = new TH1D(hn.Data(), ht.Data(), nBin_scint, lowEdge_scint_end, upEdge_scint_end);
          }
        }
      }
    }
  }


  std::map<uint16_t, std::vector<struct Event> >::iterator it_m;
  std::vector<struct Event>::iterator it_v;

  m_evtsChannel.clear();
  m_cFlag.clear();
  m_minimizerOptions.SetDefaultStrategy(2);

  m_time_channelAvg_scint = 0.0;
  m_time_channelAvg_rpc = 0.0;

  /**********************************
   * First loop
   * Estimation of effected light speed for Scintillators and RPCs, separately.
   **********************************/
  B2INFO("Effected Light Speed Estimation.");
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    int iSub = klmChannel.getSubdetector();

    m_cFlag[channelId] = 0;
    if (m_evts.find(channelId) == m_evts.end()) continue;
    m_evtsChannel = m_evts[channelId];

    int n = m_evtsChannel.size();
    if (n < m_lower_limit_counts) {
      B2WARNING("Not enough calibration data collected." << LogVar("channel", channelId) << LogVar("number of digit", n));
      continue;
    }

    m_cFlag[channelId] = 1;

    for (it_v = m_evtsChannel.begin(); it_v != m_evtsChannel.end(); ++it_v) {
      TVector3 diffD = TVector3(it_v->diffDistX, it_v->diffDistY, it_v->diffDistZ);
      h_diff->Fill(diffD.Mag());
      double timeHit = it_v->time();
      if (m_useEventT0) timeHit = timeHit - it_v->t0;
      if (iSub == KLMElementNumbers::c_BKLM) {
        int iF = klmChannel.getSection();
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane();
        int iC = klmChannel.getStrip() - 1;
        h_timeFSLPC_tc[iF][iS][iL][iP][iC]->Fill(timeHit);
        if (iL > 1) {
          h_time_rpc_tc->Fill(timeHit);
        } else {
          h_time_scint_tc->Fill(timeHit);
        }
      } else {
        int iF = klmChannel.getSection() - 1;
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane() - 1;
        int iC = klmChannel.getStrip() - 1;
        h_timeFSLPC_tc_end[iF][iS][iL][iP][iC]->Fill(timeHit);
        h_time_scint_tc_end->Fill(timeHit);
      }
    }
  }
  B2INFO("Effected Light Speed Estimation! Hists and Graph filling done.");

  m_timeShift.clear();

  double tmpMean_rpc_global = h_time_rpc_tc->GetMean();
  double tmpMean_scint_global = h_time_scint_tc->GetMean();
  double tmpMean_scint_global_end = h_time_scint_tc_end->GetMean();

  B2INFO("Global Mean for Raw." << LogVar("RPC", tmpMean_rpc_global) << LogVar("Scint BKLM",
         tmpMean_scint_global) << LogVar("Scint EKLM", tmpMean_scint_global_end));

  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (!m_cFlag[channelId]) continue;

    int iSub = klmChannel.getSubdetector();
    if (iSub == KLMElementNumbers::c_BKLM) {
      int iF = klmChannel.getSection();
      int iS = klmChannel.getSector() - 1;
      int iL = klmChannel.getLayer() - 1;
      int iP = klmChannel.getPlane();
      int iC = klmChannel.getStrip() - 1;
      h_timeFSLPC_tc[iF][iS][iL][iP][iC]->Fit(fcn_gaus, "LESQ");
      double tmpMean_channel = fcn_gaus->GetParameter(1);
      if (iL > 1) {
        m_timeShift[channelId] = tmpMean_channel - tmpMean_rpc_global;
      } else {
        m_timeShift[channelId] = tmpMean_channel - tmpMean_scint_global;
      }
    } else {
      int iF = klmChannel.getSection() - 1;
      int iS = klmChannel.getSector() - 1;
      int iL = klmChannel.getLayer() - 1;
      int iP = klmChannel.getPlane() - 1;
      int iC = klmChannel.getStrip() - 1;
      h_timeFSLPC_tc_end[iF][iS][iL][iP][iC]->Fit(fcn_gaus, "LESQ");
      double tmpMean_channel = fcn_gaus->GetParameter(1);
      m_timeShift[channelId] = tmpMean_channel - tmpMean_scint_global_end;
    }
  }

  delete h_time_scint_tc;
  delete h_time_scint_tc_end;
  delete h_time_rpc_tc;
  B2INFO("Effected Light m_timeShift obtained. done.");

  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (!m_cFlag[channelId]) continue;

    m_evtsChannel = m_evts[channelId];
    int iSub = klmChannel.getSubdetector();

    for (it_v = m_evtsChannel.begin(); it_v != m_evtsChannel.end(); ++it_v) {
      double timeHit = it_v->time() - m_timeShift[channelId];
      if (m_useEventT0) timeHit = timeHit - it_v->t0;
      double distHit = it_v->dist;

      if (iSub == KLMElementNumbers::c_BKLM) {
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane();
        if (iL > 1) {
          if (iP) {
            hprf_rpc_phi_effC->Fill(distHit, timeHit);
          } else {
            hprf_rpc_z_effC->Fill(distHit, timeHit);
          }
        } else {
          if (iP) {
            hprf_scint_phi_effC->Fill(distHit, timeHit);
          } else {
            hprf_scint_z_effC->Fill(distHit, timeHit);
          }
        }
      } else {
        int iP = klmChannel.getPlane() - 1;
        if (iP) {
          hprf_scint_plane1_effC_end->Fill(distHit, timeHit);
        } else {
          hprf_scint_plane2_effC_end->Fill(distHit, timeHit);
        }
      }
    }
  }

  B2INFO("Effective Ligit Spead Fitting.");
  hprf_rpc_phi_effC->Fit("fcn_pol1", "EMQ");
  double slope_rpc_phi = fcn_pol1->GetParameter(1);
  double e_slope_rpc_phi = fcn_pol1->GetParError(1);
  double effC_rpc_phi = 1.0 / slope_rpc_phi;
  double e_effC_rpc_phi = e_slope_rpc_phi / (slope_rpc_phi * slope_rpc_phi);

  hprf_rpc_z_effC->Fit("fcn_pol1", "EMQ");
  double slope_rpc_z = fcn_pol1->GetParameter(1);
  double e_slope_rpc_z = fcn_pol1->GetParError(1);
  double effC_rpc_z = 1.0 / slope_rpc_z;
  double e_effC_rpc_z = e_slope_rpc_z / (slope_rpc_z * slope_rpc_z);

  hprf_scint_phi_effC->Fit("fcn_pol1", "EMQ");
  double slope_scint_phi = fcn_pol1->GetParameter(1);
  double e_slope_scint_phi = fcn_pol1->GetParError(1);
  double effC_scint_phi = 1.0 / slope_scint_phi;
  double e_effC_scint_phi = e_slope_scint_phi / (slope_scint_phi * slope_scint_phi);

  hprf_scint_z_effC->Fit("fcn_pol1", "EMQ");
  double slope_scint_z = fcn_pol1->GetParameter(1);
  double e_slope_scint_z = fcn_pol1->GetParError(1);
  double effC_scint_z = 1.0 / slope_scint_z;
  double e_effC_scint_z = e_slope_scint_z / (slope_scint_z * slope_scint_z);

  hprf_scint_plane1_effC_end->Fit("fcn_pol1", "EMQ");
  double slope_scint_plane1_end = fcn_pol1->GetParameter(1);
  double e_slope_scint_plane1_end = fcn_pol1->GetParError(1);
  double effC_scint_plane1_end = 1.0 / slope_scint_plane1_end;
  double e_effC_scint_plane1_end = e_slope_scint_plane1_end / (slope_scint_plane1_end * slope_scint_plane1_end);

  hprf_scint_plane2_effC_end->Fit("fcn_pol1", "EMQ");
  double slope_scint_plane2_end = fcn_pol1->GetParameter(1);
  double e_slope_scint_plane2_end = fcn_pol1->GetParError(1);
  double effC_scint_plane2_end = 1.0 / slope_scint_plane2_end;
  double e_effC_scint_plane2_end = e_slope_scint_plane2_end / (slope_scint_plane2_end * slope_scint_plane2_end);

  TString logStr_phi, logStr_z;
  logStr_phi = Form("%.4f +/- %.4f cm/ns", effC_rpc_phi, e_effC_rpc_phi);
  logStr_z = Form("%.4f +/- %.4f cm/ns", effC_rpc_z, e_effC_rpc_z);
  B2INFO("Estimation of Effected Speed Light of RPCs: "
         << LogVar("Fitted Value (phi readout) ", logStr_phi.Data())
         << LogVar("Fitted Value (z readout) ", logStr_z.Data()));
  logStr_phi = Form("%.4f +/- %.4f cm/ns", effC_scint_phi, e_effC_scint_phi);
  logStr_z = Form("%.4f +/- %.4f cm/ns", effC_scint_z, e_effC_scint_z);
  B2INFO("Estimation of Effected Speed Light of Scintillators: "
         << LogVar("Fitted Value (phi readout) ", logStr_phi.Data())
         << LogVar("Fitted Value (z readout) ", logStr_z.Data()));
  logStr_phi = Form("%.4f +/- %.4f cm/ns", effC_scint_plane1_end, e_effC_scint_plane1_end);
  logStr_z = Form("%.4f +/- %.4f cm/ns", effC_scint_plane2_end, e_effC_scint_plane2_end);
  B2INFO("Estimation of Effected Speed Light of Scintillators (EKLM): "
         << LogVar("Fitted Value (plane1 readout) ", logStr_phi.Data())
         << LogVar("Fitted Value (plane2 readout) ", logStr_z.Data()));

  // Default Effected Light Speed in current Database
  effSpeed_end = 0.5 * (fabs(effC_scint_plane1_end) + fabs(effC_scint_plane2_end));
  effSpeed = 0.5 * (fabs(effC_scint_phi) + fabs(effC_scint_z));
  effSpeed_RPC = 0.5 * (fabs(effC_rpc_phi) + fabs(effC_rpc_z));

  effSpeed_RPC = 0.50 * Const::speedOfLight;

  m_timeConstants->setEffLightSpeed(effSpeed_end, KLMTimeConstants::c_EKLM);
  m_timeConstants->setAmpTimeConstant(0, KLMTimeConstants::c_EKLM);
  m_timeConstants->setEffLightSpeed(effSpeed, KLMTimeConstants::c_BKLM);
  m_timeConstants->setAmpTimeConstant(0, KLMTimeConstants::c_BKLM);
  m_timeConstants->setEffLightSpeed(effSpeed_RPC, KLMTimeConstants::c_RPC);
  m_timeConstants->setAmpTimeConstant(0, KLMTimeConstants::c_RPC);

  effSpeed_end = 0.5671 * Const::speedOfLight;
  effSpeed = 0.5671 * Const::speedOfLight;

  /** ======================================================================================= **/
  B2INFO("Time distribution filling begins.");
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    int iSub = klmChannel.getSubdetector();

    if (!m_cFlag[channelId]) continue;
    m_evtsChannel = m_evts[channelId];

    for (it_v = m_evtsChannel.begin(); it_v != m_evtsChannel.end(); ++it_v) {
      double timeHit = it_v->time();
      if (m_useEventT0) timeHit = timeHit - it_v->t0;
      if (iSub == KLMElementNumbers::c_BKLM) {
        int iF = klmChannel.getSection();
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane();
        int iC = klmChannel.getStrip() - 1;
        if (iL > 1) {
          double propgationT = it_v->dist / effSpeed_RPC;
          h_time_rpc->Fill(timeHit - propgationT);
          h_timeF_rpc[iF]->Fill(timeHit - propgationT);
          h_timeFS_rpc[iF][iS]->Fill(timeHit - propgationT);
          h_timeFSL[iF][iS][iL]->Fill(timeHit - propgationT);
          h_timeFSLP[iF][iS][iL][iP]->Fill(timeHit - propgationT);
          h_timeFSLPC[iF][iS][iL][iP][iC]->Fill(timeHit - propgationT);
          h2_timeF_rpc[iF]->Fill(iS, timeHit - propgationT);
          h2_timeFS[iF][iS]->Fill(iL, timeHit - propgationT);
          h2_timeFSLP[iF][iS][iL][iP]->Fill(iC, timeHit - propgationT);
        } else {
          double propgationT = it_v->dist / effSpeed;
          h_time_scint->Fill(timeHit - propgationT);
          h_timeF_scint[iF]->Fill(timeHit - propgationT);
          h_timeFS_scint[iF][iS]->Fill(timeHit - propgationT);
          h_timeFSL[iF][iS][iL]->Fill(timeHit - propgationT);
          h_timeFSLP[iF][iS][iL][iP]->Fill(timeHit - propgationT);
          h_timeFSLPC[iF][iS][iL][iP][iC]->Fill(timeHit - propgationT);
          h2_timeF_scint[iF]->Fill(iS, timeHit - propgationT);
          h2_timeFS[iF][iS]->Fill(iL, timeHit - propgationT);
          h2_timeFSLP[iF][iS][iL][iP]->Fill(iC, timeHit - propgationT);
        }
      } else {
        int iF = klmChannel.getSection() - 1;
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane() - 1;
        int iC = klmChannel.getStrip() - 1;
        double propgationT = it_v->dist / effSpeed;
        h_time_scint_end->Fill(timeHit - propgationT);
        h_timeF_scint_end[iF]->Fill(timeHit - propgationT);
        h_timeFS_scint_end[iF][iS]->Fill(timeHit - propgationT);
        h_timeFSL_end[iF][iS][iL]->Fill(timeHit - propgationT);
        h_timeFSLP_end[iF][iS][iL][iP]->Fill(timeHit - propgationT);
        h_timeFSLPC_end[iF][iS][iL][iP][iC]->Fill(timeHit - propgationT);
        h2_timeF_scint_end[iF]->Fill(iS, timeHit - propgationT);
        h2_timeFS_end[iF][iS]->Fill(iL, timeHit - propgationT);
        h2_timeFSLP_end[iF][iS][iL][iP]->Fill(iC, timeHit - propgationT);
      }
    }
  }

  B2INFO("Orignal Filling Done.");

  int iChannel_rpc = 0;
  int iChannel = 0;
  int iChannel_end = 0;
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (!m_cFlag[channelId]) continue;
    int iSub = klmChannel.getSubdetector();

    if (iSub == KLMElementNumbers::c_BKLM) {
      int iF = klmChannel.getSection();
      int iS = klmChannel.getSector() - 1;
      int iL = klmChannel.getLayer() - 1;
      int iP = klmChannel.getPlane();
      int iC = klmChannel.getStrip() - 1;

      TFitResultPtr r = h_timeFSLPC[iF][iS][iL][iP][iC]->Fit(fcn_gaus, "LESQ");
      if (int(r) != 0) continue;
      if (int(r) == 0) m_cFlag[channelId] = 2;
      m_time_channel[channelId] = fcn_gaus->GetParameter(1);
      m_etime_channel[channelId] = fcn_gaus->GetParError(1);
      if (iL > 1) {
        gre_time_channel_rpc->SetPoint(iChannel, channelId, m_time_channel[channelId]);
        gre_time_channel_rpc->SetPointError(iChannel, 0., m_etime_channel[channelId]);
        iChannel++;
      } else {
        gre_time_channel_scint->SetPoint(iChannel_rpc, channelId, m_time_channel[channelId]);
        gre_time_channel_scint->SetPointError(iChannel_rpc, 0., m_etime_channel[channelId]);
        iChannel_rpc++;
      }
    } else {
      int iF = klmChannel.getSection() - 1;
      int iS = klmChannel.getSector() - 1;
      int iL = klmChannel.getLayer() - 1;
      int iP = klmChannel.getPlane() - 1;
      int iC = klmChannel.getStrip() - 1;

      TFitResultPtr r = h_timeFSLPC_end[iF][iS][iL][iP][iC]->Fit(fcn_gaus, "LESQ");
      if (int(r) != 0) continue;
      if (int(r) == 0) m_cFlag[channelId] = 2;
      m_time_channel[channelId] = fcn_gaus->GetParameter(1);
      m_etime_channel[channelId] = fcn_gaus->GetParError(1);
      gre_time_channel_scint_end->SetPoint(iChannel_end, channelId, m_time_channel[channelId]);
      gre_time_channel_scint_end->SetPointError(iChannel_end, 0., m_etime_channel[channelId]);
      iChannel_end++;
    }
  }

  gre_time_channel_scint->Fit("fcn_const", "EMQ");
  m_time_channelAvg_scint = fcn_const->GetParameter(0);
  m_etime_channelAvg_scint = fcn_const->GetParError(0);

  gre_time_channel_scint_end->Fit("fcn_const", "EMQ");
  m_time_channelAvg_scint_end = fcn_const->GetParameter(0);
  m_etime_channelAvg_scint_end = fcn_const->GetParError(0);

  gre_time_channel_rpc->Fit("fcn_const", "EMQ");
  m_time_channelAvg_rpc = fcn_const->GetParameter(0);
  m_etime_channelAvg_rpc = fcn_const->GetParError(0);

  B2INFO("Channel's time distribution Fitting Done.");
  B2DEBUG(29, LogVar("Average Time (RPC)", m_time_channelAvg_rpc) << LogVar("Average Time (Scint BKLM)",
          m_time_channelAvg_scint) << LogVar("Average Time (Scint EKLM)", m_time_channelAvg_scint_end));

  B2INFO("Calibrated channel's time distribution Filling. Begins.");

  m_timeShift.clear();
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    h_calibrated->Fill(int(m_cFlag[channelId]));
    if (m_time_channel.find(channelId) == m_time_channel.end()) continue;

    int iSub = klmChannel.getSubdetector();
    if (iSub == KLMElementNumbers::c_BKLM) {
      int iL = klmChannel.getLayer() - 1;
      if (iL > 1) {
        m_timeShift[channelId] = m_time_channel[channelId] - m_time_channelAvg_rpc;
      } else {
        m_timeShift[channelId] = m_time_channel[channelId] - m_time_channelAvg_scint;
      }
    } else {
      m_timeShift[channelId] = m_time_channel[channelId] - m_time_channelAvg_scint_end;
    }
    m_timeCableDelay->setTimeShift(channelId, m_timeShift[channelId]);
  }

  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (m_timeShift.find(channelId) != m_timeShift.end()) continue;
    m_timeShift[channelId] = esti_timeShift(klmChannel);
    m_timeCableDelay->setTimeShift(channelId, m_timeShift[channelId]);
    B2DEBUG(29, "Uncalibrated Estimation " << LogVar("Channel", channelId) << LogVar("Estimated value", m_timeShift[channelId]));
  }

  iChannel_rpc = 0;
  iChannel = 0;
  iChannel_end = 0;
  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    if (m_timeShift.find(channelId) == m_timeShift.end()) {
      B2ERROR("!!! Not All Channels Calibration Constant Set. Error Happended on " << LogVar("Channel", channelId));
      continue;
    }
    int iSub = klmChannel.getSubdetector();
    if (iSub == KLMElementNumbers::c_BKLM) {
      int iL = klmChannel.getLayer();
      if (iL > 2) {
        gr_timeShift_channel_rpc->SetPoint(iChannel, channelId, m_timeShift[channelId]);
        iChannel_rpc++;
      } else {
        gr_timeShift_channel_scint->SetPoint(iChannel, channelId, m_timeShift[channelId]);
        iChannel++;
      }
    } else {
      gr_timeShift_channel_scint_end->SetPoint(iChannel_end, channelId, m_timeShift[channelId]);
      iChannel_end++;
    }
  }

  for (KLMChannelIndex klmChannel = m_klmChannels.begin(); klmChannel != m_klmChannels.end(); ++klmChannel) {
    channelId = klmChannel.getKLMChannelNumber();
    int iSub = klmChannel.getSubdetector();
    m_evtsChannel = m_evts[channelId];
    for (it_v = m_evtsChannel.begin(); it_v != m_evtsChannel.end(); ++it_v) {
      double timeHit = it_v->time();
      if (m_useEventT0) timeHit = timeHit - it_v->t0;
      if (iSub == KLMElementNumbers::c_BKLM) {
        int iF = klmChannel.getSection();
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane();
        int iC = klmChannel.getStrip() - 1;
        if (iL > 1) {
          double propgationT = it_v->dist / effSpeed_RPC;
          hc_time_rpc->Fill(timeHit - propgationT - m_timeShift[channelId]);
          hc_timeF_rpc[iF]->Fill(timeHit - propgationT - m_timeShift[channelId]);
          hc_timeFS_rpc[iF][iS]->Fill(timeHit - propgationT - m_timeShift[channelId]);
          hc_timeFSL[iF][iS][iL]->Fill(timeHit - propgationT - m_timeShift[channelId]);
          hc_timeFSLP[iF][iS][iL][iP]->Fill(timeHit - propgationT - m_timeShift[channelId]);
          hc_timeFSLPC[iF][iS][iL][iP][iC]->Fill(timeHit - propgationT - m_timeShift[channelId]);
          h2c_timeF_rpc[iF]->Fill(iS, timeHit - propgationT - m_timeShift[channelId]);
          h2c_timeFS[iF][iS]->Fill(iL, timeHit - propgationT - m_timeShift[channelId]);
          h2c_timeFSLP[iF][iS][iL][iP]->Fill(iC, timeHit - propgationT - m_timeShift[channelId]);
        } else {
          double propgationT = it_v->dist / effSpeed;
          hc_time_scint->Fill(timeHit - propgationT - m_timeShift[channelId]);
          hc_timeF_scint[iF]->Fill(timeHit - propgationT - m_timeShift[channelId]);
          hc_timeFS_scint[iF][iS]->Fill(timeHit - propgationT - m_timeShift[channelId]);
          hc_timeFSL[iF][iS][iL]->Fill(timeHit - propgationT - m_timeShift[channelId]);
          hc_timeFSLP[iF][iS][iL][iP]->Fill(timeHit - propgationT - m_timeShift[channelId]);
          hc_timeFSLPC[iF][iS][iL][iP][iC]->Fill(timeHit - propgationT - m_timeShift[channelId]);
          h2c_timeF_scint[iF]->Fill(iS, timeHit - propgationT - m_timeShift[channelId]);
          h2c_timeFS[iF][iS]->Fill(iL, timeHit - propgationT - m_timeShift[channelId]);
          h2c_timeFSLP[iF][iS][iL][iP]->Fill(iC, timeHit - propgationT - m_timeShift[channelId]);
        }
      } else {
        int iF = klmChannel.getSection() - 1;
        int iS = klmChannel.getSector() - 1;
        int iL = klmChannel.getLayer() - 1;
        int iP = klmChannel.getPlane() - 1;
        int iC = klmChannel.getStrip() - 1;
        double propgationT = it_v->dist / effSpeed;
        hc_time_scint_end->Fill(timeHit - propgationT - m_timeShift[channelId]);
        hc_timeF_scint_end[iF]->Fill(timeHit - propgationT - m_timeShift[channelId]);
        hc_timeFS_scint_end[iF][iS]->Fill(timeHit - propgationT - m_timeShift[channelId]);
        hc_timeFSL_end[iF][iS][iL]->Fill(timeHit - propgationT - m_timeShift[channelId]);
        hc_timeFSLP_end[iF][iS][iL][iP]->Fill(timeHit - propgationT - m_timeShift[channelId]);
        hc_timeFSLPC_end[iF][iS][iL][iP][iC]->Fill(timeHit - propgationT - m_timeShift[channelId]);
        h2c_timeF_scint_end[iF]->Fill(iS, timeHit - propgationT - m_timeShift[channelId]);
        h2c_timeFS_end[iF][iS]->Fill(iL, timeHit - propgationT - m_timeShift[channelId]);
        h2c_timeFSLP_end[iF][iS][iL][iP]->Fill(iC, timeHit - propgationT - m_timeShift[channelId]);
      }
    }
  }

  saveHist();

  delete fcn_const;
  m_evts.clear();
  m_timeShift.clear();
  m_cFlag.clear();

  saveCalibration(m_timeCableDelay, "KLMTime_cableDelay");
  saveCalibration(m_timeConstants, "KLMTime_basicConstant");
  return CalibrationAlgorithm::c_OK;
}


void KLMTimeCalibrationAlgorithm::saveHist()
{
  m_outFile->cd();
  B2INFO("Save Histograms into Files.");
  TDirectory* dir_monitor = m_outFile->mkdir("monitor_Hists");
  dir_monitor->cd();
  h_calibrated->SetDirectory(dir_monitor);
  h_diff->SetDirectory(dir_monitor);

  m_outFile->cd();
  TDirectory* dir_effC = m_outFile->mkdir("effC_Hists");
  dir_effC->cd();
  hprf_rpc_phi_effC->SetDirectory(dir_effC);
  hprf_rpc_z_effC->SetDirectory(dir_effC);
  hprf_scint_phi_effC->SetDirectory(dir_effC);
  hprf_scint_z_effC->SetDirectory(dir_effC);
  hprf_scint_plane1_effC_end->SetDirectory(dir_effC);
  hprf_scint_plane2_effC_end->SetDirectory(dir_effC);

  m_outFile->cd();
  TDirectory* dir_time = m_outFile->mkdir("time");
  dir_time->cd();

  h_time_scint->SetDirectory(dir_time);
  hc_time_scint->SetDirectory(dir_time);

  h_time_scint_end->SetDirectory(dir_time);
  hc_time_scint_end->SetDirectory(dir_time);

  h_time_rpc->SetDirectory(dir_time);
  hc_time_rpc->SetDirectory(dir_time);

  gre_time_channel_rpc->Write("gre_time_channel_rpc");
  gre_time_channel_scint->Write("gre_time_channel_scint");
  gre_time_channel_scint_end->Write("gre_time_channel_scint_end");
  gr_timeShift_channel_rpc->Write("gr_timeShift_channel_rpc");
  gr_timeShift_channel_scint->Write("gr_timeShift_channel_scint");
  gr_timeShift_channel_scint_end->Write("gr_timeShift_channel_scint_end");

  B2INFO("Top file setup Done.");

  TDirectory* dir_time_F[2];
  TDirectory* dir_time_FS[2][8];
  TDirectory* dir_time_FSL[2][8][15];
  TDirectory* dir_time_FSLP[2][8][15][2];
  TDirectory* dir_time_F_end[2];
  TDirectory* dir_time_FS_end[2][4];
  TDirectory* dir_time_FSL_end[2][4][14];
  TDirectory* dir_time_FSLP_end[2][4][14][2];
  char dirname[50];
  B2INFO("Sub files declare Done.");
  for (int iF = 0; iF < 2; ++iF) {
    h_timeF_rpc[iF]->SetDirectory(dir_time);
    hc_timeF_rpc[iF]->SetDirectory(dir_time);

    h2_timeF_rpc[iF]->SetDirectory(dir_time);
    h2c_timeF_rpc[iF]->SetDirectory(dir_time);

    h_timeF_scint[iF]->SetDirectory(dir_time);
    hc_timeF_scint[iF]->SetDirectory(dir_time);

    h2_timeF_scint[iF]->SetDirectory(dir_time);
    h2c_timeF_scint[iF]->SetDirectory(dir_time);

    h_timeF_scint_end[iF]->SetDirectory(dir_time);
    hc_timeF_scint_end[iF]->SetDirectory(dir_time);

    h2_timeF_scint_end[iF]->SetDirectory(dir_time);
    h2c_timeF_scint_end[iF]->SetDirectory(dir_time);

    sprintf(dirname, "isForward_%d", iF);
    dir_time_F[iF] = dir_time->mkdir(dirname);
    dir_time_F[iF]->cd();

    for (int iS = 0; iS < 8; ++iS) {
      h_timeFS_rpc[iF][iS]->SetDirectory(dir_time_F[iF]);
      hc_timeFS_rpc[iF][iS]->SetDirectory(dir_time_F[iF]);

      h_timeFS_scint[iF][iS]->SetDirectory(dir_time_F[iF]);
      hc_timeFS_scint[iF][iS]->SetDirectory(dir_time_F[iF]);

      h2_timeFS[iF][iS]->SetDirectory(dir_time_F[iF]);
      h2c_timeFS[iF][iS]->SetDirectory(dir_time_F[iF]);

      sprintf(dirname, "Sector_%d", iS + 1);
      dir_time_FS[iF][iS] = dir_time_F[iF]->mkdir(dirname);
      dir_time_FS[iF][iS]->cd();

      for (int iL = 0; iL < 15; ++iL) {
        h_timeFSL[iF][iS][iL]->SetDirectory(dir_time_FS[iF][iS]);
        hc_timeFSL[iF][iS][iL]->SetDirectory(dir_time_FS[iF][iS]);

        sprintf(dirname, "Layer_%d", iL + 1);
        dir_time_FSL[iF][iS][iL] = dir_time_FS[iF][iS]->mkdir(dirname);
        dir_time_FSL[iF][iS][iL]->cd();
        for (int iP = 0; iP < 2; ++iP) {
          h_timeFSLP[iF][iS][iL][iP]->SetDirectory(dir_time_FSL[iF][iS][iL]);
          hc_timeFSLP[iF][iS][iL][iP]->SetDirectory(dir_time_FSL[iF][iS][iL]);
          h2_timeFSLP[iF][iS][iL][iP]->SetDirectory(dir_time_FSL[iF][iS][iL]);
          h2c_timeFSLP[iF][iS][iL][iP]->SetDirectory(dir_time_FSL[iF][iS][iL]);

          sprintf(dirname, "Plane_%d", iP);
          dir_time_FSLP[iF][iS][iL][iP] = dir_time_FSL[iF][iS][iL]->mkdir(dirname);
          dir_time_FSLP[iF][iS][iL][iP]->cd();

          int nchannel_max = BKLMElementNumbers::getNStrips(iF, iS + 1, iL + 1, iP);
          for (int iC = 0; iC < nchannel_max; ++iC) {
            h_timeFSLPC[iF][iS][iL][iP][iC]->SetDirectory(dir_time_FSLP[iF][iS][iL][iP]);
            hc_timeFSLPC[iF][iS][iL][iP][iC]->SetDirectory(dir_time_FSLP[iF][iS][iL][iP]);
            delete h_timeFSLPC_tc[iF][iS][iL][iP][iC];
          }
        }
      }
    }

    sprintf(dirname, "isForward_%d_end", iF + 1);
    dir_time_F_end[iF] = dir_time->mkdir(dirname);
    dir_time_F_end[iF]->cd();
    int maxLayer = 12 + 2 * iF;
    for (int iS = 0; iS < 4; ++iS) {
      h_timeFS_scint_end[iF][iS]->SetDirectory(dir_time_F_end[iF]);
      hc_timeFS_scint_end[iF][iS]->SetDirectory(dir_time_F_end[iF]);

      h2_timeFS_end[iF][iS]->SetDirectory(dir_time_F_end[iF]);
      h2c_timeFS_end[iF][iS]->SetDirectory(dir_time_F_end[iF]);

      sprintf(dirname, "Sector_%d_end", iS + 1);
      dir_time_FS_end[iF][iS] = dir_time_F_end[iF]->mkdir(dirname);
      dir_time_FS_end[iF][iS]->cd();
      for (int iL = 0; iL < maxLayer; ++iL) {
        h_timeFSL_end[iF][iS][iL]->SetDirectory(dir_time_FS_end[iF][iS]);
        hc_timeFSL_end[iF][iS][iL]->SetDirectory(dir_time_FS_end[iF][iS]);

        sprintf(dirname, "Layer_%d_end", iL + 1);
        dir_time_FSL_end[iF][iS][iL] = dir_time_FS_end[iF][iS]->mkdir(dirname);
        dir_time_FSL_end[iF][iS][iL]->cd();
        for (int iP = 0; iP < 2; ++iP) {
          h_timeFSLP_end[iF][iS][iL][iP]->SetDirectory(dir_time_FSL_end[iF][iS][iL]);
          hc_timeFSLP_end[iF][iS][iL][iP]->SetDirectory(dir_time_FSL_end[iF][iS][iL]);
          h2_timeFSLP_end[iF][iS][iL][iP]->SetDirectory(dir_time_FSL_end[iF][iS][iL]);
          h2c_timeFSLP_end[iF][iS][iL][iP]->SetDirectory(dir_time_FSL_end[iF][iS][iL]);

          sprintf(dirname, "plane_%d_end", iP);
          dir_time_FSLP_end[iF][iS][iL][iP] = dir_time_FSL_end[iF][iS][iL]->mkdir(dirname);
          dir_time_FSLP_end[iF][iS][iL][iP]->cd();

          for (int iC = 0; iC < 75; ++iC) {
            h_timeFSLPC_end[iF][iS][iL][iP][iC]->SetDirectory(dir_time_FSLP_end[iF][iS][iL][iP]);
            hc_timeFSLPC_end[iF][iS][iL][iP][iC]->SetDirectory(dir_time_FSLP_end[iF][iS][iL][iP]);
            delete h_timeFSLPC_tc_end[iF][iS][iL][iP][iC];
          }
        }
      }
    }
  }
  m_outFile->cd();
  m_outFile->Write();
  m_outFile->Close();
  B2INFO("File Write and Close. Done.");
}

double KLMTimeCalibrationAlgorithm::esti_timeShift(KLMChannelIndex& klmChannel)
{
  double tS = 0.0;
  int iSub = klmChannel.getSubdetector();
  int iF = klmChannel.getSection();
  int iS = klmChannel.getSector();
  int iL = klmChannel.getLayer();
  int iP = klmChannel.getPlane();
  int iC = klmChannel.getStrip();
  int totNStrips = EKLMElementNumbers::getMaximalStripNumber();
  if (iSub == KLMElementNumbers::c_BKLM) totNStrips = BKLMElementNumbers::getNStrips(iF, iS, iL, iP);
  if (iC == 1) {
    KLMChannelIndex kCIndex_upper(iSub, iF, iS, iL, iP, iC + 1);
    tS = tS_upperStrip(kCIndex_upper).second;
  } else if (iC == totNStrips) {
    KLMChannelIndex kCIndex_lower(iSub, iF, iS, iL, iP, iC - 1);
    tS = tS_lowerStrip(kCIndex_lower).second;
  } else {
    KLMChannelIndex kCIndex_upper(iSub, iF, iS, iL, iP, iC + 1);
    KLMChannelIndex kCIndex_lower(iSub, iF, iS, iL, iP, iC - 1);
    std::pair<int, double> tS_upper = tS_upperStrip(kCIndex_upper);
    std::pair<int, double> tS_lower = tS_lowerStrip(kCIndex_lower);
    unsigned int td_upper = tS_upper.first - iC;
    unsigned int td_lower = iC - tS_lower.first;
    unsigned int td = tS_upper.first - tS_lower.first;
    tS = (double(td_upper) * tS_lower.second + double(td_lower) * tS_upper.second) / double(td);
  }
  return tS;
}

std::pair<int, double> KLMTimeCalibrationAlgorithm::tS_upperStrip(KLMChannelIndex& klmChannel)
{
  std::pair<int, double> tS;
  int cId = klmChannel.getKLMChannelNumber();
  int iSub = klmChannel.getSubdetector();
  int iF = klmChannel.getSection();
  int iS = klmChannel.getSector();
  int iL = klmChannel.getLayer();
  int iP = klmChannel.getPlane();
  int iC = klmChannel.getStrip();
  int totNStrips = EKLMElementNumbers::getMaximalStripNumber();
  if (iSub == KLMElementNumbers::c_BKLM) totNStrips = BKLMElementNumbers::getNStrips(iF, iS, iL, iP);
  if (m_timeShift.find(cId) != m_timeShift.end()) {
    tS.first = iC;
    tS.second = m_timeShift[cId];
  } else if (iC == totNStrips) {
    tS.first = iC;
    tS.second = 0.0;
  } else {
    KLMChannelIndex kCIndex(iSub, iF, iS, iL, iP, iC + 1);
    tS = tS_upperStrip(kCIndex);
  }
  return tS;
}

std::pair<int, double> KLMTimeCalibrationAlgorithm::tS_lowerStrip(KLMChannelIndex& klmChannel)
{
  std::pair<int, double> tS;
  int cId = klmChannel.getKLMChannelNumber();
  int iSub = klmChannel.getSubdetector();
  int iF = klmChannel.getSection();
  int iS = klmChannel.getSector();
  int iL = klmChannel.getLayer();
  int iP = klmChannel.getPlane();
  int iC = klmChannel.getStrip();
  if (m_timeShift.find(cId) != m_timeShift.end()) {
    tS.first = iC;
    tS.second = m_timeShift[cId];
  } else if (iC == 1) {
    tS.first = iC;
    tS.second = 0.0;
  } else {
    KLMChannelIndex kCIndex(iSub, iF, iS, iL, iP, iC - 1);
    tS = tS_lowerStrip(kCIndex);
  }
  return tS;
}
