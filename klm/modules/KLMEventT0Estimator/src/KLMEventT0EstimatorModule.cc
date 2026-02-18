/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMEventT0Estimator/KLMEventT0EstimatorModule.h>

/* KLM headers. */
#include <klm/bklm/geometry/Module.h>
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/dataobjects/KLMHit2d.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/bklm/BKLMHit1d.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/KLMMuidLikelihood.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/eklm/geometry/TransformData.h>
#include <klm/dbobjects/KLMTimeConstants.h>
#include <klm/dbobjects/KLMTimeCableDelay.h>
#include <klm/dbobjects/KLMChannelStatus.h>
#include <klm/dbobjects/KLMEventT0HitResolution.h>

/* Basf2 headers. */
#include <mdst/dataobjects/Track.h>
#include <framework/dataobjects/EventExtraInfo.h>
#include <tracking/dataobjects/ExtHit.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/dataobjects/MCInitialParticles.h>

/* ROOT headers. */
#include <TDirectory.h>
#include <TF1.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TH2D.h>

/* C++ headers. */
#include <cmath>
#include <algorithm>
#include <vector>
#include <limits>
#include <string>
#include <utility>

using namespace Belle2;
using namespace Belle2::bklm;
using namespace Belle2::EKLM;

REG_MODULE(KLMEventT0Estimator);

/* Constructor and destructor. */

KLMEventT0EstimatorModule::KLMEventT0EstimatorModule() :
  HistoModule(),
  m_geoParB(nullptr),
  m_geoParE(nullptr),
  m_transformE(nullptr)
{
  setDescription("Estimate per-event T0 using KLM digits matched to extrapolated tracks (BKLM/EKLM scintillators and RPC) with per-event track/hit averages, uncertainties (SEM), final combined KLM value, and optional pull distributions.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("MuonListName", m_MuonListName,
           "Muon (or generic) ParticleList name used to access tracks and KLM relations (e.g. 'mu+:forT0').",
           std::string("mu+:forT0"));
  addParam("useCDCTemporaryT0", m_useCDCTemporaryT0,
           "Read CDC temporary EventT0 as a seed/diagnostic (not applied to the mean).",
           true);
  addParam("IgnoreBackwardPropagation", m_ignoreBackward,
           "Ignore backward-propagated ExtHits when forming entry/exit pairs.",
           false);
  addParam("histogramDirectoryName", m_histDirName,
           "Top directory for KLMEventT0Estimator histograms inside the ROOT file.",
           std::string("KLMEventT0Estimator"));
  addParam("histogramSubdirUncorrected", m_histSubdirUncorr,
           "Subdirectory name for uncorrected timing histograms.",
           std::string("uncorrected"));
  addParam("histogramSubdirCorrected", m_histSubdirCorr,
           "Subdirectory name for truth-corrected timing histograms.",
           std::string("truth_corrected"));
  addParam("FillTruthCorrectedTiming", m_fillTruthCorrectedTiming,
           "If true, also book/fill timing histograms with MC truth T0 subtracted (in 'truth_corrected').",
           true);
  addParam("FillSampleTypeFlag", m_fillSampleTypeFlag,
           "If true, book/fill a 'Data vs MC' histogram and a truth T0 monitor.",
           true);
  addParam("FillDimuonDeltaT0", m_fillDimuonDeltaT0,
           "If true, fill ΔT0 = T0(mu+) - T0(mu-) using per-track averages across all KLM categories.",
           false);

  // Steering parameters
  addParam("FinalAverageMode", m_FinalAverageMode,
           "Final KLM averaging mode: 'track' or 'hit'. Default: 'track'.",
           std::string("track"));
  addParam("FinalUseRPCInKLM", m_FinalUseRPCInKLM,
           "If true, include BKLM-RPC in the final KLM weighting. Default: false.",
           false);
  addParam("UseNewHitResolution", m_UseNewHitResolution,
           "If true, use calibrated per-hit resolution from payload. If false, use old per-event SEM calculation.",
           true);

  // ADC cut parameters
  addParam("ADCCut_BKLM_Scint_Min", m_ADCCut_BKLM_Scint_Min,
           "Minimum ADC charge cut for BKLM scintillator. Set to 0 to disable lower cut.",
           30.0);
  addParam("ADCCut_BKLM_Scint_Max", m_ADCCut_BKLM_Scint_Max,
           "Maximum ADC charge cut for BKLM scintillator. Set to large value to disable upper cut.",
           320.0);
  addParam("ADCCut_EKLM_Scint_Min", m_ADCCut_EKLM_Scint_Min,
           "Minimum ADC charge cut for EKLM scintillator. Set to 0 to disable lower cut.",
           40.0);
  addParam("ADCCut_EKLM_Scint_Max", m_ADCCut_EKLM_Scint_Max,
           "Maximum ADC charge cut for EKLM scintillator. Set to large value to disable upper cut.",
           350.0);

  // Pulls
  addParam("FillPulls", m_FillPulls,
           "If true, book and fill pull histograms.", true);
  addParam("PullsUseOppositeChargesOnly", m_PullsUseOppositeChargesOnly,
           "If true, only opposite-charge pairs form same-subdetector pulls.", true);
  addParam("PullsUseSameTrackCross", m_PullsUseSameTrackCross,
           "If true, cross-subdetector pulls are formed using the same track. If false, use event-level track averages.", false);
}

KLMEventT0EstimatorModule::~KLMEventT0EstimatorModule()
{
  delete m_transformE;
}

/* Histogram definition. */

void KLMEventT0EstimatorModule::defineHisto()
{
  // Parent directory for this module
  TDirectory* topdir = gDirectory->mkdir(m_histDirName.c_str());
  TDirectory::TContext ctxTop{gDirectory, topdir};

  // Subdirectory for uncorrected timing histograms (always created)
  TDirectory* d_unc = topdir->mkdir(m_histSubdirUncorr.c_str());

  // Subdirectory for truth-corrected histograms (only created if enabled)
  TDirectory* d_cor = nullptr;
  if (m_fillTruthCorrectedTiming) {
    d_cor = topdir->mkdir(m_histSubdirCorr.c_str());
  }

  auto H1 = [](const char* n, const char* t, int nb, double lo, double hi) {
    return new TH1D(n, t, nb, lo, hi);
  };

  /* Uncorrected timing histograms. */
  {
    TDirectory::TContext ctxUnc{gDirectory, d_unc};

    // --- per_track/ subdirectory ---
    TDirectory* d_per_track = d_unc->mkdir("per_track");
    {
      TDirectory::TContext ctxTrk{gDirectory, d_per_track};
      m_hT0Trk_BKLM_Scint = H1("h_t0trk_bklm_scint", "Per-track T0 (BKLM Scint);T0 [ns]", 800, -100, 100);
      m_hT0Trk_BKLM_RPC   = H1("h_t0trk_bklm_rpc",   "Per-track T0 (BKLM RPC);T0 [ns]",   800, -100, 100);
      m_hT0Trk_EKLM_Scint = H1("h_t0trk_eklm_scint", "Per-track T0 (EKLM Scint);T0 [ns]", 800, -100, 100);
    }

    // --- per_event/ subdirectory ---
    TDirectory* d_per_event = d_unc->mkdir("per_event");
    {
      TDirectory::TContext ctxEvt{gDirectory, d_per_event};

      // Track-average
      m_hT0Evt_TrkAvg_BKLM_Scint     = H1("h_t0evt_trkavg_bklm_scint",     "Per-event T0 (track-avg, BKLM Scint);T0 [ns]", 800, -100,
                                          100);
      m_hT0Evt_TrkAvg_BKLM_RPC       = H1("h_t0evt_trkavg_bklm_rpc",       "Per-event T0 (track-avg, BKLM RPC);T0 [ns]",   800, -100,
                                          100);
      m_hT0Evt_TrkAvg_EKLM_Scint     = H1("h_t0evt_trkavg_eklm_scint",     "Per-event T0 (track-avg, EKLM Scint);T0 [ns]", 800, -100,
                                          100);
      m_hT0Evt_TrkAvg_All            = H1("h_t0evt_trkavg_all",            "Per-event T0 (track-avg, all categories);T0 [ns]", 800, -100,
                                          100);
      m_hT0Evt_TrkAvg_BKLM_Scint_SEM = H1("h_t0evt_trkavg_bklm_scint_sem", "SEM (track-avg, BKLM Scint);SEM [ns]", 800, 0.0, 20.0);
      m_hT0Evt_TrkAvg_BKLM_RPC_SEM   = H1("h_t0evt_trkavg_bklm_rpc_sem",   "SEM (track-avg, BKLM RPC);SEM [ns]",   800, 0.0, 20.0);
      m_hT0Evt_TrkAvg_EKLM_Scint_SEM = H1("h_t0evt_trkavg_eklm_scint_sem", "SEM (track-avg, EKLM Scint);SEM [ns]", 800, 0.0, 20.0);
      m_hT0Evt_TrkAvg_All_SEM        = H1("h_t0evt_trkavg_all_sem",        "SEM (track-avg, all categories);SEM [ns]", 800, 0.0, 20.0);

      // Hit-average
      m_hT0Evt_HitAvg_BKLM_Scint     = H1("h_t0evt_hitavg_bklm_scint",     "Per-event T0 (hit-avg, BKLM Scint);T0 [ns]", 800, -100, 100);
      m_hT0Evt_HitAvg_BKLM_RPC       = H1("h_t0evt_hitavg_bklm_rpc",       "Per-event T0 (hit-avg, BKLM RPC);T0 [ns]",   800, -100, 100);
      m_hT0Evt_HitAvg_EKLM_Scint     = H1("h_t0evt_hitavg_eklm_scint",     "Per-event T0 (hit-avg, EKLM Scint);T0 [ns]", 800, -100, 100);
      m_hT0Evt_HitAvg_All            = H1("h_t0evt_hitavg_all",            "Per-event T0 (hit-avg, all categories);T0 [ns]", 800, -100,
                                          100);
      m_hT0Evt_HitAvg_BKLM_Scint_SEM = H1("h_t0evt_hitavg_bklm_scint_sem", "SEM (hit-avg, BKLM Scint);SEM [ns]", 800, 0.0, 20.0);
      m_hT0Evt_HitAvg_BKLM_RPC_SEM   = H1("h_t0evt_hitavg_bklm_rpc_sem",   "SEM (hit-avg, BKLM RPC);SEM [ns]",   800, 0.0, 20.0);
      m_hT0Evt_HitAvg_EKLM_Scint_SEM = H1("h_t0evt_hitavg_eklm_scint_sem", "SEM (hit-avg, EKLM Scint);SEM [ns]", 800, 0.0, 20.0);
      m_hT0Evt_HitAvg_All_SEM        = H1("h_t0evt_hitavg_all_sem",        "SEM (hit-avg, all categories);SEM [ns]", 800, 0.0, 20.0);

      // Final-source audit: all possible combinations of B(KLM Scint), E(KLM Scint), R(PC)
      m_hFinalSource = new TH1I("h_final_source", "Final KLM source;;events", 7, 0.5, 7.5);
      m_hFinalSource->GetXaxis()->SetBinLabel(1, "B only");
      m_hFinalSource->GetXaxis()->SetBinLabel(2, "E only");
      m_hFinalSource->GetXaxis()->SetBinLabel(3, "R only");
      m_hFinalSource->GetXaxis()->SetBinLabel(4, "B+E");
      m_hFinalSource->GetXaxis()->SetBinLabel(5, "B+R");
      m_hFinalSource->GetXaxis()->SetBinLabel(6, "E+R");
      m_hFinalSource->GetXaxis()->SetBinLabel(7, "B+E+R");
    }

    // --- diagnostics/ subdirectory ---
    TDirectory* d_diag = d_unc->mkdir("diagnostics");
    {
      TDirectory::TContext ctxDiag{gDirectory, d_diag};

      // Hit counting
      m_hNumKLM2DPerTrack     = new TH1I("h_nKLM2d_pertrk",        "N KLMHit2d per track;count", 20, 0, 20);
      m_hNumDigitsPerB1dRPC   = new TH1I("h_nDigits_perB1d_rpc",   "Number of KLMDigit per BKLMHit1d (RPC);digits",   15, 0, 15);
      m_hNumDigitsPerB1dScint = new TH1I("h_nDigits_perB1d_scint", "Number of KLMDigit per BKLMHit1d (scint);digits", 15, 0, 15);
      m_hNumDigitsPerE2dScint = new TH1I("h_nDigits_perE2d_scint", "Number of KLMDigit per EKLMHit2d (scint);digits", 15, 0, 15);

      // Digit charge
      m_hDigitCharge_BKLM_Scint = H1("h_digitQ_bklm_scint", "KLMDigit charge (BKLM Scint);ADC (a.u.)", 100, 0.0, 800.0);
      m_hDigitCharge_EKLM_Scint = H1("h_digitQ_eklm_scint", "KLMDigit charge (EKLM Scint);ADC (a.u.)", 100, 0.0, 800.0);

      // Per-track diagnostics for pull validation
      m_hNHits_PerTrack_BKLM_Scint = new TH1I("h_nhits_pertrk_bklm_scint", "Hits per track (BKLM Scint);N_{hits}", 50, 0, 50);
      m_hNHits_PerTrack_EKLM_Scint = new TH1I("h_nhits_pertrk_eklm_scint", "Hits per track (EKLM Scint);N_{hits}", 50, 0, 50);
      m_hSEM_PerTrack_BKLM_Scint = H1("h_sem_pertrk_bklm_scint", "SEM per track (BKLM Scint);SEM [ns]", 200, 0.0, 10.0);
      m_hSEM_PerTrack_EKLM_Scint = H1("h_sem_pertrk_eklm_scint", "SEM per track (EKLM Scint);SEM [ns]", 200, 0.0, 10.0);

      // Sample-type flag (Data vs MC)
      if (m_fillSampleTypeFlag) {
        m_hSampleType = new TH1I("h_sample_type", "Sample type;;events", 2, 0.5, 2.5);
        m_hSampleType->GetXaxis()->SetBinLabel(1, "Data");
        m_hSampleType->GetXaxis()->SetBinLabel(2, "MC");
      }

      // Per-digit timing components
      m_hTrec_BKLM_Scint   = H1("h_Trec_bklm_scint",   "T_{rec} (BKLM Scint);time [ns]", 800, -5000, -4000);
      m_hTcable_BKLM_Scint = H1("h_Tcable_bklm_scint", "T_{cable} (BKLM Scint);time [ns]", 800, -5000, -4000);
      m_hTprop_BKLM_Scint  = H1("h_Tprop_bklm_scint",  "T_{prop} (BKLM Scint);time [ns]", 800, -50, 50);
      m_hTfly_BKLM_Scint   = H1("h_Tfly_bklm_scint",   "T_{fly} (BKLM Scint);time [ns]", 800, -100, 100);

      m_hTrec_BKLM_RPC     = H1("h_Trec_bklm_rpc",     "T_{rec} (BKLM RPC);time [ns]", 800, -800, -500);
      m_hTcable_BKLM_RPC   = H1("h_Tcable_bklm_rpc",   "T_{cable} (BKLM RPC);time [ns]", 800, -800, -500);
      m_hTprop_BKLM_RPC    = H1("h_Tprop_bklm_rpc",    "T_{prop} (BKLM RPC);time [ns]", 800, -50, 50);
      m_hTfly_BKLM_RPC     = H1("h_Tfly_bklm_rpc",     "T_{fly} (BKLM RPC);time [ns]", 800, -100, 100);

      m_hTrec_EKLM_Scint   = H1("h_Trec_eklm_scint",   "T_{rec} (EKLM Scint);time [ns]", 800, -5000, -4000);
      m_hTcable_EKLM_Scint = H1("h_Tcable_eklm_scint", "T_{cable} (EKLM Scint);time [ns]", 800, -5000, -4000);
      m_hTprop_EKLM_Scint  = H1("h_Tprop_eklm_scint",  "T_{prop} (EKLM Scint);time [ns]", 800, -50, 50);
      m_hTfly_EKLM_Scint   = H1("h_Tfly_eklm_scint",   "T_{fly} (EKLM Scint);time [ns]", 800, -100, 100);

      // Optional dimuon ΔT0
      if (m_fillDimuonDeltaT0) {
        m_hDimuonDeltaT0   = H1("h_dimuon_delta_t0",          "Dimuon #DeltaT0 = T0(#mu^{+}) - T0(#mu^{-});#DeltaT0 [ns]", 800, -100, 100);
        m_hDimuonDeltaT0_B = H1("h_dimuon_delta_t0_bklm_scint", "Dimuon #DeltaT0 (BKLM Scint);#DeltaT0 [ns]", 800, -100, 100);
        m_hDimuonDeltaT0_R = H1("h_dimuon_delta_t0_bklm_rpc",  "Dimuon #DeltaT0 (BKLM RPC);#DeltaT0 [ns]",   800, -100, 100);
        m_hDimuonDeltaT0_E = H1("h_dimuon_delta_t0_eklm_scint", "Dimuon #DeltaT0 (EKLM Scint);#DeltaT0 [ns]", 800, -100, 100);

        // Combined category ΔT0 (matching final event T0 styles)
        m_hDimuonDeltaT0_ScintOnly = H1("h_dimuon_delta_t0_scint_only",
                                        "Dimuon #DeltaT0 (Scint Only: BKLM+EKLM per track);#DeltaT0 [ns]", 400, -50, 50);
        m_hDimuonDeltaT0_WithRPC = H1("h_dimuon_delta_t0_with_rpc",
                                      "Dimuon #DeltaT0 (With RPC: Scint+RPC per track);#DeltaT0 [ns]", 400, -50, 50);
        m_hDimuonDeltaT0_WithRPCDir = H1("h_dimuon_delta_t0_with_rpc_dir",
                                         "Dimuon #DeltaT0 (With RPC Dir: Scint+RPC#phi+RPCz per track);#DeltaT0 [ns]", 400, -50, 50);

        // T0 resolution summaries (filled in endRun)
        // Bins: 1=BKLM Scint, 2=BKLM RPC, 3=EKLM Scint, 4=ScintOnly, 5=WithRPC, 6=WithRPCDir
        // Per-track resolution: σ_track = σ(ΔT0)/√2
        m_hPerTrackT0Resolution = new TH1D("h_per_track_t0_resolution",
                                           "Per-Track T0 Resolution (#sigma_{#DeltaT0}/#sqrt{2});Category;Resolution [ns]",
                                           6, 0.5, 6.5);
        m_hPerTrackT0Resolution->GetXaxis()->SetBinLabel(1, "BKLM Scint");
        m_hPerTrackT0Resolution->GetXaxis()->SetBinLabel(2, "BKLM RPC");
        m_hPerTrackT0Resolution->GetXaxis()->SetBinLabel(3, "EKLM Scint");
        m_hPerTrackT0Resolution->GetXaxis()->SetBinLabel(4, "Scint Only");
        m_hPerTrackT0Resolution->GetXaxis()->SetBinLabel(5, "With RPC");
        m_hPerTrackT0Resolution->GetXaxis()->SetBinLabel(6, "With RPC Dir");

        // Event T0 resolution (combining 2 tracks): σ_event = σ(ΔT0)/2
        m_hEventT0Resolution = new TH1D("h_event_t0_resolution",
                                        "Event T0 Resolution (#sigma_{#DeltaT0}/2);Category;Resolution [ns]",
                                        6, 0.5, 6.5);
        m_hEventT0Resolution->GetXaxis()->SetBinLabel(1, "BKLM Scint");
        m_hEventT0Resolution->GetXaxis()->SetBinLabel(2, "BKLM RPC");
        m_hEventT0Resolution->GetXaxis()->SetBinLabel(3, "EKLM Scint");
        m_hEventT0Resolution->GetXaxis()->SetBinLabel(4, "Scint Only");
        m_hEventT0Resolution->GetXaxis()->SetBinLabel(5, "With RPC");
        m_hEventT0Resolution->GetXaxis()->SetBinLabel(6, "With RPC Dir");
      }
    } // end diagnostics/ directory

    // --- pulls/ subdirectory ---
    TDirectory* d_pulls = d_unc->mkdir("pulls");
    {
      TDirectory::TContext ctxPulls{gDirectory, d_pulls};

      // Pull histograms (uncorrected)
      if (m_FillPulls) {
        m_hPull_BKLM_Scint = H1("h_pull_bklm_scint",
                                "Pull (same-detector, BKLM Scint);(T0_{i}-T0_{j})/sqrt(SEM_{i}^{2}+SEM_{j}^{2})", 200, -10, 10);
        m_hPull_BKLM_RPC   = H1("h_pull_bklm_rpc",
                                "Pull (same-detector, BKLM RPC combined);(T0_{i}-T0_{j})/sqrt(SEM_{i}^{2}+SEM_{j}^{2})", 200, -10, 10);
        m_hPull_BKLM_RPC_Phi = H1("h_pull_bklm_rpc_phi",
                                  "Pull (same-detector, BKLM RPC Phi);(T0_{i}-T0_{j})/sqrt(SEM_{i}^{2}+SEM_{j}^{2})", 200, -10, 10);
        m_hPull_BKLM_RPC_Z   = H1("h_pull_bklm_rpc_z",
                                  "Pull (same-detector, BKLM RPC Z);(T0_{i}-T0_{j})/sqrt(SEM_{i}^{2}+SEM_{j}^{2})", 200, -10, 10);
        m_hPull_EKLM_Scint = H1("h_pull_eklm_scint",
                                "Pull (same-detector, EKLM Scint);(T0_{i}-T0_{j})/sqrt(SEM_{i}^{2}+SEM_{j}^{2})", 200, -10, 10);

        m_hPull_B_vs_E = H1("h_pull_bklm_scint_vs_eklm_scint",
                            "Pull (BKLM Scint vs EKLM Scint);(T0_{B}-T0_{E})/sqrt(SEM_{B}^{2}+SEM_{E}^{2})", 200, -10, 10);
        m_hPull_B_vs_R = H1("h_pull_bklm_scint_vs_bklm_rpc",
                            "Pull (BKLM Scint vs BKLM RPC);(T0_{B}-T0_{R})/sqrt(SEM_{B}^{2}+SEM_{R}^{2})", 200, -10, 10);
        m_hPull_E_vs_R = H1("h_pull_eklm_scint_vs_bklm_rpc",
                            "Pull (EKLM Scint vs BKLM RPC);(T0_{E}-T0_{R})/sqrt(SEM_{E}^{2}+SEM_{R}^{2})", 200, -10, 10);

        // Summary histograms: 4 categories for overall pull diagnostics
        // Categories: BKLM Scint, EKLM Scint, RPC Phi, RPC Z
        const int nPullCategories = 4;
        m_hPullSummary_Mean = H1("h_pull_summary_mean",
                                 "Pull Distribution Means (Gaussian);Category;#mu", nPullCategories, 0.5, nPullCategories + 0.5);
        m_hPullSummary_Width = H1("h_pull_summary_width",
                                  "Pull Distribution Widths (Gaussian #sigma);Category;#sigma", nPullCategories, 0.5, nPullCategories + 0.5);

        // Set bin labels for summary histograms
        const char* labels[nPullCategories] = {
          "BKLM Scint", "EKLM Scint", "RPC Phi", "RPC Z"
        };
        for (int i = 0; i < nPullCategories; ++i) {
          m_hPullSummary_Mean->GetXaxis()->SetBinLabel(i + 1, labels[i]);
          m_hPullSummary_Width->GetXaxis()->SetBinLabel(i + 1, labels[i]);
        }

        // --- sector/ subdirectory for pairwise sector pull analysis ---
        TDirectory* d_sector = d_pulls->mkdir("sector");
        {
          TDirectory::TContext ctxSector{gDirectory, d_sector};

          // BKLM Scintillator pairwise histograms (8x8)
          TDirectory* d_bscint = d_sector->mkdir("bklm_scint");
          {
            TDirectory::TContext ctxBScint{gDirectory, d_bscint};
            for (int i = 0; i < c_nBKLMSectors; ++i) {
              for (int j = 0; j < c_nBKLMSectors; ++j) {
                m_hPullPairwise_BScint[i][j] = H1(Form("h_pull_trk1_sec%d_vs_trk2_sec%d", i, j),
                                                  Form("BKLM Scint: Track1 Sector %d vs Track2 Sector %d;Pull", i, j), 200, -10, 10);
              }
            }
          }

          // EKLM Scintillator cross-endcap: Forward sector vs Backward sector (4x4)
          // For e+e- -> mu+mu- events: one muon goes forward, one goes backward
          TDirectory* d_escint_fwd_vs_bwd = d_sector->mkdir("eklm_scint_fwd_vs_bwd");
          {
            TDirectory::TContext ctxEScintFwdBwd{gDirectory, d_escint_fwd_vs_bwd};
            for (int i = 0; i < c_nEKLMSectors; ++i) {
              for (int j = 0; j < c_nEKLMSectors; ++j) {
                m_hPullPairwise_EScint_FwdVsBwd[i][j] = H1(Form("h_pull_fwd_sec%d_vs_bwd_sec%d", i + 1, j + 1),
                                                           Form("EKLM: Forward Sector %d vs Backward Sector %d;Pull", i + 1, j + 1), 200, -10, 10);
              }
            }
          }

          // RPC Phi pairwise histograms (8x8)
          TDirectory* d_rpc_phi = d_sector->mkdir("rpc_phi");
          {
            TDirectory::TContext ctxRPCPhi{gDirectory, d_rpc_phi};
            for (int i = 0; i < c_nBKLMSectors; ++i) {
              for (int j = 0; j < c_nBKLMSectors; ++j) {
                m_hPullPairwise_RPC_Phi[i][j] = H1(Form("h_pull_trk1_sec%d_vs_trk2_sec%d", i, j),
                                                   Form("RPC Phi: Track1 Sector %d vs Track2 Sector %d;Pull", i, j), 200, -10, 10);
              }
            }
          }

          // RPC Z pairwise histograms (8x8)
          TDirectory* d_rpc_z = d_sector->mkdir("rpc_z");
          {
            TDirectory::TContext ctxRPCZ{gDirectory, d_rpc_z};
            for (int i = 0; i < c_nBKLMSectors; ++i) {
              for (int j = 0; j < c_nBKLMSectors; ++j) {
                m_hPullPairwise_RPC_Z[i][j] = H1(Form("h_pull_trk1_sec%d_vs_trk2_sec%d", i, j),
                                                 Form("RPC Z: Track1 Sector %d vs Track2 Sector %d;Pull", i, j), 200, -10, 10);
              }
            }
          }

          // Summary 2D histograms (filled in endRun after Gaussian fits)
          // BKLM Scintillator
          m_h2PullSummary_BScint_Mean = new TH2D("h2_pull_summary_bklm_scint_mean",
                                                 "Pull Mean (BKLM Scint);Track 1 Sector;Track 2 Sector",
                                                 c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5,
                                                 c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5);
          m_h2PullSummary_BScint_Sigma = new TH2D("h2_pull_summary_bklm_scint_sigma",
                                                  "Pull Sigma (BKLM Scint);Track 1 Sector;Track 2 Sector",
                                                  c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5,
                                                  c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5);

          // EKLM Scintillator cross-endcap: Forward vs Backward
          m_h2PullSummary_EScint_FwdVsBwd_Mean = new TH2D("h2_pull_summary_eklm_scint_fwd_vs_bwd_mean",
                                                          "Pull Mean (EKLM Fwd vs Bwd);Forward Sector;Backward Sector",
                                                          c_nEKLMSectors, 0.5, c_nEKLMSectors + 0.5,
                                                          c_nEKLMSectors, 0.5, c_nEKLMSectors + 0.5);
          m_h2PullSummary_EScint_FwdVsBwd_Sigma = new TH2D("h2_pull_summary_eklm_scint_fwd_vs_bwd_sigma",
                                                           "Pull Sigma (EKLM Fwd vs Bwd);Forward Sector;Backward Sector",
                                                           c_nEKLMSectors, 0.5, c_nEKLMSectors + 0.5,
                                                           c_nEKLMSectors, 0.5, c_nEKLMSectors + 0.5);

          // RPC Phi
          m_h2PullSummary_RPC_Phi_Mean = new TH2D("h2_pull_summary_rpc_phi_mean",
                                                  "Pull Mean (RPC Phi);Track 1 Sector;Track 2 Sector",
                                                  c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5,
                                                  c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5);
          m_h2PullSummary_RPC_Phi_Sigma = new TH2D("h2_pull_summary_rpc_phi_sigma",
                                                   "Pull Sigma (RPC Phi);Track 1 Sector;Track 2 Sector",
                                                   c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5,
                                                   c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5);

          // RPC Z
          m_h2PullSummary_RPC_Z_Mean = new TH2D("h2_pull_summary_rpc_z_mean",
                                                "Pull Mean (RPC Z);Track 1 Sector;Track 2 Sector",
                                                c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5,
                                                c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5);
          m_h2PullSummary_RPC_Z_Sigma = new TH2D("h2_pull_summary_rpc_z_sigma",
                                                 "Pull Sigma (RPC Z);Track 1 Sector;Track 2 Sector",
                                                 c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5,
                                                 c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5);
        }
      } // end if (m_FillPulls)
    } // end pulls/ directory

    // --- residuals/ subdirectory (mirror of pulls/ but without normalization) ---
    TDirectory* d_residuals = d_unc->mkdir("residuals");
    {
      TDirectory::TContext ctxResiduals{gDirectory, d_residuals};

      if (m_FillPulls) {  // Reuse same flag to control residual booking
        // Same-subdetector residuals: ΔT0 = T0_i − T0_j (no SEM normalization)
        m_hResidual_BKLM_Scint = H1("h_residual_bklm_scint",
                                    "Residual (same-detector, BKLM Scint);#DeltaT0 = T0_{i} - T0_{j} [ns]", 200, -50, 50);
        m_hResidual_BKLM_RPC   = H1("h_residual_bklm_rpc",
                                    "Residual (same-detector, BKLM RPC combined);#DeltaT0 [ns]", 200, -50, 50);
        m_hResidual_BKLM_RPC_Phi = H1("h_residual_bklm_rpc_phi",
                                      "Residual (same-detector, BKLM RPC Phi);#DeltaT0 [ns]", 200, -50, 50);
        m_hResidual_BKLM_RPC_Z   = H1("h_residual_bklm_rpc_z",
                                      "Residual (same-detector, BKLM RPC Z);#DeltaT0 [ns]", 200, -50, 50);
        m_hResidual_EKLM_Scint = H1("h_residual_eklm_scint",
                                    "Residual (same-detector, EKLM Scint);#DeltaT0 [ns]", 200, -50, 50);

        // Cross-subdetector residuals
        m_hResidual_B_vs_E = H1("h_residual_bklm_scint_vs_eklm_scint",
                                "Residual (BKLM Scint vs EKLM Scint);#DeltaT0 = T0_{B} - T0_{E} [ns]", 200, -50, 50);
        m_hResidual_B_vs_R = H1("h_residual_bklm_scint_vs_bklm_rpc",
                                "Residual (BKLM Scint vs BKLM RPC);#DeltaT0 = T0_{B} - T0_{R} [ns]", 200, -50, 50);
        m_hResidual_E_vs_R = H1("h_residual_eklm_scint_vs_bklm_rpc",
                                "Residual (EKLM Scint vs BKLM RPC);#DeltaT0 = T0_{E} - T0_{R} [ns]", 200, -50, 50);

        // Summary histograms for residuals: 4 categories
        const int nResidualCategories = 4;
        m_hResidualSummary_Mean = H1("h_residual_summary_mean",
                                     "Residual Distribution Means (Gaussian);Category;#mu [ns]", nResidualCategories, 0.5, nResidualCategories + 0.5);
        m_hResidualSummary_Width = H1("h_residual_summary_width",
                                      "Residual Distribution Widths (Gaussian #sigma);Category;#sigma [ns]", nResidualCategories, 0.5, nResidualCategories + 0.5);

        const char* residualLabels[nResidualCategories] = {
          "BKLM Scint", "EKLM Scint", "RPC Phi", "RPC Z"
        };
        for (int i = 0; i < nResidualCategories; ++i) {
          m_hResidualSummary_Mean->GetXaxis()->SetBinLabel(i + 1, residualLabels[i]);
          m_hResidualSummary_Width->GetXaxis()->SetBinLabel(i + 1, residualLabels[i]);
        }

        // --- sector/ subdirectory for pairwise sector residual analysis ---
        TDirectory* d_sector_res = d_residuals->mkdir("sector");
        {
          TDirectory::TContext ctxSectorRes{gDirectory, d_sector_res};

          // BKLM Scintillator pairwise histograms (8x8)
          TDirectory* d_bscint_res = d_sector_res->mkdir("bklm_scint");
          {
            TDirectory::TContext ctxBScintRes{gDirectory, d_bscint_res};
            for (int i = 0; i < c_nBKLMSectors; ++i) {
              for (int j = 0; j < c_nBKLMSectors; ++j) {
                m_hResidualPairwise_BScint[i][j] = H1(Form("h_residual_trk1_sec%d_vs_trk2_sec%d", i, j),
                                                      Form("BKLM Scint: Track1 Sector %d vs Track2 Sector %d;#DeltaT0 [ns]", i, j), 200, -50, 50);
              }
            }
          }

          // EKLM Scintillator cross-endcap: Forward sector vs Backward sector (4x4)
          TDirectory* d_escint_fwd_vs_bwd_res = d_sector_res->mkdir("eklm_scint_fwd_vs_bwd");
          {
            TDirectory::TContext ctxEScintFwdBwdRes{gDirectory, d_escint_fwd_vs_bwd_res};
            for (int i = 0; i < c_nEKLMSectors; ++i) {
              for (int j = 0; j < c_nEKLMSectors; ++j) {
                m_hResidualPairwise_EScint_FwdVsBwd[i][j] = H1(Form("h_residual_fwd_sec%d_vs_bwd_sec%d", i + 1, j + 1),
                                                               Form("EKLM: Forward Sector %d vs Backward Sector %d;#DeltaT0 [ns]", i + 1, j + 1), 200, -50, 50);
              }
            }
          }

          // RPC Phi pairwise histograms (8x8)
          TDirectory* d_rpc_phi_res = d_sector_res->mkdir("rpc_phi");
          {
            TDirectory::TContext ctxRPCPhiRes{gDirectory, d_rpc_phi_res};
            for (int i = 0; i < c_nBKLMSectors; ++i) {
              for (int j = 0; j < c_nBKLMSectors; ++j) {
                m_hResidualPairwise_RPC_Phi[i][j] = H1(Form("h_residual_trk1_sec%d_vs_trk2_sec%d", i, j),
                                                       Form("RPC Phi: Track1 Sector %d vs Track2 Sector %d;#DeltaT0 [ns]", i, j), 200, -50, 50);
              }
            }
          }

          // RPC Z pairwise histograms (8x8)
          TDirectory* d_rpc_z_res = d_sector_res->mkdir("rpc_z");
          {
            TDirectory::TContext ctxRPCZRes{gDirectory, d_rpc_z_res};
            for (int i = 0; i < c_nBKLMSectors; ++i) {
              for (int j = 0; j < c_nBKLMSectors; ++j) {
                m_hResidualPairwise_RPC_Z[i][j] = H1(Form("h_residual_trk1_sec%d_vs_trk2_sec%d", i, j),
                                                     Form("RPC Z: Track1 Sector %d vs Track2 Sector %d;#DeltaT0 [ns]", i, j), 200, -50, 50);
              }
            }
          }

          // Summary 2D histograms for residuals (filled in endRun after Gaussian fits)
          // BKLM Scintillator
          m_h2ResidualSummary_BScint_Mean = new TH2D("h2_residual_summary_bklm_scint_mean",
                                                     "Residual Mean (BKLM Scint);Track 1 Sector;Track 2 Sector",
                                                     c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5,
                                                     c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5);
          m_h2ResidualSummary_BScint_Sigma = new TH2D("h2_residual_summary_bklm_scint_sigma",
                                                      "Residual Sigma (BKLM Scint);Track 1 Sector;Track 2 Sector",
                                                      c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5,
                                                      c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5);

          // EKLM Scintillator cross-endcap: Forward vs Backward
          m_h2ResidualSummary_EScint_FwdVsBwd_Mean = new TH2D("h2_residual_summary_eklm_scint_fwd_vs_bwd_mean",
                                                              "Residual Mean (EKLM Fwd vs Bwd);Forward Sector;Backward Sector",
                                                              c_nEKLMSectors, 0.5, c_nEKLMSectors + 0.5,
                                                              c_nEKLMSectors, 0.5, c_nEKLMSectors + 0.5);
          m_h2ResidualSummary_EScint_FwdVsBwd_Sigma = new TH2D("h2_residual_summary_eklm_scint_fwd_vs_bwd_sigma",
                                                               "Residual Sigma (EKLM Fwd vs Bwd);Forward Sector;Backward Sector",
                                                               c_nEKLMSectors, 0.5, c_nEKLMSectors + 0.5,
                                                               c_nEKLMSectors, 0.5, c_nEKLMSectors + 0.5);

          // RPC Phi
          m_h2ResidualSummary_RPC_Phi_Mean = new TH2D("h2_residual_summary_rpc_phi_mean",
                                                      "Residual Mean (RPC Phi);Track 1 Sector;Track 2 Sector",
                                                      c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5,
                                                      c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5);
          m_h2ResidualSummary_RPC_Phi_Sigma = new TH2D("h2_residual_summary_rpc_phi_sigma",
                                                       "Residual Sigma (RPC Phi);Track 1 Sector;Track 2 Sector",
                                                       c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5,
                                                       c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5);

          // RPC Z
          m_h2ResidualSummary_RPC_Z_Mean = new TH2D("h2_residual_summary_rpc_z_mean",
                                                    "Residual Mean (RPC Z);Track 1 Sector;Track 2 Sector",
                                                    c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5,
                                                    c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5);
          m_h2ResidualSummary_RPC_Z_Sigma = new TH2D("h2_residual_summary_rpc_z_sigma",
                                                     "Residual Sigma (RPC Z);Track 1 Sector;Track 2 Sector",
                                                     c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5,
                                                     c_nBKLMSectors, -0.5, c_nBKLMSectors - 0.5);
        } // end sector/ subdirectory
      } // end if (m_FillPulls)
    } // end residuals/ directory

    // --- cross_detector/ subdirectory for cross-detector ΔT0 analysis ---
    // Regions: 0=EKLM Backward, 1=EKLM Forward, 2=BKLM RPC, 3=BKLM Scint
    TDirectory* d_cross_det = d_unc->mkdir("cross_detector");
    {
      TDirectory::TContext ctxCrossDet{gDirectory, d_cross_det};

      if (m_FillPulls) {  // Reuse same flag
        const char* regionNames[c_nDetectorRegions] = {
          "EKLM_Bwd", "EKLM_Fwd", "BKLM_RPC", "BKLM_Scint"
        };
        const char* regionLabels[c_nDetectorRegions] = {
          "EKLM Backward", "EKLM Forward", "BKLM RPC", "BKLM Scint"
        };

        // 4x4 matrix of ΔT0 histograms for each region combination
        for (int i = 0; i < c_nDetectorRegions; ++i) {
          for (int j = 0; j < c_nDetectorRegions; ++j) {
            m_hDeltaT0_DetectorCombo[i][j] = H1(
                                               Form("h_deltaT0_%s_vs_%s", regionNames[i], regionNames[j]),
                                               Form("#DeltaT0: %s vs %s;#DeltaT0 = T0_{trk1} - T0_{trk2} [ns]", regionLabels[i], regionLabels[j]),
                                               200, -50, 50);
          }
        }

        // 2D summary histograms
        m_h2DeltaT0_DetectorCombo_Mean = new TH2D("h2_deltaT0_detector_combo_mean",
                                                  "#DeltaT0 Mean by Detector Region;Track 1 Region;Track 2 Region",
                                                  c_nDetectorRegions, -0.5, c_nDetectorRegions - 0.5,
                                                  c_nDetectorRegions, -0.5, c_nDetectorRegions - 0.5);
        m_h2DeltaT0_DetectorCombo_Sigma = new TH2D("h2_deltaT0_detector_combo_sigma",
                                                   "#DeltaT0 Sigma by Detector Region;Track 1 Region;Track 2 Region",
                                                   c_nDetectorRegions, -0.5, c_nDetectorRegions - 0.5,
                                                   c_nDetectorRegions, -0.5, c_nDetectorRegions - 0.5);
        m_h2DeltaT0_DetectorCombo_Entries = new TH2D("h2_deltaT0_detector_combo_entries",
                                                     "#DeltaT0 Entries by Detector Region;Track 1 Region;Track 2 Region",
                                                     c_nDetectorRegions, -0.5, c_nDetectorRegions - 0.5,
                                                     c_nDetectorRegions, -0.5, c_nDetectorRegions - 0.5);

        // Set bin labels for summary 2D histograms
        for (int i = 0; i < c_nDetectorRegions; ++i) {
          m_h2DeltaT0_DetectorCombo_Mean->GetXaxis()->SetBinLabel(i + 1, regionLabels[i]);
          m_h2DeltaT0_DetectorCombo_Mean->GetYaxis()->SetBinLabel(i + 1, regionLabels[i]);
          m_h2DeltaT0_DetectorCombo_Sigma->GetXaxis()->SetBinLabel(i + 1, regionLabels[i]);
          m_h2DeltaT0_DetectorCombo_Sigma->GetYaxis()->SetBinLabel(i + 1, regionLabels[i]);
          m_h2DeltaT0_DetectorCombo_Entries->GetXaxis()->SetBinLabel(i + 1, regionLabels[i]);
          m_h2DeltaT0_DetectorCombo_Entries->GetYaxis()->SetBinLabel(i + 1, regionLabels[i]);
        }
      } // end if (m_FillPulls)
    } // end cross_detector/ directory

    // --- final/ subdirectory ---
    TDirectory* d_final = d_unc->mkdir("final");
    {
      TDirectory::TContext ctxFinal{gDirectory, d_final};

      // Multiple Final EventT0 combinations (always booked)
      m_hT0Evt_Final_ScintOnly = H1("h_t0evt_final_scint_only",
                                    "Final KLM T0 (Scint only: BKLM+EKLM);T0 [ns]", 800, -100, 100);
      m_hT0Evt_Final_ScintOnly_SEM = H1("h_t0evt_final_scint_only_sem",
                                        "Final KLM T0 SEM (Scint only);SEM [ns]", 800, 0.0, 20.0);

      m_hT0Evt_Final_WithRPC = H1("h_t0evt_final_with_rpc",
                                  "Final KLM T0 (Scint + RPC combined);T0 [ns]", 800, -100, 100);
      m_hT0Evt_Final_WithRPC_SEM = H1("h_t0evt_final_with_rpc_sem",
                                      "Final KLM T0 SEM (Scint + RPC);SEM [ns]", 800, 0.0, 20.0);

      m_hT0Evt_Final_WithRPCDir = H1("h_t0evt_final_with_rpc_dir",
                                     "Final KLM T0 (Scint + RPC phi/z separate);T0 [ns]", 800, -100, 100);
      m_hT0Evt_Final_WithRPCDir_SEM = H1("h_t0evt_final_with_rpc_dir_sem",
                                         "Final KLM T0 SEM (Scint + RPC dir);SEM [ns]", 800, 0.0, 20.0);
    } // end final/ directory
  } // end uncorrected/ directory

  /* Truth-corrected timing histograms. */
  if (m_fillTruthCorrectedTiming) {
    TDirectory::TContext ctxCor{gDirectory, d_cor};

    if (m_fillSampleTypeFlag)
      m_hTruthT0 = H1("h_truth_t0", "Truth event T0 (MC);T0_{true} [ns]", 800, -100, 100);

    // Per-track T0 corrected
    m_hT0Trk_BKLM_Scint_corr = H1("h_t0trk_bklm_scint", "Per-track T0 (BKLM Scint, truth-corrected);T0 - T0_{true} [ns]", 800, -100,
                                  100);
    m_hT0Trk_BKLM_RPC_corr   = H1("h_t0trk_bklm_rpc",   "Per-track T0 (BKLM RPC, truth-corrected);T0 - T0_{true} [ns]",   800, -100,
                                  100);
    m_hT0Trk_EKLM_Scint_corr = H1("h_t0trk_eklm_scint", "Per-track T0 (EKLM Scint, truth-corrected);T0 - T0_{true} [ns]", 800, -100,
                                  100);

    // Per-event track-avg corrected means + mirrored SEM
    m_hT0Evt_TrkAvg_BKLM_Scint_corr     = H1("h_t0evt_trkavg_bklm_scint",
                                             "Per-event T0 (track-avg, BKLM Scint, corr);T0 - T0_{true} [ns]", 800, -100, 100);
    m_hT0Evt_TrkAvg_BKLM_RPC_corr       = H1("h_t0evt_trkavg_bklm_rpc",
                                             "Per-event T0 (track-avg, BKLM RPC, corr);T0 - T0_{true} [ns]",   800, -100, 100);
    m_hT0Evt_TrkAvg_EKLM_Scint_corr     = H1("h_t0evt_trkavg_eklm_scint",
                                             "Per-event T0 (track-avg, EKLM Scint, corr);T0 - T0_{true} [ns]", 800, -100, 100);
    m_hT0Evt_TrkAvg_All_corr            = H1("h_t0evt_trkavg_all",
                                             "Per-event T0 (track-avg, all categories, corr);T0 - T0_{true} [ns]", 800, -100, 100);
    m_hT0Evt_TrkAvg_BKLM_Scint_SEM_corr = H1("h_t0evt_trkavg_bklm_scint_sem", "SEM (track-avg, BKLM Scint, corr mirror);SEM [ns]", 800,
                                             0.0, 20.0);
    m_hT0Evt_TrkAvg_BKLM_RPC_SEM_corr   = H1("h_t0evt_trkavg_bklm_rpc_sem",   "SEM (track-avg, BKLM RPC, corr mirror);SEM [ns]",   800,
                                             0.0, 20.0);
    m_hT0Evt_TrkAvg_EKLM_Scint_SEM_corr = H1("h_t0evt_trkavg_eklm_scint_sem", "SEM (track-avg, EKLM Scint, corr mirror);SEM [ns]", 800,
                                             0.0, 20.0);
    m_hT0Evt_TrkAvg_All_SEM_corr        = H1("h_t0evt_trkavg_all_sem",        "SEM (track-avg, all categories, corr mirror);SEM [ns]",
                                             800, 0.0, 20.0);

    // Per-event hit-avg corrected means + mirrored SEM
    m_hT0Evt_HitAvg_BKLM_Scint_corr     = H1("h_t0evt_hitavg_bklm_scint",
                                             "Per-event T0 (hit-avg, BKLM Scint, corr);T0 - T0_{true} [ns]", 800, -100, 100);
    m_hT0Evt_HitAvg_BKLM_RPC_corr       = H1("h_t0evt_hitavg_bklm_rpc",
                                             "Per-event T0 (hit-avg, BKLM RPC, corr);T0 - T0_{true} [ns]",   800, -100, 100);
    m_hT0Evt_HitAvg_EKLM_Scint_corr     = H1("h_t0evt_hitavg_eklm_scint",
                                             "Per-event T0 (hit-avg, EKLM Scint, corr);T0 - T0_{true} [ns]", 800, -100, 100);
    m_hT0Evt_HitAvg_All_corr            = H1("h_t0evt_hitavg_all",
                                             "Per-event T0 (hit-avg, all categories, corr);T0 - T0_{true} [ns]", 800, -100, 100);
    m_hT0Evt_HitAvg_BKLM_Scint_SEM_corr = H1("h_t0evt_hitavg_bklm_scint_sem", "SEM (hit-avg, BKLM Scint, corr mirror);SEM [ns]", 800,
                                             0.0, 20.0);
    m_hT0Evt_HitAvg_BKLM_RPC_SEM_corr   = H1("h_t0evt_hitavg_bklm_rpc_sem",   "SEM (hit-avg, BKLM RPC, corr mirror);SEM [ns]",   800,
                                             0.0, 20.0);
    m_hT0Evt_HitAvg_EKLM_Scint_SEM_corr = H1("h_t0evt_hitavg_eklm_scint_sem", "SEM (hit-avg, EKLM Scint, corr mirror);SEM [ns]", 800,
                                             0.0, 20.0);
    m_hT0Evt_HitAvg_All_SEM_corr        = H1("h_t0evt_hitavg_all_sem",        "SEM (hit-avg, all categories, corr mirror);SEM [ns]",
                                             800, 0.0, 20.0);

    // Multiple Final EventT0 combinations (truth-corrected)
    m_hT0Evt_Final_ScintOnly_corr = H1("h_t0evt_final_scint_only_corr",
                                       "Final KLM T0 (Scint only, truth-corr);T0 - T0_{true} [ns]", 800, -100, 100);
    m_hT0Evt_Final_WithRPC_corr = H1("h_t0evt_final_with_rpc_corr",
                                     "Final KLM T0 (Scint + RPC, truth-corr);T0 - T0_{true} [ns]", 800, -100, 100);
    m_hT0Evt_Final_WithRPCDir_corr = H1("h_t0evt_final_with_rpc_dir_corr",
                                        "Final KLM T0 (Scint + RPC dir, truth-corr);T0 - T0_{true} [ns]", 800, -100, 100);

    // Per-digit timing components (mirrored)
    m_hTrec_BKLM_Scint_corr   = H1("h_Trec_bklm_scint",   "T_{rec} (BKLM Scint, MC mirror);time [ns]", 800, -5000, -4000);
    m_hTcable_BKLM_Scint_corr = H1("h_Tcable_bklm_scint", "T_{cable} (BKLM Scint, MC mirror);time [ns]", 800, -5000, -4000);
    m_hTprop_BKLM_Scint_corr  = H1("h_Tprop_bklm_scint",  "T_{prop} (BKLM Scint, MC mirror);time [ns]", 800, -50, 50);
    m_hTfly_BKLM_Scint_corr   = H1("h_Tfly_bklm_scint",   "T_{fly} (BKLM Scint, MC mirror);time [ns]", 800, -100, 100);

    m_hTrec_BKLM_RPC_corr     = H1("h_Trec_bklm_rpc",     "T_{rec} (BKLM RPC, MC mirror);time [ns]", 800, -800, -500);
    m_hTcable_BKLM_RPC_corr   = H1("h_Tcable_bklm_rpc",   "T_{cable} (BKLM RPC, MC mirror);time [ns]", 800, -800, -500);
    m_hTprop_BKLM_RPC_corr    = H1("h_Tprop_bklm_rpc",    "T_{prop} (BKLM RPC, MC mirror);time [ns]", 800, -50, 50);
    m_hTfly_BKLM_RPC_corr     = H1("h_Tfly_bklm_rpc",     "T_{fly} (BKLM RPC, MC mirror);time [ns]", 800, -100, 100);

    m_hTrec_EKLM_Scint_corr   = H1("h_Trec_eklm_scint",   "T_{rec} (EKLM Scint, MC mirror);time [ns]", 800, -5000, -4000);
    m_hTcable_EKLM_Scint_corr = H1("h_Tcable_eklm_scint", "T_{cable} (EKLM Scint, MC mirror);time [ns]", 800, -5000, -4000);
    m_hTprop_EKLM_Scint_corr  = H1("h_Tprop_eklm_scint",  "T_{prop} (EKLM Scint, MC mirror);time [ns]", 800, -50, 50);
    m_hTfly_EKLM_Scint_corr   = H1("h_Tfly_eklm_scint",   "T_{fly} (EKLM Scint, MC mirror);time [ns]", 800, -100, 100);
  }
}

/* Lifecycle. */

void KLMEventT0EstimatorModule::initialize()
{
  // Register that we define histograms
  REG_HISTOGRAM;

  // Inputs
  m_MuonList.isRequired(m_MuonListName);
  m_tracks.isRequired();

  // Geometry
  m_geoParB = bklm::GeometryPar::instance();
  m_geoParE = &(EKLM::GeometryData::Instance());
  m_transformE = new EKLM::TransformData(true, EKLM::TransformData::c_None);

  // Log ADC cut settings
  B2INFO("KLMEventT0Estimator: ADC cuts configured:"
         << LogVar("BKLM Scint min", m_ADCCut_BKLM_Scint_Min)
         << LogVar("BKLM Scint max", m_ADCCut_BKLM_Scint_Max)
         << LogVar("EKLM Scint min", m_ADCCut_EKLM_Scint_Min)
         << LogVar("EKLM Scint max", m_ADCCut_EKLM_Scint_Max));
}

void KLMEventT0EstimatorModule::beginRun()
{
  if (m_UseNewHitResolution) {
    if (!m_eventT0HitResolution.isValid())
      B2FATAL("KLMEventT0Estimator: KLM EventT0 hit resolution data are not available. "
              "Either provide the calibration or set UseNewHitResolution=False.");

    B2INFO("KLMEventT0Estimator: Using NEW calibrated per-hit resolution method."
           << LogVar("σ_RPC (ns)", m_eventT0HitResolution->getSigmaRPC())
           << LogVar("σ_BKLM_Scint (ns)", m_eventT0HitResolution->getSigmaBKLMScint())
           << LogVar("σ_EKLM_Scint (ns)", m_eventT0HitResolution->getSigmaEKLMScint()));
  } else {
    B2WARNING("KLMEventT0Estimator: Using OLD per-event SEM calculation (may be unreliable with few hits).");
  }
}

void KLMEventT0EstimatorModule::endRun()
{
  // Fill summary histograms using Gaussian fit ONLY for pull distributions
  if (!m_FillPulls) return;

  // Helper to fit Gaussian and extract mean/sigma (no RMS fallback)
  auto fillSummaryGaussian = [&](TH1D * h, int bin, const char* name) {
    if (!h || !m_hPullSummary_Mean || !m_hPullSummary_Width) return;
    if (h->GetEntries() < 50) {
      B2WARNING("KLMEventT0Estimator: Insufficient entries (" << (h ? h->GetEntries() : 0)
                << ") for " << name << " pull Gaussian fit - skipping.");
      return;
    }

    // Create Gaussian fit function with initial parameters from histogram
    TF1* gaus = new TF1("gaus_fit", "gaus", -5.0, 5.0);
    gaus->SetParameters(h->GetMaximum(), h->GetMean(), h->GetStdDev());
    gaus->SetParLimits(2, 0.1, 5.0);  // Sigma must be positive and reasonable

    // Fit with likelihood method (better for low statistics), quiet mode
    int fitStatus = h->Fit(gaus, "LQ", "", -4.0, 4.0);

    if (fitStatus == 0) {
      // Successful Gaussian fit
      const double mean = gaus->GetParameter(1);
      const double meanErr = gaus->GetParError(1);
      const double sigma = gaus->GetParameter(2);
      const double sigmaErr = gaus->GetParError(2);

      m_hPullSummary_Mean->SetBinContent(bin, mean);
      m_hPullSummary_Mean->SetBinError(bin, meanErr);
      m_hPullSummary_Width->SetBinContent(bin, sigma);
      m_hPullSummary_Width->SetBinError(bin, sigmaErr);

      B2INFO("KLMEventT0Estimator: " << name << " pull Gaussian fit: mean="
             << mean << " +/- " << meanErr << ", sigma=" << sigma << " +/- " << sigmaErr
             << " (N=" << h->GetEntries() << ")");
    } else {
      // Fit failed - do NOT use RMS fallback
      B2WARNING("KLMEventT0Estimator: Gaussian fit failed for " << name << " - bin not filled.");
    }

    delete gaus;
  };

  // 4 categories for overall pull diagnostics
  fillSummaryGaussian(m_hPull_BKLM_Scint, 1, "BKLM Scint");
  fillSummaryGaussian(m_hPull_EKLM_Scint, 2, "EKLM Scint");
  fillSummaryGaussian(m_hPull_BKLM_RPC_Phi, 3, "RPC Phi");
  fillSummaryGaussian(m_hPull_BKLM_RPC_Z, 4, "RPC Z");

  B2INFO("KLMEventT0Estimator: Pull summary (4 categories) filled with Gaussian fits at end of run.");

  // Helper to fit pairwise histogram and fill 2D summary
  auto fitPairwiseAndFill2D = [&](TH1D * h, TH2D * hMean, TH2D * hSigma, int binX, int binY, const char* name) {
    if (!h || !hMean || !hSigma) return;
    if (h->GetEntries() < 30) {
      B2DEBUG(20, "KLMEventT0Estimator: Insufficient entries (" << h->GetEntries()
              << ") for " << name << " Gaussian fit - skipping.");
      return;
    }

    TF1* gaus = new TF1("gaus_fit_pairwise", "gaus", -5.0, 5.0);
    gaus->SetParameters(h->GetMaximum(), h->GetMean(), h->GetStdDev());
    gaus->SetParLimits(2, 0.1, 5.0);

    int fitStatus = h->Fit(gaus, "LQ", "", -4.0, 4.0);

    if (fitStatus == 0) {
      const double mean = gaus->GetParameter(1);
      const double sigma = gaus->GetParameter(2);
      hMean->SetBinContent(binX, binY, mean);
      hSigma->SetBinContent(binX, binY, sigma);
      B2DEBUG(20, "KLMEventT0Estimator: " << name << " Gaussian fit: mean=" << mean
              << ", sigma=" << sigma << " (N=" << h->GetEntries() << ")");
    }
    delete gaus;
  };

  // Fill pairwise sector summary 2D histograms

  // BKLM Scintillator (8x8)
  if (m_h2PullSummary_BScint_Mean && m_h2PullSummary_BScint_Sigma) {
    for (int i = 0; i < c_nBKLMSectors; ++i) {
      for (int j = 0; j < c_nBKLMSectors; ++j) {
        fitPairwiseAndFill2D(m_hPullPairwise_BScint[i][j],
                             m_h2PullSummary_BScint_Mean, m_h2PullSummary_BScint_Sigma,
                             i + 1, j + 1, Form("BKLM Scint sector %d vs %d", i, j));
      }
    }
    B2INFO("KLMEventT0Estimator: BKLM Scint pairwise sector summary filled.");
  }

  // EKLM Scintillator cross-endcap: Forward vs Backward (4x4)
  if (m_h2PullSummary_EScint_FwdVsBwd_Mean && m_h2PullSummary_EScint_FwdVsBwd_Sigma) {
    for (int i = 0; i < c_nEKLMSectors; ++i) {
      for (int j = 0; j < c_nEKLMSectors; ++j) {
        fitPairwiseAndFill2D(m_hPullPairwise_EScint_FwdVsBwd[i][j],
                             m_h2PullSummary_EScint_FwdVsBwd_Mean, m_h2PullSummary_EScint_FwdVsBwd_Sigma,
                             i + 1, j + 1, Form("EKLM Fwd sec %d vs Bwd sec %d", i + 1, j + 1));
      }
    }
    B2INFO("KLMEventT0Estimator: EKLM Scint Forward vs Backward pairwise sector summary filled.");
  }

  // RPC Phi (8x8)
  if (m_h2PullSummary_RPC_Phi_Mean && m_h2PullSummary_RPC_Phi_Sigma) {
    for (int i = 0; i < c_nBKLMSectors; ++i) {
      for (int j = 0; j < c_nBKLMSectors; ++j) {
        fitPairwiseAndFill2D(m_hPullPairwise_RPC_Phi[i][j],
                             m_h2PullSummary_RPC_Phi_Mean, m_h2PullSummary_RPC_Phi_Sigma,
                             i + 1, j + 1, Form("RPC Phi sector %d vs %d", i, j));
      }
    }
    B2INFO("KLMEventT0Estimator: RPC Phi pairwise sector summary filled.");
  }

  // RPC Z (8x8)
  if (m_h2PullSummary_RPC_Z_Mean && m_h2PullSummary_RPC_Z_Sigma) {
    for (int i = 0; i < c_nBKLMSectors; ++i) {
      for (int j = 0; j < c_nBKLMSectors; ++j) {
        fitPairwiseAndFill2D(m_hPullPairwise_RPC_Z[i][j],
                             m_h2PullSummary_RPC_Z_Mean, m_h2PullSummary_RPC_Z_Sigma,
                             i + 1, j + 1, Form("RPC Z sector %d vs %d", i, j));
      }
    }
    B2INFO("KLMEventT0Estimator: RPC Z pairwise sector summary filled.");
  }

  // Fit dimuon ΔT0 histograms and fill T0 resolution summaries
  // Per-track resolution: σ_track = σ(ΔT0) / √2
  // Event T0 resolution (combining 2 tracks): σ_event = σ_track / √2 = σ(ΔT0) / 2
  if (m_hPerTrackT0Resolution || m_hEventT0Resolution) {
    auto fitDeltaT0AndFillResolution = [this](TH1D * h, int bin, const char* name) {
      if (!h || h->GetEntries() < 50) return;  // Need minimum stats for fit
      TF1* gaus = new TF1("gaus_res", "gaus", -50, 50);
      gaus->SetParameters(h->GetMaximum(), h->GetMean(), h->GetRMS());
      h->Fit(gaus, "QN0", "", h->GetMean() - 2 * h->GetRMS(), h->GetMean() + 2 * h->GetRMS());
      const double sigma = std::abs(gaus->GetParameter(2));
      const double sigmaErr = gaus->GetParError(2);
      const double perTrackRes = sigma / std::sqrt(2.0);      // σ(ΔT0)/√2
      const double eventT0Res = sigma / 2.0;                   // σ(ΔT0)/2

      if (m_hPerTrackT0Resolution) {
        m_hPerTrackT0Resolution->SetBinContent(bin, perTrackRes);
        m_hPerTrackT0Resolution->SetBinError(bin, sigmaErr / std::sqrt(2.0));
      }
      if (m_hEventT0Resolution) {
        m_hEventT0Resolution->SetBinContent(bin, eventT0Res);
        m_hEventT0Resolution->SetBinError(bin, sigmaErr / 2.0);
      }
      B2INFO("KLMEventT0Estimator: " << name << " ΔT0 σ=" << sigma
             << " ns, per-track res=" << perTrackRes << " ns, event T0 res=" << eventT0Res << " ns");
      delete gaus;
    };

    // Fit each category: 1=BKLM Scint, 2=BKLM RPC, 3=EKLM Scint, 4=ScintOnly, 5=WithRPC, 6=WithRPCDir
    fitDeltaT0AndFillResolution(m_hDimuonDeltaT0_B, 1, "BKLM Scint");
    fitDeltaT0AndFillResolution(m_hDimuonDeltaT0_R, 2, "BKLM RPC");
    fitDeltaT0AndFillResolution(m_hDimuonDeltaT0_E, 3, "EKLM Scint");
    fitDeltaT0AndFillResolution(m_hDimuonDeltaT0_ScintOnly, 4, "Scint Only");
    fitDeltaT0AndFillResolution(m_hDimuonDeltaT0_WithRPC, 5, "With RPC");
    fitDeltaT0AndFillResolution(m_hDimuonDeltaT0_WithRPCDir, 6, "With RPC Dir");

    B2INFO("KLMEventT0Estimator: T0 resolution summaries filled.");
  }

  // ---------------- Residual summary fitting ----------------
  // Helper to fit Gaussian to residual (wider range since in ns, not normalized)
  auto fillResidualSummaryGaussian = [&](TH1D * h, int bin, const char* name) {
    if (!h || !m_hResidualSummary_Mean || !m_hResidualSummary_Width) return;
    if (h->GetEntries() < 50) {
      B2WARNING("KLMEventT0Estimator: Insufficient entries (" << (h ? h->GetEntries() : 0)
                << ") for " << name << " residual Gaussian fit - skipping.");
      return;
    }

    TF1* gaus = new TF1("gaus_res_fit", "gaus", -30.0, 30.0);
    gaus->SetParameters(h->GetMaximum(), h->GetMean(), h->GetStdDev());
    gaus->SetParLimits(2, 0.1, 30.0);

    int fitStatus = h->Fit(gaus, "LQ", "", -25.0, 25.0);

    if (fitStatus == 0) {
      const double mean = gaus->GetParameter(1);
      const double meanErr = gaus->GetParError(1);
      const double sigma = gaus->GetParameter(2);
      const double sigmaErr = gaus->GetParError(2);

      m_hResidualSummary_Mean->SetBinContent(bin, mean);
      m_hResidualSummary_Mean->SetBinError(bin, meanErr);
      m_hResidualSummary_Width->SetBinContent(bin, sigma);
      m_hResidualSummary_Width->SetBinError(bin, sigmaErr);

      B2INFO("KLMEventT0Estimator: " << name << " residual Gaussian fit: mean="
             << mean << " +/- " << meanErr << " ns, sigma=" << sigma << " +/- " << sigmaErr
             << " ns (N=" << h->GetEntries() << ")");
    } else {
      B2WARNING("KLMEventT0Estimator: Gaussian fit failed for " << name << " residual - bin not filled.");
    }
    delete gaus;
  };

  // 4 categories for overall residual diagnostics
  fillResidualSummaryGaussian(m_hResidual_BKLM_Scint, 1, "BKLM Scint");
  fillResidualSummaryGaussian(m_hResidual_EKLM_Scint, 2, "EKLM Scint");
  fillResidualSummaryGaussian(m_hResidual_BKLM_RPC_Phi, 3, "RPC Phi");
  fillResidualSummaryGaussian(m_hResidual_BKLM_RPC_Z, 4, "RPC Z");

  B2INFO("KLMEventT0Estimator: Residual summary (4 categories) filled with Gaussian fits.");

  // Helper to fit pairwise residual histogram and fill 2D summary
  auto fitPairwiseResidualAndFill2D = [&](TH1D * h, TH2D * hMean, TH2D * hSigma, int binX, int binY, const char* name) {
    if (!h || !hMean || !hSigma) return;
    if (h->GetEntries() < 30) {
      B2DEBUG(20, "KLMEventT0Estimator: Insufficient entries (" << h->GetEntries()
              << ") for " << name << " residual Gaussian fit - skipping.");
      return;
    }

    TF1* gaus = new TF1("gaus_res_pairwise", "gaus", -30.0, 30.0);
    gaus->SetParameters(h->GetMaximum(), h->GetMean(), h->GetStdDev());
    gaus->SetParLimits(2, 0.1, 30.0);

    int fitStatus = h->Fit(gaus, "LQ", "", -25.0, 25.0);

    if (fitStatus == 0) {
      const double mean = gaus->GetParameter(1);
      const double sigma = gaus->GetParameter(2);
      hMean->SetBinContent(binX, binY, mean);
      hSigma->SetBinContent(binX, binY, sigma);
      B2DEBUG(20, "KLMEventT0Estimator: " << name << " residual fit: mean=" << mean
              << " ns, sigma=" << sigma << " ns (N=" << h->GetEntries() << ")");
    }
    delete gaus;
  };

  // Fill pairwise sector residual summary 2D histograms

  // BKLM Scintillator residuals (8x8)
  if (m_h2ResidualSummary_BScint_Mean && m_h2ResidualSummary_BScint_Sigma) {
    for (int i = 0; i < c_nBKLMSectors; ++i) {
      for (int j = 0; j < c_nBKLMSectors; ++j) {
        fitPairwiseResidualAndFill2D(m_hResidualPairwise_BScint[i][j],
                                     m_h2ResidualSummary_BScint_Mean, m_h2ResidualSummary_BScint_Sigma,
                                     i + 1, j + 1, Form("BKLM Scint sector %d vs %d", i, j));
      }
    }
    B2INFO("KLMEventT0Estimator: BKLM Scint pairwise sector residual summary filled.");
  }

  // EKLM Scintillator cross-endcap residuals (4x4)
  if (m_h2ResidualSummary_EScint_FwdVsBwd_Mean && m_h2ResidualSummary_EScint_FwdVsBwd_Sigma) {
    for (int i = 0; i < c_nEKLMSectors; ++i) {
      for (int j = 0; j < c_nEKLMSectors; ++j) {
        fitPairwiseResidualAndFill2D(m_hResidualPairwise_EScint_FwdVsBwd[i][j],
                                     m_h2ResidualSummary_EScint_FwdVsBwd_Mean, m_h2ResidualSummary_EScint_FwdVsBwd_Sigma,
                                     i + 1, j + 1, Form("EKLM Fwd sec %d vs Bwd sec %d residual", i + 1, j + 1));
      }
    }
    B2INFO("KLMEventT0Estimator: EKLM Fwd vs Bwd pairwise sector residual summary filled.");
  }

  // RPC Phi residuals (8x8)
  if (m_h2ResidualSummary_RPC_Phi_Mean && m_h2ResidualSummary_RPC_Phi_Sigma) {
    for (int i = 0; i < c_nBKLMSectors; ++i) {
      for (int j = 0; j < c_nBKLMSectors; ++j) {
        fitPairwiseResidualAndFill2D(m_hResidualPairwise_RPC_Phi[i][j],
                                     m_h2ResidualSummary_RPC_Phi_Mean, m_h2ResidualSummary_RPC_Phi_Sigma,
                                     i + 1, j + 1, Form("RPC Phi sector %d vs %d residual", i, j));
      }
    }
    B2INFO("KLMEventT0Estimator: RPC Phi pairwise sector residual summary filled.");
  }

  // RPC Z residuals (8x8)
  if (m_h2ResidualSummary_RPC_Z_Mean && m_h2ResidualSummary_RPC_Z_Sigma) {
    for (int i = 0; i < c_nBKLMSectors; ++i) {
      for (int j = 0; j < c_nBKLMSectors; ++j) {
        fitPairwiseResidualAndFill2D(m_hResidualPairwise_RPC_Z[i][j],
                                     m_h2ResidualSummary_RPC_Z_Mean, m_h2ResidualSummary_RPC_Z_Sigma,
                                     i + 1, j + 1, Form("RPC Z sector %d vs %d residual", i, j));
      }
    }
    B2INFO("KLMEventT0Estimator: RPC Z pairwise sector residual summary filled.");
  }

  // ---------------- Cross-detector ΔT0 summary fitting ----------------
  // Regions: 0=EKLM Backward, 1=EKLM Forward, 2=BKLM RPC, 3=BKLM Scint
  if (m_h2DeltaT0_DetectorCombo_Mean && m_h2DeltaT0_DetectorCombo_Sigma && m_h2DeltaT0_DetectorCombo_Entries) {
    const char* regionNames[c_nDetectorRegions] = {"EKLM Bwd", "EKLM Fwd", "BKLM RPC", "BKLM Scint"};

    for (int i = 0; i < c_nDetectorRegions; ++i) {
      for (int j = 0; j < c_nDetectorRegions; ++j) {
        TH1D* h = m_hDeltaT0_DetectorCombo[i][j];
        if (!h) continue;

        const int entries = static_cast<int>(h->GetEntries());
        m_h2DeltaT0_DetectorCombo_Entries->SetBinContent(i + 1, j + 1, entries);

        if (entries < 30) {
          B2DEBUG(20, "KLMEventT0Estimator: Insufficient entries (" << entries
                  << ") for " << regionNames[i] << " vs " << regionNames[j] << " ΔT0 fit - skipping.");
          continue;
        }

        TF1* gaus = new TF1("gaus_cross_det", "gaus", -30.0, 30.0);
        gaus->SetParameters(h->GetMaximum(), h->GetMean(), h->GetStdDev());
        gaus->SetParLimits(2, 0.1, 30.0);

        int fitStatus = h->Fit(gaus, "LQ", "", -25.0, 25.0);

        if (fitStatus == 0) {
          const double mean = gaus->GetParameter(1);
          const double sigma = gaus->GetParameter(2);
          m_h2DeltaT0_DetectorCombo_Mean->SetBinContent(i + 1, j + 1, mean);
          m_h2DeltaT0_DetectorCombo_Sigma->SetBinContent(i + 1, j + 1, sigma);
          B2INFO("KLMEventT0Estimator: Cross-detector ΔT0 (" << regionNames[i] << " vs " << regionNames[j]
                 << "): mean=" << mean << " ns, σ=" << sigma << " ns (N=" << entries << ")");
        }
        delete gaus;
      }
    }
    B2INFO("KLMEventT0Estimator: Cross-detector ΔT0 summary (4x4 matrix) filled.");
  }
}

void KLMEventT0EstimatorModule::terminate() {}

/* Helper methods. */

bool KLMEventT0EstimatorModule::passesADCCut(double charge, int subdetector, int layer, bool inRPC) const
{
  // RPC: No ADC cut applied
  if (subdetector == KLMElementNumbers::c_BKLM && (inRPC || layer >= BKLMElementNumbers::c_FirstRPCLayer)) {
    return true;
  }

  // BKLM Scintillator
  if (subdetector == KLMElementNumbers::c_BKLM) {
    return (charge >= m_ADCCut_BKLM_Scint_Min && charge <= m_ADCCut_BKLM_Scint_Max);
  }

  // EKLM Scintillator
  if (subdetector == KLMElementNumbers::c_EKLM) {
    return (charge >= m_ADCCut_EKLM_Scint_Min && charge <= m_ADCCut_EKLM_Scint_Max);
  }

  return true;
}

double KLMEventT0EstimatorModule::getHitSigma(int subdetector, int layer, bool inRPC, int plane) const
{
  if (!m_UseNewHitResolution) {
    // OLD METHOD: Return dummy value (uncertainty will be computed from scatter)
    return 1.0; // Placeholder, not actually used in old method
  }

  // NEW METHOD: Use calibrated payload
  if (!m_eventT0HitResolution.isValid()) {
    B2ERROR("KLMEventT0Estimator: Calibrated hit resolution payload not available!");
    return 1.0; // Fallback
  }

  if (subdetector == KLMElementNumbers::c_BKLM) {
    if (inRPC || layer >= BKLMElementNumbers::c_FirstRPCLayer) {
      // Use direction-specific RPC resolution if available (version 2+)
      // Fall back to combined RPC resolution for backward compatibility
      if (plane == BKLMElementNumbers::c_ZPlane) {
        float sigmaZ = m_eventT0HitResolution->getSigmaRPCZ();
        return (sigmaZ > 0.0) ? sigmaZ : m_eventT0HitResolution->getSigmaRPC();
      } else {
        float sigmaPhi = m_eventT0HitResolution->getSigmaRPCPhi();
        return (sigmaPhi > 0.0) ? sigmaPhi : m_eventT0HitResolution->getSigmaRPC();
      }
    } else {
      return m_eventT0HitResolution->getSigmaBKLMScint();
    }
  } else { // EKLM
    return m_eventT0HitResolution->getSigmaEKLMScint();
  }
}

KLMEventT0EstimatorModule::ExtPair
KLMEventT0EstimatorModule::matchExt(unsigned int key, ExtMap& v_ExtHits)
{
  ExtHit* entryHit = nullptr;
  ExtHit* exitHit  = nullptr;
  auto itlow = v_ExtHits.lower_bound(key);
  auto itup  = v_ExtHits.upper_bound(key);
  for (auto it = itlow; it != itup; ++it) {
    if (!entryHit || it->second.getTOF() < entryHit->getTOF()) entryHit = &(it->second);
    if (!exitHit  || it->second.getTOF() > exitHit->getTOF())  exitHit  = &(it->second);
  }
  return std::make_pair(entryHit, exitHit);
}

void KLMEventT0EstimatorModule::collectExtrapolatedHits(const Track* track,
                                                        ExtMap& scintMap,
                                                        ExtMap& rpcMap)
{
  scintMap.clear();
  rpcMap.clear();

  RelationVector<ExtHit> extHits = track->getRelationsTo<ExtHit>();
  KLMMuidLikelihood* muidLikelihood = track->getRelatedTo<KLMMuidLikelihood>();

  for (const ExtHit& eHit : extHits) {
    if (eHit.getStatus() != EXT_EXIT) continue;
    if (m_ignoreBackward && eHit.isBackwardPropagated()) continue;

    const bool isB = (eHit.getDetectorID() == Const::EDetector::BKLM);
    const bool isE = (eHit.getDetectorID() == Const::EDetector::EKLM);
    if (!isB && !isE) continue;

    int copyId = eHit.getCopyID();
    int tFor, tSec, tLay, tPla, tStr;
    int tSub = -1;

    if (isE) {
      tSub = KLMElementNumbers::c_EKLM;
      EKLMElementNumbers::Instance().stripNumberToElementNumbers(copyId,
                                                                 &tFor, &tLay, &tSec, &tPla, &tStr);
    }
    if (isB) {
      tSub = KLMElementNumbers::c_BKLM;
      BKLMElementNumbers::channelNumberToElementNumbers(copyId,
                                                        &tFor, &tSec, &tLay, &tPla, &tStr);
    }
    if (tSub < 0) continue;

    bool crossed = false;

    if (isB) {
      crossed = muidLikelihood
                ? muidLikelihood->isExtrapolatedBarrelLayerCrossed(tLay - 1)
                : true;
      if (!crossed) continue;

      // FIX 1: Use constant instead of hardcoded value
      const bool isRPC = (tLay >= BKLMElementNumbers::c_FirstRPCLayer);

      if (isRPC) {
        // RPC: match by module
        unsigned int moduleKey =
          m_elementNum->moduleNumber(tSub, tFor, tSec, tLay);
        rpcMap.insert(std::make_pair(moduleKey, eHit));
      } else {
        // BKLM scintillator: match by channel
        unsigned int channelKey =
          m_elementNum->channelNumber(tSub, tFor, tSec, tLay, tPla, tStr);
        if (m_channelStatus.isValid() &&
            m_channelStatus->getChannelStatus(channelKey) != KLMChannelStatus::c_Normal)
          continue;
        scintMap.insert(std::make_pair(channelKey, eHit));
      }
    }

    if (isE) {
      crossed = muidLikelihood
                ? muidLikelihood->isExtrapolatedEndcapLayerCrossed(tLay - 1)
                : true;
      if (!crossed) continue;

      unsigned int channelKey =
        m_elementNum->channelNumber(tSub, tFor, tSec, tLay, tPla, tStr);
      if (m_channelStatus.isValid() &&
          m_channelStatus->getChannelStatus(channelKey) != KLMChannelStatus::c_Normal)
        continue;
      scintMap.insert(std::make_pair(channelKey, eHit));
    }
  }
}

/* Per-subdetector accumulators. */

namespace {
  // OLD: Unweighted (for empirical variance calculation)
  inline void acc_stat_unweighted(double t, double& w, double& wt, double& wt2)
  {
    w += 1.0;
    wt += t;
    wt2 += t * t;
  }

  // NEW: Weighted accumulation using per-hit sigma
  inline void acc_stat_weighted(double t, double sigma, double& sumW, double& sumWT)
  {
    if (sigma <= 0.0 || !std::isfinite(sigma)) return;
    const double w = 1.0 / (sigma * sigma);
    sumW += w;
    sumWT += w * t;
  }
}

/* EKLM scintillator */
void KLMEventT0EstimatorModule::accumulateEKLM(const RelationVector<KLMHit2d>& klmHit2ds,
                                               const ExtMap& scintMap,
                                               double& sumW, double& sumWT, double& sumWT2,
                                               double& sumW_new, double& sumWT_new,
                                               int& nHits, double& sumPropDist, double& sumLayer,
                                               int& sumSection, int& sumSector)
{
  DBObjPtr<KLMTimeConstants> timeConstants;
  DBObjPtr<KLMTimeCableDelay> timeCableDelay;

  const double delayScint = timeConstants.isValid()
                            ? timeConstants->getDelay(KLMTimeConstants::c_EKLM)
                            : 0.0;

  HepGeom::Point3D<double> hitGlobal_ext, hitLocal_ext;

  for (const KLMHit2d& hit2d : klmHit2ds) {
    if (hit2d.getSubdetector() != KLMElementNumbers::c_EKLM) continue;

    RelationVector<KLMDigit> digits = hit2d.getRelationsTo<KLMDigit>();
    if (digits.size() == 0) continue;

    int nGoodDigits = 0;

    for (const KLMDigit& d : digits) {
      if (!d.isGood()) continue;

      unsigned int cid = d.getUniqueChannelID();
      if (m_channelStatus.isValid() &&
          m_channelStatus->getChannelStatus(cid) != KLMChannelStatus::c_Normal) continue;

      ++nGoodDigits;

      // Fill ADC histogram BEFORE cut
      if (m_hDigitCharge_EKLM_Scint) m_hDigitCharge_EKLM_Scint->Fill(d.getCharge());

      // Apply ADC cut
      if (!passesADCCut(d.getCharge(), KLMElementNumbers::c_EKLM, d.getLayer(), false)) {
        continue;
      }

      // Match using channel ID
      ExtPair ex = const_cast<KLMEventT0EstimatorModule*>(this)->matchExt(cid, const_cast<ExtMap&>(scintMap));
      if (!ex.first || !ex.second) continue;
      const double flyTime = 0.5 * (ex.first->getTOF() + ex.second->getTOF());

      // Distance along strip to readout
      const ROOT::Math::XYZVector posGlobExt = 0.5 * (ex.first->getPosition() + ex.second->getPosition());
      hitGlobal_ext.setX(posGlobExt.X() / Unit::mm * CLHEP::mm);
      hitGlobal_ext.setY(posGlobExt.Y() / Unit::mm * CLHEP::mm);
      hitGlobal_ext.setZ(posGlobExt.Z() / Unit::mm * CLHEP::mm);

      const double Lmm = m_geoParE->getStripLength(d.getStrip()) / CLHEP::mm * Unit::mm;
      const HepGeom::Transform3D* tr = m_transformE->getStripGlobalToLocal(const_cast<KLMDigit*>(&d));
      hitLocal_ext = (*tr) * hitGlobal_ext;
      const double dist_mm = 0.5 * Lmm - hitLocal_ext.x() / CLHEP::mm * Unit::mm;

      // Components
      const double Trec = d.getTime();
      const double Tcable = timeCableDelay.isValid() ? timeCableDelay->getTimeDelay(cid) : 0.0;
      const double Tprop = dist_mm * delayScint;
      const double Tfly = flyTime;

      // Fill component histograms
      if (m_hTrec_EKLM_Scint)   m_hTrec_EKLM_Scint->Fill(Trec);
      if (m_hTcable_EKLM_Scint) m_hTcable_EKLM_Scint->Fill(Tcable);
      if (m_hTprop_EKLM_Scint)  m_hTprop_EKLM_Scint->Fill(Tprop);
      if (m_hTfly_EKLM_Scint)   m_hTfly_EKLM_Scint->Fill(Tfly);
      if (m_fillTruthCorrectedTiming && std::isfinite(m_truthT0)) {
        if (m_hTrec_EKLM_Scint_corr)   m_hTrec_EKLM_Scint_corr->Fill(Trec);
        if (m_hTcable_EKLM_Scint_corr) m_hTcable_EKLM_Scint_corr->Fill(Tcable);
        if (m_hTprop_EKLM_Scint_corr)  m_hTprop_EKLM_Scint_corr->Fill(Tprop);
        if (m_hTfly_EKLM_Scint_corr)   m_hTfly_EKLM_Scint_corr->Fill(Tfly);
      }

      // Correct digit time
      double t = Trec;
      if (timeCableDelay.isValid()) t -= Tcable;
      t -= Tprop;

      const double t0_est = t - Tfly;
      if (!std::isfinite(t0_est)) continue;

      // OLD METHOD: Unweighted accumulation
      acc_stat_unweighted(t0_est, sumW, sumWT, sumWT2);

      // NEW METHOD: Weighted accumulation using calibrated sigma
      const double sigma = getHitSigma(KLMElementNumbers::c_EKLM, d.getLayer(), false);
      acc_stat_weighted(t0_est, sigma, sumW_new, sumWT_new);

      // Diagnostic accumulators
      ++nHits;
      sumPropDist += std::fabs(dist_mm);  // dist_mm is propagation distance in mm
      sumLayer += d.getLayer();
      // EKLM section: 1=backward (z<0), 2=forward (z>0)
      // Convert to 0/1 for consistency with BKLM: 0=backward, 1=forward
      sumSection += (d.getSection() == 2) ? 1 : 0;
      sumSector += d.getSector();  // EKLM sectors: 1-4
    }

    if (m_hNumDigitsPerE2dScint) m_hNumDigitsPerE2dScint->Fill(nGoodDigits);
  }
}

/* BKLM scintillator */
void KLMEventT0EstimatorModule::accumulateBKLMScint(RelationVector<KLMHit2d>& klmHit2ds,
                                                    const ExtMap& scintMap,
                                                    double& sumW, double& sumWT, double& sumWT2,
                                                    double& sumW_new, double& sumWT_new,
                                                    int& nHits, double& sumPropDist, double& sumLayer,
                                                    int& sumSection, int& sumSector)
{
  DBObjPtr<KLMTimeConstants> timeConstants;
  DBObjPtr<KLMTimeCableDelay> timeCableDelay;

  const double delayScint = timeConstants.isValid()
                            ? timeConstants->getDelay(KLMTimeConstants::c_BKLM)
                            : 0.0;

  for (KLMHit2d& hit2d : klmHit2ds) {
    if (hit2d.getSubdetector() != KLMElementNumbers::c_BKLM) continue;
    if (hit2d.inRPC()) continue;
    if (hit2d.isOutOfTime()) continue;

    RelationVector<BKLMHit1d> b1ds = hit2d.getRelationsTo<BKLMHit1d>();
    if (b1ds.size() == 0) continue;

    const bklm::Module* mod = m_geoParB->findModule(hit2d.getSection(), hit2d.getSector(), hit2d.getLayer());
    const ROOT::Math::XYZVector posG2d = hit2d.getPosition();

    for (const BKLMHit1d& h1d : b1ds) {
      RelationVector<KLMDigit> digits = h1d.getRelationsTo<KLMDigit>();

      int nGoodDigitsForThisB1d = 0;

      for (const KLMDigit& d : digits) {
        if (d.inRPC() || !d.isGood()) continue;

        unsigned int cid = d.getUniqueChannelID();
        if (m_channelStatus.isValid() &&
            m_channelStatus->getChannelStatus(cid) != KLMChannelStatus::c_Normal) continue;

        // Fill ADC histogram BEFORE cut
        if (m_hDigitCharge_BKLM_Scint) m_hDigitCharge_BKLM_Scint->Fill(d.getCharge());

        ++nGoodDigitsForThisB1d;

        // Apply ADC cut
        if (!passesADCCut(d.getCharge(), KLMElementNumbers::c_BKLM, d.getLayer(), false)) {
          continue;
        }

        // Match using channel ID
        ExtPair p = const_cast<KLMEventT0EstimatorModule*>(this)->matchExt(cid, const_cast<ExtMap&>(scintMap));
        if (!p.first || !p.second) continue;

        const double flyTime = 0.5 * (p.first->getTOF() + p.second->getTOF());
        const ROOT::Math::XYZVector posGext = 0.5 * (p.first->getPosition() + p.second->getPosition());

        // Gate in local
        const CLHEP::Hep3Vector locExt  = mod->globalToLocal(CLHEP::Hep3Vector(posGext.X(), posGext.Y(), posGext.Z()), true);
        const CLHEP::Hep3Vector locHit2 = mod->globalToLocal(CLHEP::Hep3Vector(posG2d.X(),  posG2d.Y(),  posG2d.Z()),  true);
        const CLHEP::Hep3Vector diff    = locExt - locHit2;
        if (std::fabs(diff.z()) > mod->getZStripWidth() || std::fabs(diff.y()) > mod->getPhiStripWidth()) continue;

        // Prop distance
        const bool isPhiReadout = h1d.isPhiReadout();
        double propaLen = mod->getPropagationDistance(locExt, d.getStrip(), isPhiReadout);

        // Components
        const double Trec = d.getTime();
        const double Tcable = timeCableDelay.isValid() ? timeCableDelay->getTimeDelay(cid) : 0.0;
        const double Tprop = propaLen * delayScint;
        const double Tfly = flyTime;

        if (m_hTrec_BKLM_Scint)   m_hTrec_BKLM_Scint->Fill(Trec);
        if (m_hTcable_BKLM_Scint) m_hTcable_BKLM_Scint->Fill(Tcable);
        if (m_hTprop_BKLM_Scint)  m_hTprop_BKLM_Scint->Fill(Tprop);
        if (m_hTfly_BKLM_Scint)   m_hTfly_BKLM_Scint->Fill(Tfly);
        if (m_fillTruthCorrectedTiming && std::isfinite(m_truthT0)) {
          if (m_hTrec_BKLM_Scint_corr)   m_hTrec_BKLM_Scint_corr->Fill(Trec);
          if (m_hTcable_BKLM_Scint_corr) m_hTcable_BKLM_Scint_corr->Fill(Tcable);
          if (m_hTprop_BKLM_Scint_corr)  m_hTprop_BKLM_Scint_corr->Fill(Tprop);
          if (m_hTfly_BKLM_Scint_corr)   m_hTfly_BKLM_Scint_corr->Fill(Tfly);
        }

        double t = Trec;
        if (timeCableDelay.isValid()) t -= Tcable;
        t -= Tprop;

        const double t0_est = t - Tfly;
        if (!std::isfinite(t0_est)) continue;

        // OLD METHOD: Unweighted accumulation
        acc_stat_unweighted(t0_est, sumW, sumWT, sumWT2);

        // NEW METHOD: Weighted accumulation using calibrated sigma
        const double sigma = getHitSigma(KLMElementNumbers::c_BKLM, d.getLayer(), false);
        acc_stat_weighted(t0_est, sigma, sumW_new, sumWT_new);

        // Diagnostic accumulators
        ++nHits;
        sumPropDist += std::fabs(propaLen);  // propaLen is propagation distance in cm
        sumLayer += d.getLayer();
        sumSection += hit2d.getSection();  // 0=backward, 1=forward
        sumSector += hit2d.getSector();    // 1-8
      }

      if (m_hNumDigitsPerB1dScint) m_hNumDigitsPerB1dScint->Fill(nGoodDigitsForThisB1d);
    }
  }
}

/* BKLM RPC */
// Accumulate BKLM RPC hits with a specific readout plane filter
void KLMEventT0EstimatorModule::accumulateBKLMRPCFiltered(RelationVector<KLMHit2d>& klmHit2ds,
                                                          const ExtMap& rpcMap,
                                                          bool acceptPhi,  // true = accept phi hits, false = accept z hits
                                                          double& sumW, double& sumWT, double& sumWT2,
                                                          double& sumW_new, double& sumWT_new,
                                                          int& nHits, double& sumPropDist, double& sumLayer,
                                                          int& sumSection, int& sumSector)
{
  DBObjPtr<KLMTimeConstants> timeConstants;
  DBObjPtr<KLMTimeCableDelay> timeCableDelay;

  const double delayPhi = timeConstants.isValid()
                          ? timeConstants->getDelay(KLMTimeConstants::c_RPCPhi)
                          : 0.0;
  const double delayZ   = timeConstants.isValid()
                          ? timeConstants->getDelay(KLMTimeConstants::c_RPCZ)
                          : 0.0;

  for (KLMHit2d& hit2d : klmHit2ds) {
    if (hit2d.getSubdetector() != KLMElementNumbers::c_BKLM) continue;
    if (!hit2d.inRPC()) continue;
    if (hit2d.isOutOfTime()) continue;

    RelationVector<BKLMHit1d> b1ds = hit2d.getRelationsTo<BKLMHit1d>();
    if (b1ds.size() == 0) continue;

    const bklm::Module* mod = m_geoParB->findModule(hit2d.getSection(), hit2d.getSector(), hit2d.getLayer());
    const ROOT::Math::XYZVector posG2d = hit2d.getPosition();

    for (const BKLMHit1d& h1d : b1ds) {
      // Filter by readout direction if specified
      const bool isPhi = h1d.isPhiReadout();
      if (acceptPhi && !isPhi) continue;  // Want phi but this is z
      if (!acceptPhi && isPhi) continue;  // Want z but this is phi

      RelationVector<KLMDigit> digits = h1d.getRelationsTo<KLMDigit>();

      int nGoodDigitsForThisB1d = 0;

      for (const KLMDigit& d : digits) {
        // *** FIX 3: CRITICAL - Skip non-RPC digits! ***
        if (!d.inRPC()) continue;

        unsigned int cid = d.getUniqueChannelID();
        if (m_channelStatus.isValid() &&
            m_channelStatus->getChannelStatus(cid) != KLMChannelStatus::c_Normal) continue;

        if (!d.isGood()) continue;
        ++nGoodDigitsForThisB1d;

        // NOTE: No ADC cut for RPC

        // RPC matched by module key
        unsigned int moduleKey = m_elementNum->moduleNumber(d.getSubdetector(), d.getSection(), d.getSector(), d.getLayer());
        ExtPair p = const_cast<KLMEventT0EstimatorModule*>(this)->matchExt(moduleKey, const_cast<ExtMap&>(rpcMap));
        if (!p.first || !p.second) continue;

        const double flyTime = 0.5 * (p.first->getTOF() + p.second->getTOF());
        const ROOT::Math::XYZVector posGext = 0.5 * (p.first->getPosition() + p.second->getPosition());

        const CLHEP::Hep3Vector locExt  = mod->globalToLocal(CLHEP::Hep3Vector(posGext.X(), posGext.Y(), posGext.Z()), true);
        const CLHEP::Hep3Vector locHit2 = mod->globalToLocal(CLHEP::Hep3Vector(posG2d.X(),  posG2d.Y(),  posG2d.Z()),  true);

        const CLHEP::Hep3Vector diff = locExt - locHit2;
        if (std::fabs(diff.z()) > mod->getZStripWidth() || std::fabs(diff.y()) > mod->getPhiStripWidth()) continue;

        // FIX: Use getPropagationDistance() (not getPropagationTimes()) to avoid double speed conversion
        const CLHEP::Hep3Vector propaV = mod->getPropagationDistance(locExt);
        const double propaDist = isPhi ? propaV.y() : propaV.z();  // Distance in cm

        // Components
        const double Trec = d.getTime();
        const double Tcable = timeCableDelay.isValid() ? timeCableDelay->getTimeDelay(cid) : 0.0;
        const double Tprop = propaDist * (isPhi ? delayPhi : delayZ);  // delay is in ns/cm
        const double Tfly = flyTime;

        if (m_hTrec_BKLM_RPC)   m_hTrec_BKLM_RPC->Fill(Trec);
        if (m_hTcable_BKLM_RPC) m_hTcable_BKLM_RPC->Fill(Tcable);
        if (m_hTprop_BKLM_RPC)  m_hTprop_BKLM_RPC->Fill(Tprop);
        if (m_hTfly_BKLM_RPC)   m_hTfly_BKLM_RPC->Fill(Tfly);
        if (m_fillTruthCorrectedTiming && std::isfinite(m_truthT0)) {
          if (m_hTrec_BKLM_RPC_corr)   m_hTrec_BKLM_RPC_corr->Fill(Trec);
          if (m_hTcable_BKLM_RPC_corr) m_hTcable_BKLM_RPC_corr->Fill(Tcable);
          if (m_hTprop_BKLM_RPC_corr)  m_hTprop_BKLM_RPC_corr->Fill(Tprop);
          if (m_hTfly_BKLM_RPC_corr)   m_hTfly_BKLM_RPC_corr->Fill(Tfly);
        }

        double t = Trec;
        if (timeCableDelay.isValid()) t -= Tcable;
        t -= Tprop;

        const double t0_est = t - Tfly;
        if (!std::isfinite(t0_est)) continue;

        // OLD METHOD: Unweighted accumulation
        acc_stat_unweighted(t0_est, sumW, sumWT, sumWT2);

        // NEW METHOD: Weighted accumulation using calibrated sigma with direction-specific resolution
        const int plane = isPhi ? BKLMElementNumbers::c_PhiPlane : BKLMElementNumbers::c_ZPlane;
        const double sigma = getHitSigma(KLMElementNumbers::c_BKLM, d.getLayer(), true, plane);
        acc_stat_weighted(t0_est, sigma, sumW_new, sumWT_new);

        // Diagnostic accumulators
        ++nHits;
        sumPropDist += std::fabs(propaDist);  // propaDist is propagation distance in cm
        sumLayer += d.getLayer();
        sumSection += hit2d.getSection();  // 0=backward, 1=forward
        sumSector += hit2d.getSector();    // 1-8
      }

      if (m_hNumDigitsPerB1dRPC) m_hNumDigitsPerB1dRPC->Fill(nGoodDigitsForThisB1d);
    }
  }
}

// Wrapper: accumulate ALL RPC hits (both phi and z) for backward compatibility
void KLMEventT0EstimatorModule::accumulateBKLMRPC(RelationVector<KLMHit2d>& klmHit2ds,
                                                  const ExtMap& rpcMap,
                                                  double& sumW, double& sumWT, double& sumWT2,
                                                  double& sumW_new, double& sumWT_new,
                                                  int& nHits, double& sumPropDist, double& sumLayer,
                                                  int& sumSection, int& sumSector)
{
  // Use filtered function twice (once for phi, once for z) and accumulate into same sums
  double wPhi = 0, wTPhi = 0, wT2Phi = 0, wPhi_new = 0, wTPhi_new = 0;
  int nPhi = 0; double propPhi = 0, layerPhi = 0; int secPhi = 0, sectPhi = 0;
  accumulateBKLMRPCFiltered(klmHit2ds, rpcMap, true, wPhi, wTPhi, wT2Phi, wPhi_new, wTPhi_new, nPhi, propPhi, layerPhi, secPhi,
                            sectPhi);

  double wZ = 0, wTZ = 0, wT2Z = 0, wZ_new = 0, wTZ_new = 0;
  int nZ = 0; double propZ = 0, layerZ = 0; int secZ = 0, sectZ = 0;
  accumulateBKLMRPCFiltered(klmHit2ds, rpcMap, false, wZ, wTZ, wT2Z, wZ_new, wTZ_new, nZ, propZ, layerZ, secZ, sectZ);

  // Combine
  sumW += wPhi + wZ;
  sumWT += wTPhi + wTZ;
  sumWT2 += wT2Phi + wT2Z;
  sumW_new += wPhi_new + wZ_new;
  sumWT_new += wTPhi_new + wTZ_new;
  nHits += nPhi + nZ;
  sumPropDist += propPhi + propZ;
  sumLayer += layerPhi + layerZ;
  sumSection += secPhi + secZ;
  sumSector += sectPhi + sectZ;
}

// Wrapper: accumulate only phi-readout RPC hits
void KLMEventT0EstimatorModule::accumulateBKLMRPCPhi(RelationVector<KLMHit2d>& klmHit2ds,
                                                     const ExtMap& rpcMap,
                                                     double& sumW, double& sumWT, double& sumWT2,
                                                     double& sumW_new, double& sumWT_new,
                                                     int& nHits, double& sumPropDist, double& sumLayer,
                                                     int& sumSection, int& sumSector)
{
  accumulateBKLMRPCFiltered(klmHit2ds, rpcMap, true, sumW, sumWT, sumWT2, sumW_new, sumWT_new, nHits, sumPropDist, sumLayer,
                            sumSection, sumSector);
}

// Wrapper: accumulate only z-readout RPC hits
void KLMEventT0EstimatorModule::accumulateBKLMRPCZ(RelationVector<KLMHit2d>& klmHit2ds,
                                                   const ExtMap& rpcMap,
                                                   double& sumW, double& sumWT, double& sumWT2,
                                                   double& sumW_new, double& sumWT_new,
                                                   int& nHits, double& sumPropDist, double& sumLayer,
                                                   int& sumSection, int& sumSector)
{
  accumulateBKLMRPCFiltered(klmHit2ds, rpcMap, false, sumW, sumWT, sumWT2, sumW_new, sumWT_new, nHits, sumPropDist, sumLayer,
                            sumSection, sumSector);
}

/* Event. */

void KLMEventT0EstimatorModule::event()
{
  // CDC seed for logging (not used to compute the means)
  m_seedT0 = 0.0;
  if (m_useCDCTemporaryT0) {
    StoreObjPtr<EventT0> evtT0("EventT0", DataStore::c_Event);
    if (evtT0.isValid() && evtT0->hasTemporaryEventT0(Const::EDetector::CDC)) {
      const auto best = evtT0->getBestCDCTemporaryEventT0();
      if (best) m_seedT0 = best->eventT0;
    }
  }

  // MC truth T0 (if available) and sample-type monitor
  m_truthT0 = std::numeric_limits<double>::quiet_NaN();
  StoreObjPtr<MCInitialParticles> ip;
  const bool hasTruth = ip.isValid();
  if (m_fillSampleTypeFlag && m_hSampleType) m_hSampleType->Fill(hasTruth ? 2 : 1);
  if (hasTruth) {
    m_truthT0 = ip->getTime();
    if (m_fillSampleTypeFlag && m_hTruthT0 && std::isfinite(m_truthT0)) m_hTruthT0->Fill(m_truthT0);
  }

  if (!m_MuonList.isValid()) { B2WARNING("KLMEventT0Estimator: ParticleList '" << m_MuonListName << "' not found."); return; }
  const unsigned nTracks = m_MuonList->getListSize();
  if (nTracks == 0u) return;

  // Per-event accumulators for hit-averages (OLD method)
  double sumW = 0.0, sumWT = 0.0, sumWT2 = 0.0; // overall
  double sumWE = 0.0, sumWTE = 0.0, sumWT2E = 0.0; // EKLM
  double sumWB = 0.0, sumWTB = 0.0, sumWT2B = 0.0; // BKLM scint
  double sumWR = 0.0, sumWTR = 0.0, sumWT2R = 0.0; // RPC (combined for backward compat)
  double sumWRphi = 0.0, sumWTRphi = 0.0, sumWT2Rphi = 0.0; // RPC Phi
  double sumWRz = 0.0, sumWTRz = 0.0, sumWT2Rz = 0.0; // RPC Z

  // Per-event accumulators for hit-averages (NEW method - weighted)
  double sumW_new = 0.0, sumWT_new = 0.0;
  double sumWE_new = 0.0, sumWTE_new = 0.0;
  double sumWB_new = 0.0, sumWTB_new = 0.0;
  double sumWR_new = 0.0, sumWTR_new = 0.0; // Combined for backward compat
  double sumWRphi_new = 0.0, sumWTRphi_new = 0.0; // RPC Phi
  double sumWRz_new = 0.0, sumWTRz_new = 0.0; // RPC Z

  // For per-event track-averages: pairs of (T0, SEM) for weighted averaging
  std::vector<std::pair<double, double>> vTrk_B, vTrk_R, vTrk_Rphi, vTrk_Rz, vTrk_E, vTrk_All;

  // Per-track containers for pulls with diagnostic info
  struct TrkVal {
    double mu;           // Mean T0
    double sem;          // Standard error of mean
    int charge;          // Track charge
    int nHits;           // Number of hits
    double avgPropDist;  // Average propagation distance
    double avgLayer;     // Average layer number
    int dominantSection; // Dominant section (0=backward, 1=forward, -1=unknown)
    int dominantSector;  // Dominant sector (1-8, -1=unknown)
    TrkVal(double m, double s, int q, int n = 0, double d = 0, double l = 0, int sec = -1, int sect = -1)
      : mu(m), sem(s), charge(q), nHits(n), avgPropDist(d), avgLayer(l), dominantSection(sec), dominantSector(sect) {}
  };
  std::vector<TrkVal> trkB, trkE, trkR, trkRphi, trkRz;

  // Combined per-track T0 for final-style pulls (matching final event T0 scenarios)
  std::vector<TrkVal> trkScintOnly;  // Scint only: BKLM Scint + EKLM Scint combined per track
  std::vector<TrkVal> trkWithRPC;    // Scint + RPC: all detectors combined per track
  std::vector<TrkVal> trkWithRPCDir; // Scint + RPC Phi + RPC Z: RPC directions treated separately

  // For dimuon ΔT0 using per-track overall means
  double t0_pos = NAN, t0_neg = NAN; bool havePos = false, haveNeg = false;
  double t0_pos_B = NAN, t0_neg_B = NAN; bool havePos_B = false, haveNeg_B = false;
  double t0_pos_R = NAN, t0_neg_R = NAN; bool havePos_R = false, haveNeg_R = false;
  double t0_pos_E = NAN, t0_neg_E = NAN; bool havePos_E = false, haveNeg_E = false;

  // Helper to compute mean and SEM from running sums (OLD method - empirical variance)
  auto mu_sem_from_sums = [](double w, double wt, double wt2) -> std::pair<double, double> {
    if (w <= 0.0) return {NAN, NAN};
    const double mu = wt / w;
    if (w <= 1.0) return {mu, 0.0};
    const double ss  = std::max(wt2 - w* mu * mu, 0.0);
    const double var = ss / (w - 1.0);
    return {mu, std::sqrt(var / w)};
  };

  // Helper to compute weighted mean and uncertainty (NEW method)
  auto weighted_result = [](double wsum, double wtsum) -> std::pair<double, double> {
    if (wsum <= 0.0) return {NAN, NAN};
    return {wtsum / wsum, std::sqrt(1.0 / wsum)};
  };

  for (unsigned i = 0; i < nTracks; ++i) {
    const Particle* particle = m_MuonList->getParticle(i);
    if (!particle) continue;
    const Track* track = particle->getTrack();
    if (!track) continue;

    RelationVector<KLMHit2d> hit2ds = track->getRelationsTo<KLMHit2d>();
    if (hit2ds.size() == 0) continue;

    if (m_hNumKLM2DPerTrack) m_hNumKLM2DPerTrack->Fill(static_cast<int>(hit2ds.size()));

    // Build ExtHit maps for this track
    m_extScint.clear();
    m_extRPC.clear();
    collectExtrapolatedHits(track, m_extScint, m_extRPC);

    // Per-track digit sums per category (BOTH old and new accumulators)
    // Plus diagnostic info: nHits, sumPropDist, sumLayer, sumSection, sumSector
    double wE = 0, wTE = 0, wT2E = 0, wE_new = 0, wTE_new = 0;
    int nE = 0; double propE = 0, layerE = 0; int secE = 0, sectE = 0;
    accumulateEKLM(hit2ds, m_extScint, wE, wTE, wT2E, wE_new, wTE_new, nE, propE, layerE, secE, sectE);

    double wB = 0, wTB = 0, wT2B = 0, wB_new = 0, wTB_new = 0;
    int nB = 0; double propB = 0, layerB = 0; int secB = 0, sectB = 0;
    accumulateBKLMScint(hit2ds, m_extScint, wB, wTB, wT2B, wB_new, wTB_new, nB, propB, layerB, secB, sectB);

    // RPC: accumulate separately for phi and z, plus combined for backward compat
    double wR = 0, wTR = 0, wT2R = 0, wR_new = 0, wTR_new = 0;
    int nR = 0; double propR = 0, layerR = 0; int secR = 0, sectR = 0;
    double wRphi = 0, wTRphi = 0, wT2Rphi = 0, wRphi_new = 0, wTRphi_new = 0;
    int nRphi = 0; double propRphi = 0, layerRphi = 0; int secRphi = 0, sectRphi = 0;
    double wRz = 0, wTRz = 0, wT2Rz = 0, wRz_new = 0, wTRz_new = 0;
    int nRz = 0; double propRz = 0, layerRz = 0; int secRz = 0, sectRz = 0;

    accumulateBKLMRPCPhi(hit2ds, m_extRPC, wRphi, wTRphi, wT2Rphi, wRphi_new, wTRphi_new, nRphi, propRphi, layerRphi, secRphi,
                         sectRphi);
    accumulateBKLMRPCZ(hit2ds, m_extRPC, wRz, wTRz, wT2Rz, wRz_new, wTRz_new, nRz, propRz, layerRz, secRz, sectRz);

    // Combined RPC for backward compatibility
    wR = wRphi + wRz;
    wTR = wTRphi + wTRz;
    wT2R = wT2Rphi + wT2Rz;
    wR_new = wRphi_new + wRz_new;
    wTR_new = wTRphi_new + wTRz_new;
    nR = nRphi + nRz;
    propR = propRphi + propRz;
    layerR = layerRphi + layerRz;
    secR = secRphi + secRz;
    sectR = sectRphi + sectRz;

    // Update per-event hit-avg accumulators (OLD)
    sumWE += wE; sumWTE += wTE; sumWT2E += wT2E;
    sumWB += wB; sumWTB += wTB; sumWT2B += wT2B;
    sumWR += wR; sumWTR += wTR; sumWT2R += wT2R;
    sumWRphi += wRphi; sumWTRphi += wTRphi; sumWT2Rphi += wT2Rphi;
    sumWRz += wRz; sumWTRz += wTRz; sumWT2Rz += wT2Rz;
    sumW  += (wE + wB + wR);
    sumWT += (wTE + wTB + wTR);
    sumWT2 += (wT2E + wT2B + wT2R);

    // Update per-event hit-avg accumulators (NEW)
    sumWE_new += wE_new; sumWTE_new += wTE_new;
    sumWB_new += wB_new; sumWTB_new += wTB_new;
    sumWR_new += wR_new; sumWTR_new += wTR_new;
    sumWRphi_new += wRphi_new; sumWTRphi_new += wTRphi_new;
    sumWRz_new += wRz_new; sumWTRz_new += wTRz_new;
    sumW_new  += (wE_new + wB_new + wR_new);
    sumWT_new += (wTE_new + wTB_new + wTR_new);

    // Per-track means and SEMs by category, plus per-track T0 histos
    const int q = (particle->getCharge() > 0.0 ? +1 : (particle->getCharge() < 0.0 ? -1 : 0));
    double trkW_all = 0.0, trkWT_all = 0.0;

    // Decide which method to use for per-track values
    if (wB > 0.0) {
      double muB, seB;
      if (m_UseNewHitResolution) {
        std::tie(muB, seB) = weighted_result(wB_new, wTB_new);
      } else {
        std::tie(muB, seB) = mu_sem_from_sums(wB, wTB, wT2B);
      }

      if (m_hT0Trk_BKLM_Scint && std::isfinite(muB)) m_hT0Trk_BKLM_Scint->Fill(muB);
      if (m_fillTruthCorrectedTiming && std::isfinite(m_truthT0) && m_hT0Trk_BKLM_Scint_corr && std::isfinite(muB))
        m_hT0Trk_BKLM_Scint_corr->Fill(muB - m_truthT0);
      if (std::isfinite(muB)) {
        vTrk_B.push_back({muB, seB});
        // Compute average propagation distance and layer for diagnostics
        const double avgPropB = (nB > 0) ? propB / nB : 0.0;
        const double avgLayerB = (nB > 0) ? layerB / nB : 0.0;
        // Dominant section: round(sum/n), sector: round(sum/n)
        const int domSecB = (nB > 0) ? static_cast<int>(std::round(static_cast<double>(secB) / nB)) : -1;
        const int domSectB = (nB > 0) ? static_cast<int>(std::round(static_cast<double>(sectB) / nB)) : -1;
        trkB.emplace_back(muB, seB, q, nB, avgPropB, avgLayerB, domSecB, domSectB);
        // Per-track pull diagnostics for BKLM Scint
        if (m_hNHits_PerTrack_BKLM_Scint) m_hNHits_PerTrack_BKLM_Scint->Fill(nB);
        if (m_hSEM_PerTrack_BKLM_Scint && std::isfinite(seB)) m_hSEM_PerTrack_BKLM_Scint->Fill(seB);
        // For overall track average (still unweighted across categories)
        trkW_all += wB;
        trkWT_all += wTB;
      }
    }

    // RPC combined (backward compatibility - still compute for existing histograms)
    if (wR > 0.0) {
      double muR, seR;
      if (m_UseNewHitResolution) {
        std::tie(muR, seR) = weighted_result(wR_new, wTR_new);
      } else {
        std::tie(muR, seR) = mu_sem_from_sums(wR, wTR, wT2R);
      }

      if (m_hT0Trk_BKLM_RPC && std::isfinite(muR)) m_hT0Trk_BKLM_RPC->Fill(muR);
      if (m_fillTruthCorrectedTiming && std::isfinite(m_truthT0) && m_hT0Trk_BKLM_RPC_corr && std::isfinite(muR))
        m_hT0Trk_BKLM_RPC_corr->Fill(muR - m_truthT0);
      if (std::isfinite(muR)) {
        vTrk_R.push_back({muR, seR});
        const double avgPropR = (nR > 0) ? propR / nR : 0.0;
        const double avgLayerR = (nR > 0) ? layerR / nR : 0.0;
        const int domSecR = (nR > 0) ? static_cast<int>(std::round(static_cast<double>(secR) / nR)) : -1;
        const int domSectR = (nR > 0) ? static_cast<int>(std::round(static_cast<double>(sectR) / nR)) : -1;
        trkR.emplace_back(muR, seR, q, nR, avgPropR, avgLayerR, domSecR, domSectR);
        trkW_all += wR;
        trkWT_all += wTR;
      }
    }

    // RPC Phi (direction-specific)
    if (wRphi > 0.0) {
      double muRphi, seRphi;
      if (m_UseNewHitResolution) {
        std::tie(muRphi, seRphi) = weighted_result(wRphi_new, wTRphi_new);
      } else {
        std::tie(muRphi, seRphi) = mu_sem_from_sums(wRphi, wTRphi, wT2Rphi);
      }

      if (std::isfinite(muRphi)) {
        vTrk_Rphi.push_back({muRphi, seRphi});
        const double avgPropRphi = (nRphi > 0) ? propRphi / nRphi : 0.0;
        const double avgLayerRphi = (nRphi > 0) ? layerRphi / nRphi : 0.0;
        const int domSecRphi = (nRphi > 0) ? static_cast<int>(std::round(static_cast<double>(secRphi) / nRphi)) : -1;
        const int domSectRphi = (nRphi > 0) ? static_cast<int>(std::round(static_cast<double>(sectRphi) / nRphi)) : -1;
        trkRphi.emplace_back(muRphi, seRphi, q, nRphi, avgPropRphi, avgLayerRphi, domSecRphi, domSectRphi);
      }
    }

    // RPC Z (direction-specific)
    if (wRz > 0.0) {
      double muRz, seRz;
      if (m_UseNewHitResolution) {
        std::tie(muRz, seRz) = weighted_result(wRz_new, wTRz_new);
      } else {
        std::tie(muRz, seRz) = mu_sem_from_sums(wRz, wTRz, wT2Rz);
      }

      if (std::isfinite(muRz)) {
        vTrk_Rz.push_back({muRz, seRz});
        const double avgPropRz = (nRz > 0) ? propRz / nRz : 0.0;
        const double avgLayerRz = (nRz > 0) ? layerRz / nRz : 0.0;
        const int domSecRz = (nRz > 0) ? static_cast<int>(std::round(static_cast<double>(secRz) / nRz)) : -1;
        const int domSectRz = (nRz > 0) ? static_cast<int>(std::round(static_cast<double>(sectRz) / nRz)) : -1;
        trkRz.emplace_back(muRz, seRz, q, nRz, avgPropRz, avgLayerRz, domSecRz, domSectRz);
      }
    }

    if (wE > 0.0) {
      double muE, seE;
      if (m_UseNewHitResolution) {
        std::tie(muE, seE) = weighted_result(wE_new, wTE_new);
      } else {
        std::tie(muE, seE) = mu_sem_from_sums(wE, wTE, wT2E);
      }

      if (m_hT0Trk_EKLM_Scint && std::isfinite(muE)) m_hT0Trk_EKLM_Scint->Fill(muE);
      if (m_fillTruthCorrectedTiming && std::isfinite(m_truthT0) && m_hT0Trk_EKLM_Scint_corr && std::isfinite(muE))
        m_hT0Trk_EKLM_Scint_corr->Fill(muE - m_truthT0);
      if (std::isfinite(muE)) {
        vTrk_E.push_back({muE, seE});
        const double avgPropE = (nE > 0) ? propE / nE : 0.0;
        const double avgLayerE = (nE > 0) ? layerE / nE : 0.0;
        // EKLM section: 0=backward, 1=forward (converted in accumulator)
        const int domSecE = (nE > 0) ? static_cast<int>(std::round(static_cast<double>(secE) / nE)) : -1;
        const int domSectE = (nE > 0) ? static_cast<int>(std::round(static_cast<double>(sectE) / nE)) : -1;
        trkE.emplace_back(muE, seE, q, nE, avgPropE, avgLayerE, domSecE, domSectE);
        // Per-track pull diagnostics for EKLM Scint
        if (m_hNHits_PerTrack_EKLM_Scint) m_hNHits_PerTrack_EKLM_Scint->Fill(nE);
        if (m_hSEM_PerTrack_EKLM_Scint && std::isfinite(seE)) m_hSEM_PerTrack_EKLM_Scint->Fill(seE);
        trkW_all += wE;
        trkWT_all += wTE;
      }
    }

    // Per-track overall (if any category present)
    // Use weighted accumulation when new method is enabled
    {
      const double wAll_trk = (m_UseNewHitResolution) ? (wB_new + wE_new + wR_new) : trkW_all;
      const double wtAll_trk = (m_UseNewHitResolution) ? (wTB_new + wTE_new + wTR_new) : trkWT_all;
      if (wAll_trk > 0.0) {
        const double t0_trk_all = wtAll_trk / wAll_trk;
        const double se_trk_all = (m_UseNewHitResolution) ? std::sqrt(1.0 / wAll_trk) : 0.0;
        vTrk_All.push_back({t0_trk_all, se_trk_all});

        if (q > 0) { t0_pos = t0_trk_all; havePos = true; }
        if (q < 0) { t0_neg = t0_trk_all; haveNeg = true; }
      }

      // Per-detector T0 for dimuon analysis (use weighted values when new method enabled)
      if (wB_new > 0.0 || wB > 0.0) {
        const double muB_dm = (m_UseNewHitResolution && wB_new > 0.0) ? (wTB_new / wB_new) : (wTB / wB);
        if (q > 0) { t0_pos_B = muB_dm; havePos_B = true; }
        else if (q < 0) { t0_neg_B = muB_dm; haveNeg_B = true; }
      }
      if (wR_new > 0.0 || wR > 0.0) {
        const double muR_dm = (m_UseNewHitResolution && wR_new > 0.0) ? (wTR_new / wR_new) : (wTR / wR);
        if (q > 0) { t0_pos_R = muR_dm; havePos_R = true; }
        else if (q < 0) { t0_neg_R = muR_dm; haveNeg_R = true; }
      }
      if (wE_new > 0.0 || wE > 0.0) {
        const double muE_dm = (m_UseNewHitResolution && wE_new > 0.0) ? (wTE_new / wE_new) : (wTE / wE);
        if (q > 0) { t0_pos_E = muE_dm; havePos_E = true; }
        else if (q < 0) { t0_neg_E = muE_dm; haveNeg_E = true; }
      }
    }

    // Combined per-track T0 for final-style pulls (matching final event T0 scenarios)
    // Scint-only: combine BKLM Scint + EKLM Scint hits on this track
    {
      const double wScint = (m_UseNewHitResolution) ? (wB_new + wE_new) : (wB + wE);
      const double wtScint = (m_UseNewHitResolution) ? (wTB_new + wTE_new) : (wTB + wTE);
      if (wScint > 0.0) {
        const double muScint = wtScint / wScint;
        const double seScint = (m_UseNewHitResolution) ? std::sqrt(1.0 / wScint) : 0.0;  // Proper SEM for weighted avg
        trkScintOnly.emplace_back(muScint, seScint, q);
      }
    }

    // Scint + RPC: combine all KLM hits on this track
    {
      const double wAll = (m_UseNewHitResolution) ? (wB_new + wE_new + wR_new) : (wB + wE + wR);
      const double wtAll = (m_UseNewHitResolution) ? (wTB_new + wTE_new + wTR_new) : (wTB + wTE + wTR);
      if (wAll > 0.0) {
        const double muAll = wtAll / wAll;
        const double seAll = (m_UseNewHitResolution) ? std::sqrt(1.0 / wAll) : 0.0;  // Proper SEM for weighted avg
        trkWithRPC.emplace_back(muAll, seAll, q);
      }
    }

    // Scint + RPC Dir: combine Scint + RPC Phi + RPC Z (RPC directions weighted separately)
    {
      const double wAllDir = (m_UseNewHitResolution) ? (wB_new + wE_new + wRphi_new + wRz_new) : (wB + wE + wRphi + wRz);
      const double wtAllDir = (m_UseNewHitResolution) ? (wTB_new + wTE_new + wTRphi_new + wTRz_new) : (wTB + wTE + wTRphi + wTRz);
      if (wAllDir > 0.0) {
        const double muAllDir = wtAllDir / wAllDir;
        const double seAllDir = (m_UseNewHitResolution) ? std::sqrt(1.0 / wAllDir) : 0.0;
        trkWithRPCDir.emplace_back(muAllDir, seAllDir, q);
      }
    }
  }

  if (sumW <= 0.0 && sumW_new <= 0.0) {
    B2DEBUG(20, "KLMEventT0Estimator: no usable KLM timing residuals for this event.");
    return;
  }

  // Helpers for empirical variance (OLD method)
  auto mean_sem_hits = [](double w, double wt, double wt2) -> std::pair<double, double> {
    if (w <= 0.0) return {NAN, NAN};
    const double mu = wt / w;
    if (w <= 1.0) return {mu, 0.0};
    const double ss  = std::max(wt2 - w* mu * mu, 0.0);
    const double var = ss / (w - 1.0);
    return {mu, std::sqrt(var / w)};
  };

  // Weighted track averaging using inverse-variance (1/SEM²) weighting
  // Takes vector of (T0, SEM) pairs and returns weighted mean and combined uncertainty
  auto mean_sem_tracks_weighted = [](const std::vector<std::pair<double, double>>& v) -> std::pair<double, double> {
    if (v.empty()) return {NAN, NAN};
    if (v.size() == 1)
    {
      return {v[0].first, std::isfinite(v[0].second) ? v[0].second : 0.0};
    }

    double wsum = 0.0, wtsum = 0.0;
    bool allValid = true;
    for (const auto& [t0, sem] : v)
    {
      if (!std::isfinite(sem) || sem <= 0.0) { allValid = false; break; }
    }

    if (allValid)
    {
      for (const auto& [t0, sem] : v) {
        const double w = 1.0 / (sem * sem);
        wsum += w;
        wtsum += w * t0;
      }
      if (wsum > 0.0) {
        return {wtsum / wsum, std::sqrt(1.0 / wsum)};
      }
    }

    // Fallback to simple average if weights not valid
    double s = 0.0;
    for (const auto& [t0, sem] : v) s += t0;
    const double mu = s / v.size();
    double ss = 0.0;
    for (const auto& [t0, sem] : v) { const double d = t0 - mu; ss += d * d; }
    const double var = (v.size() > 1) ? ss / (v.size() - 1) : 0.0;
    return {mu, std::sqrt(var / v.size())};
  };

  // Per-event track-averages using inverse-variance (1/SEM²) weighting
  const auto [muB_trk,   seB_trk]   = mean_sem_tracks_weighted(vTrk_B);
  const auto [muR_trk,   seR_trk]   = mean_sem_tracks_weighted(vTrk_R);
  const auto [muRphi_trk, seRphi_trk] = mean_sem_tracks_weighted(vTrk_Rphi);  // RPC Phi direction
  const auto [muRz_trk,   seRz_trk]   = mean_sem_tracks_weighted(vTrk_Rz);    // RPC Z direction
  const auto [muE_trk,   seE_trk]   = mean_sem_tracks_weighted(vTrk_E);
  const auto [muAll_trk, seAll_trk] = mean_sem_tracks_weighted(vTrk_All);

  if (m_hT0Evt_TrkAvg_BKLM_Scint && std::isfinite(muB_trk))   m_hT0Evt_TrkAvg_BKLM_Scint->Fill(muB_trk);
  if (m_hT0Evt_TrkAvg_BKLM_RPC   && std::isfinite(muR_trk))   m_hT0Evt_TrkAvg_BKLM_RPC->Fill(muR_trk);
  if (m_hT0Evt_TrkAvg_EKLM_Scint && std::isfinite(muE_trk))   m_hT0Evt_TrkAvg_EKLM_Scint->Fill(muE_trk);
  if (m_hT0Evt_TrkAvg_All        && std::isfinite(muAll_trk)) m_hT0Evt_TrkAvg_All->Fill(muAll_trk);

  if (m_hT0Evt_TrkAvg_BKLM_Scint_SEM && std::isfinite(seB_trk))   m_hT0Evt_TrkAvg_BKLM_Scint_SEM->Fill(seB_trk);
  if (m_hT0Evt_TrkAvg_BKLM_RPC_SEM   && std::isfinite(seR_trk))   m_hT0Evt_TrkAvg_BKLM_RPC_SEM->Fill(seR_trk);
  if (m_hT0Evt_TrkAvg_EKLM_Scint_SEM && std::isfinite(seE_trk))   m_hT0Evt_TrkAvg_EKLM_Scint_SEM->Fill(seE_trk);
  if (m_hT0Evt_TrkAvg_All_SEM        && std::isfinite(seAll_trk)) m_hT0Evt_TrkAvg_All_SEM->Fill(seAll_trk);

  // Per-event hit-averages (choose method based on toggle)
  double t0_hit_E, seE_hit, t0_hit_B, seB_hit, t0_hit_R, seR_hit, t0_hit_all, seAll_hit;
  double t0_hit_Rphi, seRphi_hit, t0_hit_Rz, seRz_hit;  // RPC direction-specific

  if (m_UseNewHitResolution) {
    // NEW METHOD: Weighted mean and uncertainty from calibrated sigma
    std::tie(t0_hit_E,   seE_hit)   = weighted_result(sumWE_new, sumWTE_new);
    std::tie(t0_hit_B,   seB_hit)   = weighted_result(sumWB_new, sumWTB_new);
    std::tie(t0_hit_R,   seR_hit)   = weighted_result(sumWR_new, sumWTR_new);
    std::tie(t0_hit_Rphi, seRphi_hit) = weighted_result(sumWRphi_new, sumWTRphi_new);
    std::tie(t0_hit_Rz,   seRz_hit)   = weighted_result(sumWRz_new, sumWTRz_new);
    std::tie(t0_hit_all, seAll_hit) = weighted_result(sumW_new,  sumWT_new);
  } else {
    // OLD METHOD: Empirical SEM from scatter
    std::tie(t0_hit_E,   seE_hit)   = mean_sem_hits(sumWE, sumWTE, sumWT2E);
    std::tie(t0_hit_B,   seB_hit)   = mean_sem_hits(sumWB, sumWTB, sumWT2B);
    std::tie(t0_hit_R,   seR_hit)   = mean_sem_hits(sumWR, sumWTR, sumWT2R);
    std::tie(t0_hit_Rphi, seRphi_hit) = mean_sem_hits(sumWRphi, sumWTRphi, sumWT2Rphi);
    std::tie(t0_hit_Rz,   seRz_hit)   = mean_sem_hits(sumWRz, sumWTRz, sumWT2Rz);
    std::tie(t0_hit_all, seAll_hit) = mean_sem_hits(sumW,  sumWT,  sumWT2);
  }

  if (m_hT0Evt_HitAvg_BKLM_Scint && std::isfinite(t0_hit_B))   m_hT0Evt_HitAvg_BKLM_Scint->Fill(t0_hit_B);
  if (m_hT0Evt_HitAvg_BKLM_RPC   && std::isfinite(t0_hit_R))   m_hT0Evt_HitAvg_BKLM_RPC->Fill(t0_hit_R);
  if (m_hT0Evt_HitAvg_EKLM_Scint && std::isfinite(t0_hit_E))   m_hT0Evt_HitAvg_EKLM_Scint->Fill(t0_hit_E);
  if (m_hT0Evt_HitAvg_All        && std::isfinite(t0_hit_all)) m_hT0Evt_HitAvg_All->Fill(t0_hit_all);

  if (m_hT0Evt_HitAvg_BKLM_Scint_SEM && std::isfinite(seB_hit))   m_hT0Evt_HitAvg_BKLM_Scint_SEM->Fill(seB_hit);
  if (m_hT0Evt_HitAvg_BKLM_RPC_SEM   && std::isfinite(seR_hit))   m_hT0Evt_HitAvg_BKLM_RPC_SEM->Fill(seR_hit);
  if (m_hT0Evt_HitAvg_EKLM_Scint_SEM && std::isfinite(seE_hit))   m_hT0Evt_HitAvg_EKLM_Scint_SEM->Fill(seE_hit);
  if (m_hT0Evt_HitAvg_All_SEM        && std::isfinite(seAll_hit)) m_hT0Evt_HitAvg_All_SEM->Fill(seAll_hit);

  // Truth-corrected fills
  if (m_fillTruthCorrectedTiming && std::isfinite(m_truthT0)) {
    if (m_hT0Evt_TrkAvg_BKLM_Scint_corr && std::isfinite(muB_trk))   m_hT0Evt_TrkAvg_BKLM_Scint_corr->Fill(muB_trk - m_truthT0);
    if (m_hT0Evt_TrkAvg_BKLM_RPC_corr   && std::isfinite(muR_trk))   m_hT0Evt_TrkAvg_BKLM_RPC_corr->Fill(muR_trk - m_truthT0);
    if (m_hT0Evt_TrkAvg_EKLM_Scint_corr && std::isfinite(muE_trk))   m_hT0Evt_TrkAvg_EKLM_Scint_corr->Fill(muE_trk - m_truthT0);
    if (m_hT0Evt_TrkAvg_All_corr        && std::isfinite(muAll_trk)) m_hT0Evt_TrkAvg_All_corr->Fill(muAll_trk - m_truthT0);

    if (m_hT0Evt_TrkAvg_BKLM_Scint_SEM_corr && std::isfinite(seB_trk))   m_hT0Evt_TrkAvg_BKLM_Scint_SEM_corr->Fill(seB_trk);
    if (m_hT0Evt_TrkAvg_BKLM_RPC_SEM_corr   && std::isfinite(seR_trk))   m_hT0Evt_TrkAvg_BKLM_RPC_SEM_corr->Fill(seR_trk);
    if (m_hT0Evt_TrkAvg_EKLM_Scint_SEM_corr && std::isfinite(seE_trk))   m_hT0Evt_TrkAvg_EKLM_Scint_SEM_corr->Fill(seE_trk);
    if (m_hT0Evt_TrkAvg_All_SEM_corr        && std::isfinite(seAll_trk)) m_hT0Evt_TrkAvg_All_SEM_corr->Fill(seAll_trk);

    if (m_hT0Evt_HitAvg_BKLM_Scint_corr && std::isfinite(t0_hit_B))   m_hT0Evt_HitAvg_BKLM_Scint_corr->Fill(t0_hit_B - m_truthT0);
    if (m_hT0Evt_HitAvg_BKLM_RPC_corr   && std::isfinite(t0_hit_R))   m_hT0Evt_HitAvg_BKLM_RPC_corr->Fill(t0_hit_R - m_truthT0);
    if (m_hT0Evt_HitAvg_EKLM_Scint_corr && std::isfinite(t0_hit_E))   m_hT0Evt_HitAvg_EKLM_Scint_corr->Fill(t0_hit_E - m_truthT0);
    if (m_hT0Evt_HitAvg_All_corr        && std::isfinite(t0_hit_all)) m_hT0Evt_HitAvg_All_corr->Fill(t0_hit_all - m_truthT0);

    if (m_hT0Evt_HitAvg_BKLM_Scint_SEM_corr && std::isfinite(seB_hit))   m_hT0Evt_HitAvg_BKLM_Scint_SEM_corr->Fill(seB_hit);
    if (m_hT0Evt_HitAvg_BKLM_RPC_SEM_corr   && std::isfinite(seR_hit))   m_hT0Evt_HitAvg_BKLM_RPC_SEM_corr->Fill(seR_hit);
    if (m_hT0Evt_HitAvg_EKLM_Scint_SEM_corr && std::isfinite(seE_hit))   m_hT0Evt_HitAvg_EKLM_Scint_SEM_corr->Fill(seE_hit);
    if (m_hT0Evt_HitAvg_All_SEM_corr        && std::isfinite(seAll_hit)) m_hT0Evt_HitAvg_All_SEM_corr->Fill(seAll_hit);
  }

  // ---------------- Final KLM combination (single saved component) ----------------
  // Combines ALL 3 KLM detector categories: BKLM Scint, BKLM RPC, EKLM Scint.
  // The FinalUseRPCInKLM flag controls only the monitoring histograms below (scint-only
  // vs with-RPC comparisons). The saved EventT0 always uses all available information.

  // Build per-category event estimates based on FinalAverageMode
  double muB = NAN, muE = NAN, muR = NAN;
  double seB = NAN, seE = NAN, seR = NAN;

  if (m_FinalAverageMode == "hit") {
    muB = t0_hit_B; seB = seB_hit;
    muE = t0_hit_E; seE = seE_hit;
    muR = t0_hit_R; seR = seR_hit;
  } else {
    muB = muB_trk; seB = seB_trk;
    muE = muE_trk; seE = seE_trk;
    muR = muR_trk; seR = seR_trk;
  }

  // Eligible categories — all 3 detectors considered for the saved EventT0
  const bool useB = std::isfinite(muB);
  const bool useE = std::isfinite(muE);
  const bool useR = std::isfinite(muR);

  // Combine using inverse-variance weighting across all available categories
  double finalT0 = NAN, finalSE = NAN;
  // Source bins: 1=B only, 2=E only, 3=R only, 4=B+E, 5=B+R, 6=E+R, 7=B+E+R
  int sourceBin = -1;

  {
    std::vector<std::pair<double, double>> parts;
    if (useB) parts.emplace_back(muB, seB);
    if (useE) parts.emplace_back(muE, seE);
    if (useR) parts.emplace_back(muR, seR);

    if (parts.empty()) {
      B2DEBUG(20, "KLMEventT0Estimator: No eligible KLM categories for final KLM EventT0.");
    } else if (parts.size() == 1) {
      finalT0 = parts[0].first;
      finalSE = std::isfinite(parts[0].second) ? parts[0].second : 0.0;
      if (useB) sourceBin = 1;
      else if (useE) sourceBin = 2;
      else sourceBin = 3;
    } else {
      double wsum = 0.0, wtsum = 0.0;
      bool okW = true;
      for (auto& pr : parts) { const double se = pr.second; if (!std::isfinite(se) || se <= 0.0) { okW = false; break; } }
      if (okW) {
        for (auto& pr : parts) { const double w = 1.0 / (pr.second * pr.second); wsum += w; wtsum += w * pr.first; }
        if (wsum > 0.0) { finalT0 = wtsum / wsum; finalSE = std::sqrt(1.0 / wsum); }
      }
      if (!std::isfinite(finalT0)) {
        double s = 0.0;
        for (auto& pr : parts) s += pr.first;
        finalT0 = s / parts.size();
        if (parts.size() > 1) {
          double ss = 0.0;
          for (auto& pr : parts) { const double d = pr.first - finalT0; ss += d * d; }
          finalSE = std::sqrt((ss / (parts.size() - 1)) / parts.size());
        } else finalSE = std::isfinite(parts[0].second) ? parts[0].second : 0.0;
      }
      // Determine source combination
      if (useB && useE && useR) sourceBin = 7;
      else if (useB && useE) sourceBin = 4;
      else if (useB && useR) sourceBin = 5;
      else if (useE && useR) sourceBin = 6;
      else if (useB) sourceBin = 1;
      else if (useE) sourceBin = 2;
      else sourceBin = 3;
    }
  }

  // Report / monitors
  B2INFO("KLMEventT0Estimator: "
         << "T0_hitavg_all=" << t0_hit_all << " ns  (seed CDC=" << m_seedT0 << " ns)"
         << " | E=" << t0_hit_E << " | Bsc=" << t0_hit_B << " | Brpc=" << t0_hit_R
         << (std::isfinite(finalT0) ? (std::string(" | FINAL KLM=") + std::to_string(finalT0) + " ns") : std::string(""))
         << (std::isfinite(m_truthT0) ? (std::string(" | truth T0=") + std::to_string(m_truthT0) + " ns") : std::string("")));

  // Write single KLM component to EventT0 and fill final histos
  StoreObjPtr<EventT0> outT0("EventT0", DataStore::c_Event);
  if (!outT0.isValid()) outT0.construct();

  if (std::isfinite(finalT0)) {
    if (m_hFinalSource && sourceBin > 0) m_hFinalSource->Fill(sourceBin);

    // Quality = number of contributing detector categories (1–3)
    const double quality = static_cast<double>((useB ? 1 : 0) + (useE ? 1 : 0) + (useR ? 1 : 0));

    EventT0::EventT0Component klmT0Component(finalT0, std::isfinite(finalSE) ? finalSE : 0.0,
                                             Const::KLM, "KLM", quality);
    outT0->addTemporaryEventT0(klmT0Component);
    outT0->setEventT0(klmT0Component);
  }

  // ---------------- Multiple Final EventT0 combinations ----------------
  // Helper to combine categories using inverse-variance weighting
  auto combineFinal = [](const std::vector<std::pair<double, double>>& parts) -> std::pair<double, double> {
    if (parts.empty()) return {NAN, NAN};
    if (parts.size() == 1)
    {
      return {parts[0].first, std::isfinite(parts[0].second) ? parts[0].second : 0.0};
    }
    double wsum = 0.0, wtsum = 0.0;
    bool okW = true;
    for (const auto& pr : parts)
    {
      if (!std::isfinite(pr.second) || pr.second <= 0.0) { okW = false; break; }
    }
    if (okW)
    {
      for (const auto& pr : parts) {
        const double w = 1.0 / (pr.second * pr.second);
        wsum += w;
        wtsum += w * pr.first;
      }
      if (wsum > 0.0) return {wtsum / wsum, std::sqrt(1.0 / wsum)};
    }
    // Fallback: simple average
    double s = 0.0;
    for (const auto& pr : parts) s += pr.first;
    double mu = s / parts.size();
    double ss = 0.0;
    for (const auto& pr : parts) { const double d = pr.first - mu; ss += d * d; }
    double se = (parts.size() > 1) ? std::sqrt((ss / (parts.size() - 1)) / parts.size()) : 0.0;
    return {mu, se};
  };

  // Get per-category values based on FinalAverageMode
  double muRphi_final = NAN, seRphi_final = NAN;
  double muRz_final = NAN, seRz_final = NAN;
  if (m_FinalAverageMode == "hit") {
    muRphi_final = t0_hit_Rphi; seRphi_final = seRphi_hit;
    muRz_final = t0_hit_Rz; seRz_final = seRz_hit;
  } else {
    muRphi_final = muRphi_trk; seRphi_final = seRphi_trk;
    muRz_final = muRz_trk; seRz_final = seRz_trk;
  }

  // 1. Scintillator only (BKLM Scint + EKLM Scint) - no RPC
  {
    std::vector<std::pair<double, double>> partsScint;
    if (std::isfinite(muB)) partsScint.emplace_back(muB, seB);
    if (std::isfinite(muE)) partsScint.emplace_back(muE, seE);
    const auto [t0_scint, se_scint] = combineFinal(partsScint);
    if (std::isfinite(t0_scint)) {
      if (m_hT0Evt_Final_ScintOnly) m_hT0Evt_Final_ScintOnly->Fill(t0_scint);
      if (m_hT0Evt_Final_ScintOnly_SEM && std::isfinite(se_scint)) m_hT0Evt_Final_ScintOnly_SEM->Fill(se_scint);
      if (m_fillTruthCorrectedTiming && std::isfinite(m_truthT0) && m_hT0Evt_Final_ScintOnly_corr)
        m_hT0Evt_Final_ScintOnly_corr->Fill(t0_scint - m_truthT0);
    }
  }

  // 2. Scintillator + RPC combined (all detectors, RPC as single category)
  {
    std::vector<std::pair<double, double>> partsWithRPC;
    if (std::isfinite(muB)) partsWithRPC.emplace_back(muB, seB);
    if (std::isfinite(muE)) partsWithRPC.emplace_back(muE, seE);
    if (std::isfinite(muR)) partsWithRPC.emplace_back(muR, seR);  // RPC combined
    const auto [t0_rpc, se_rpc] = combineFinal(partsWithRPC);
    if (std::isfinite(t0_rpc)) {
      if (m_hT0Evt_Final_WithRPC) m_hT0Evt_Final_WithRPC->Fill(t0_rpc);
      if (m_hT0Evt_Final_WithRPC_SEM && std::isfinite(se_rpc)) m_hT0Evt_Final_WithRPC_SEM->Fill(se_rpc);
      if (m_fillTruthCorrectedTiming && std::isfinite(m_truthT0) && m_hT0Evt_Final_WithRPC_corr)
        m_hT0Evt_Final_WithRPC_corr->Fill(t0_rpc - m_truthT0);
    }
  }

  // 3. Scintillator + RPC with directions separate (phi and z as separate categories)
  {
    std::vector<std::pair<double, double>> partsWithRPCDir;
    if (std::isfinite(muB)) partsWithRPCDir.emplace_back(muB, seB);
    if (std::isfinite(muE)) partsWithRPCDir.emplace_back(muE, seE);
    if (std::isfinite(muRphi_final)) partsWithRPCDir.emplace_back(muRphi_final, seRphi_final);  // RPC Phi
    if (std::isfinite(muRz_final)) partsWithRPCDir.emplace_back(muRz_final, seRz_final);  // RPC Z
    const auto [t0_dir, se_dir] = combineFinal(partsWithRPCDir);
    if (std::isfinite(t0_dir)) {
      if (m_hT0Evt_Final_WithRPCDir) m_hT0Evt_Final_WithRPCDir->Fill(t0_dir);
      if (m_hT0Evt_Final_WithRPCDir_SEM && std::isfinite(se_dir)) m_hT0Evt_Final_WithRPCDir_SEM->Fill(se_dir);
      if (m_fillTruthCorrectedTiming && std::isfinite(m_truthT0) && m_hT0Evt_Final_WithRPCDir_corr)
        m_hT0Evt_Final_WithRPCDir_corr->Fill(t0_dir - m_truthT0);
    }
  }

  // ---------------- Pull distributions ----------------
  if (m_FillPulls) {
    // Basic pull function for standard histograms (4 main categories)
    auto fill_pairs_pull = [&](const std::vector<TrkVal>& vv, TH1D * h) {
      if (!h) return;
      const size_t n = vv.size();
      for (size_t i = 0; i < n; ++i) {
        const TrkVal& ti = vv[i];
        if (!std::isfinite(ti.mu) || !(ti.sem >= 0.0)) continue;
        for (size_t j = i + 1; j < n; ++j) {
          const TrkVal& tj = vv[j];
          if (!std::isfinite(tj.mu) || !(tj.sem >= 0.0)) continue;
          if (m_PullsUseOppositeChargesOnly && ti.charge * tj.charge >= 0) continue;
          const double den2 = ti.sem * ti.sem + tj.sem * tj.sem;
          if (!(den2 > 0.0)) continue;
          const double pull = (ti.mu - tj.mu) / std::sqrt(den2);
          if (std::isfinite(pull)) h->Fill(pull);
        }
      }
    };

    // Fill the 4 main overall pull histograms
    fill_pairs_pull(trkB, m_hPull_BKLM_Scint);
    fill_pairs_pull(trkE, m_hPull_EKLM_Scint);
    fill_pairs_pull(trkRphi, m_hPull_BKLM_RPC_Phi);
    fill_pairs_pull(trkRz, m_hPull_BKLM_RPC_Z);
    fill_pairs_pull(trkR, m_hPull_BKLM_RPC);  // Combined RPC for backward compatibility

    // Cross-subdetector pulls
    if (m_PullsUseSameTrackCross) {
      auto fill_cross_same_track = [&](const std::vector<TrkVal>& vX, const std::vector<TrkVal>& vY, TH1D * h) {
        if (!h) return;
        const size_t n = std::min(vX.size(), vY.size());
        for (size_t k = 0; k < n; ++k) {
          const TrkVal& tx = vX[k];
          const TrkVal& ty = vY[k];
          if (!std::isfinite(tx.mu) || !std::isfinite(ty.mu)) continue;
          if (!(tx.sem >= 0.0) || !(ty.sem >= 0.0)) continue;
          const double den2 = tx.sem * tx.sem + ty.sem * ty.sem;
          if (!(den2 > 0.0)) continue;
          const double pull = (tx.mu - ty.mu) / std::sqrt(den2);
          if (std::isfinite(pull)) h->Fill(pull);
        }
      };
      fill_cross_same_track(trkB, trkE, m_hPull_B_vs_E);
      fill_cross_same_track(trkB, trkR, m_hPull_B_vs_R);
      fill_cross_same_track(trkE, trkR, m_hPull_E_vs_R);
    } else {
      auto fill_cross_event = [&](double muX, double seX, double muY, double seY, TH1D * h) {
        if (!h) return;
        if (!std::isfinite(muX) || !std::isfinite(muY)) return;
        if (!(seX >= 0.0) || !(seY >= 0.0)) return;
        const double den2 = seX * seX + seY * seY;
        if (!(den2 > 0.0)) return;
        const double pull = (muX - muY) / std::sqrt(den2);
        if (std::isfinite(pull)) h->Fill(pull);
      };
      fill_cross_event(muB_trk, seB_trk, muE_trk, seE_trk, m_hPull_B_vs_E);
      fill_cross_event(muB_trk, seB_trk, muR_trk, seR_trk, m_hPull_B_vs_R);
      fill_cross_event(muE_trk, seE_trk, muR_trk, seR_trk, m_hPull_E_vs_R);
    }

    // --- Pairwise sector pull analysis ---
    // For each pair of opposite-charge tracks, fill the pairwise histogram
    // based on their dominant sectors. The pull is computed from the track's
    // overall T0 (weighted by all hits on that track for the detector category).
    // This reveals systematic offsets between sector pairs.

    // Helper to fill pairwise sector histogram
    auto fill_pairwise_sector = [&](const std::vector<TrkVal>& vv,
    TH1D * hPairwise[][c_nBKLMSectors], int nSectors, int sectorOffset = 0) {
      const size_t n = vv.size();
      for (size_t i = 0; i < n; ++i) {
        const TrkVal& ti = vv[i];
        if (!std::isfinite(ti.mu) || !(ti.sem >= 0.0)) continue;
        const int si = ti.dominantSector - sectorOffset;  // Convert to 0-indexed
        if (si < 0 || si >= nSectors) continue;

        for (size_t j = i + 1; j < n; ++j) {
          const TrkVal& tj = vv[j];
          if (!std::isfinite(tj.mu) || !(tj.sem >= 0.0)) continue;
          if (m_PullsUseOppositeChargesOnly && ti.charge * tj.charge >= 0) continue;
          const int sj = tj.dominantSector - sectorOffset;
          if (sj < 0 || sj >= nSectors) continue;

          const double den2 = ti.sem * ti.sem + tj.sem * tj.sem;
          if (!(den2 > 0.0)) continue;
          const double pull = (ti.mu - tj.mu) / std::sqrt(den2);
          if (!std::isfinite(pull)) continue;

          // Fill pairwise histogram [sector_i][sector_j]
          if (hPairwise[si][sj]) hPairwise[si][sj]->Fill(pull);
          // Also fill [sector_j][sector_i] with negated pull for symmetry
          if (si != sj && hPairwise[sj][si]) hPairwise[sj][si]->Fill(-pull);
        }
      }
    };

    // BKLM Scintillator pairwise (8 sectors, 0-indexed)
    fill_pairwise_sector(trkB, m_hPullPairwise_BScint, c_nBKLMSectors, 0);

    // RPC Phi pairwise (8 sectors, 0-indexed)
    fill_pairwise_sector(trkRphi, m_hPullPairwise_RPC_Phi, c_nBKLMSectors, 0);

    // RPC Z pairwise (8 sectors, 0-indexed)
    fill_pairwise_sector(trkRz, m_hPullPairwise_RPC_Z, c_nBKLMSectors, 0);

    // EKLM Scintillator cross-endcap pairwise (4x4 sectors)
    // For e+e- -> mu+mu-: one muon in forward endcap, one in backward
    // Histogram [i][j] = pull for forward sector (i+1) vs backward sector (j+1)
    {
      const size_t n = trkE.size();
      for (size_t i = 0; i < n; ++i) {
        const TrkVal& ti = trkE[i];
        if (!std::isfinite(ti.mu) || !(ti.sem >= 0.0)) continue;
        const int si = ti.dominantSector - 1;  // EKLM sectors 1-4 -> array index 0-3
        if (si < 0 || si >= c_nEKLMSectors) continue;
        const int secI = ti.dominantSection;  // 0=backward, 1=forward
        if (secI < 0) continue;  // Skip if section unknown

        for (size_t j = i + 1; j < n; ++j) {
          const TrkVal& tj = trkE[j];
          if (!std::isfinite(tj.mu) || !(tj.sem >= 0.0)) continue;
          if (m_PullsUseOppositeChargesOnly && ti.charge * tj.charge >= 0) continue;
          const int sj = tj.dominantSector - 1;
          if (sj < 0 || sj >= c_nEKLMSectors) continue;
          const int secJ = tj.dominantSection;
          if (secJ < 0) continue;

          // Only compare tracks from DIFFERENT endcaps (cross-endcap)
          if (secI == secJ) continue;

          const double den2 = ti.sem * ti.sem + tj.sem * tj.sem;
          if (!(den2 > 0.0)) continue;

          // Determine which track is forward and which is backward
          // pull = T0_forward - T0_backward
          int fwdSec, bwdSec;
          double pull;
          if (secI == 1) {  // ti is forward, tj is backward
            fwdSec = si;
            bwdSec = sj;
            pull = (ti.mu - tj.mu) / std::sqrt(den2);
          } else {  // ti is backward, tj is forward
            fwdSec = sj;
            bwdSec = si;
            pull = (tj.mu - ti.mu) / std::sqrt(den2);
          }
          if (!std::isfinite(pull)) continue;

          // Fill histogram [fwdSec][bwdSec]
          if (m_hPullPairwise_EScint_FwdVsBwd[fwdSec][bwdSec]) {
            m_hPullPairwise_EScint_FwdVsBwd[fwdSec][bwdSec]->Fill(pull);
          }
        }
      }
    }
  }

  // ---------------- Residual distributions (mirror of pulls, without normalization) ----------------
  if (m_FillPulls) {
    // Basic residual function: ΔT0 = T0_i − T0_j (no SEM normalization)
    auto fill_pairs_residual = [&](const std::vector<TrkVal>& vv, TH1D * h) {
      if (!h) return;
      const size_t n = vv.size();
      for (size_t i = 0; i < n; ++i) {
        const TrkVal& ti = vv[i];
        if (!std::isfinite(ti.mu)) continue;
        for (size_t j = i + 1; j < n; ++j) {
          const TrkVal& tj = vv[j];
          if (!std::isfinite(tj.mu)) continue;
          if (m_PullsUseOppositeChargesOnly && ti.charge * tj.charge >= 0) continue;
          const double residual = ti.mu - tj.mu;
          if (std::isfinite(residual)) h->Fill(residual);
        }
      }
    };

    // Fill the main overall residual histograms (same-detector)
    fill_pairs_residual(trkB, m_hResidual_BKLM_Scint);
    fill_pairs_residual(trkE, m_hResidual_EKLM_Scint);
    fill_pairs_residual(trkRphi, m_hResidual_BKLM_RPC_Phi);
    fill_pairs_residual(trkRz, m_hResidual_BKLM_RPC_Z);
    fill_pairs_residual(trkR, m_hResidual_BKLM_RPC);

    // Cross-subdetector residuals
    if (m_PullsUseSameTrackCross) {
      auto fill_cross_same_track_residual = [&](const std::vector<TrkVal>& vX, const std::vector<TrkVal>& vY, TH1D * h) {
        if (!h) return;
        const size_t n = std::min(vX.size(), vY.size());
        for (size_t k = 0; k < n; ++k) {
          const TrkVal& tx = vX[k];
          const TrkVal& ty = vY[k];
          if (!std::isfinite(tx.mu) || !std::isfinite(ty.mu)) continue;
          const double residual = tx.mu - ty.mu;
          if (std::isfinite(residual)) h->Fill(residual);
        }
      };
      fill_cross_same_track_residual(trkB, trkE, m_hResidual_B_vs_E);
      fill_cross_same_track_residual(trkB, trkR, m_hResidual_B_vs_R);
      fill_cross_same_track_residual(trkE, trkR, m_hResidual_E_vs_R);
    } else {
      auto fill_cross_event_residual = [&](double muX, double muY, TH1D * h) {
        if (!h) return;
        if (!std::isfinite(muX) || !std::isfinite(muY)) return;
        const double residual = muX - muY;
        if (std::isfinite(residual)) h->Fill(residual);
      };
      fill_cross_event_residual(muB_trk, muE_trk, m_hResidual_B_vs_E);
      fill_cross_event_residual(muB_trk, muR_trk, m_hResidual_B_vs_R);
      fill_cross_event_residual(muE_trk, muR_trk, m_hResidual_E_vs_R);
    }

    // --- Pairwise sector residual analysis ---
    auto fill_pairwise_sector_residual = [&](const std::vector<TrkVal>& vv,
    TH1D * hPairwise[][c_nBKLMSectors], int nSectors, int sectorOffset = 0) {
      const size_t n = vv.size();
      for (size_t i = 0; i < n; ++i) {
        const TrkVal& ti = vv[i];
        if (!std::isfinite(ti.mu)) continue;
        const int si = ti.dominantSector - sectorOffset;
        if (si < 0 || si >= nSectors) continue;

        for (size_t j = i + 1; j < n; ++j) {
          const TrkVal& tj = vv[j];
          if (!std::isfinite(tj.mu)) continue;
          if (m_PullsUseOppositeChargesOnly && ti.charge * tj.charge >= 0) continue;
          const int sj = tj.dominantSector - sectorOffset;
          if (sj < 0 || sj >= nSectors) continue;

          const double residual = ti.mu - tj.mu;
          if (!std::isfinite(residual)) continue;

          if (hPairwise[si][sj]) hPairwise[si][sj]->Fill(residual);
          if (si != sj && hPairwise[sj][si]) hPairwise[sj][si]->Fill(-residual);
        }
      }
    };

    // BKLM Scintillator pairwise residuals
    fill_pairwise_sector_residual(trkB, m_hResidualPairwise_BScint, c_nBKLMSectors, 0);

    // RPC Phi pairwise residuals
    fill_pairwise_sector_residual(trkRphi, m_hResidualPairwise_RPC_Phi, c_nBKLMSectors, 0);

    // RPC Z pairwise residuals
    fill_pairwise_sector_residual(trkRz, m_hResidualPairwise_RPC_Z, c_nBKLMSectors, 0);

    // EKLM Scintillator cross-endcap pairwise residuals
    {
      const size_t n = trkE.size();
      for (size_t i = 0; i < n; ++i) {
        const TrkVal& ti = trkE[i];
        if (!std::isfinite(ti.mu)) continue;
        const int si = ti.dominantSector - 1;
        if (si < 0 || si >= c_nEKLMSectors) continue;
        const int secI = ti.dominantSection;
        if (secI < 0) continue;

        for (size_t j = i + 1; j < n; ++j) {
          const TrkVal& tj = trkE[j];
          if (!std::isfinite(tj.mu)) continue;
          if (m_PullsUseOppositeChargesOnly && ti.charge * tj.charge >= 0) continue;
          const int sj = tj.dominantSector - 1;
          if (sj < 0 || sj >= c_nEKLMSectors) continue;
          const int secJ = tj.dominantSection;
          if (secJ < 0) continue;

          // Only compare tracks from DIFFERENT endcaps
          if (secI == secJ) continue;

          // residual = T0_forward - T0_backward
          int fwdSec, bwdSec;
          double residual;
          if (secI == 1) {  // ti is forward, tj is backward
            fwdSec = si;
            bwdSec = sj;
            residual = ti.mu - tj.mu;
          } else {  // ti is backward, tj is forward
            fwdSec = sj;
            bwdSec = si;
            residual = tj.mu - ti.mu;
          }
          if (!std::isfinite(residual)) continue;

          if (m_hResidualPairwise_EScint_FwdVsBwd[fwdSec][bwdSec]) {
            m_hResidualPairwise_EScint_FwdVsBwd[fwdSec][bwdSec]->Fill(residual);
          }
        }
      }
    }
  }

  // ---------------- Cross-detector ΔT0 analysis (by region) ----------------
  // Regions: 0=EKLM Backward, 1=EKLM Forward, 2=BKLM RPC, 3=BKLM Scint
  if (m_FillPulls) {
    // Build a unified list of all tracks with their detector region
    struct RegionTrk {
      double mu;
      int charge;
      int region;  // 0=EKLM Bwd, 1=EKLM Fwd, 2=BKLM RPC, 3=BKLM Scint
      RegionTrk(double m, int q, int r) : mu(m), charge(q), region(r) {}
    };
    std::vector<RegionTrk> allRegionTrks;

    // Add EKLM tracks (split by backward/forward)
    for (const auto& t : trkE) {
      if (!std::isfinite(t.mu)) continue;
      int region = -1;
      if (t.dominantSection == 0) region = 0;       // EKLM Backward
      else if (t.dominantSection == 1) region = 1;  // EKLM Forward
      if (region >= 0) allRegionTrks.emplace_back(t.mu, t.charge, region);
    }

    // Add BKLM RPC tracks (region 2)
    for (const auto& t : trkR) {
      if (!std::isfinite(t.mu)) continue;
      allRegionTrks.emplace_back(t.mu, t.charge, 2);
    }

    // Add BKLM Scint tracks (region 3)
    for (const auto& t : trkB) {
      if (!std::isfinite(t.mu)) continue;
      allRegionTrks.emplace_back(t.mu, t.charge, 3);
    }

    // Fill ΔT0 for each pair of tracks from different regions
    const size_t nReg = allRegionTrks.size();
    for (size_t i = 0; i < nReg; ++i) {
      const RegionTrk& ti = allRegionTrks[i];
      for (size_t j = i + 1; j < nReg; ++j) {
        const RegionTrk& tj = allRegionTrks[j];

        // Only opposite charges for dimuon-like events
        if (m_PullsUseOppositeChargesOnly && ti.charge * tj.charge >= 0) continue;

        const double deltaT0 = ti.mu - tj.mu;
        if (!std::isfinite(deltaT0)) continue;

        // Fill [region_i][region_j]
        if (m_hDeltaT0_DetectorCombo[ti.region][tj.region]) {
          m_hDeltaT0_DetectorCombo[ti.region][tj.region]->Fill(deltaT0);
        }
        // Also fill [region_j][region_i] with negated value for symmetry
        if (ti.region != tj.region && m_hDeltaT0_DetectorCombo[tj.region][ti.region]) {
          m_hDeltaT0_DetectorCombo[tj.region][ti.region]->Fill(-deltaT0);
        }
      }
    }
  }

  // Optional dimuon ΔT0
  if (m_fillDimuonDeltaT0 && havePos && haveNeg && m_hDimuonDeltaT0) {
    const double dT0 = t0_pos - t0_neg;
    m_hDimuonDeltaT0->Fill(dT0);
    B2INFO("KLMEventT0Estimator (dimuon): T0(mu+)= " << t0_pos
           << " ns, T0(mu-)= " << t0_neg << " ns, ΔT0= " << dT0 << " ns");
  }
  if (m_fillDimuonDeltaT0) {
    if (havePos_B && haveNeg_B && m_hDimuonDeltaT0_B) {
      const double dT0B = t0_pos_B - t0_neg_B; m_hDimuonDeltaT0_B->Fill(dT0B);
      B2INFO("KLMEventT0Estimator (dimuon, BKLM-Scint): T0(mu+)= " << t0_pos_B << " ns, T0(mu-)= " << t0_neg_B << " ns, ΔT0= " << dT0B <<
             " ns");
    }
    if (havePos_R && haveNeg_R && m_hDimuonDeltaT0_R) {
      const double dT0R = t0_pos_R - t0_neg_R; m_hDimuonDeltaT0_R->Fill(dT0R);
      B2INFO("KLMEventT0Estimator (dimuon, BKLM-RPC): T0(mu+)= " << t0_pos_R << " ns, T0(mu-)= " << t0_neg_R << " ns, ΔT0= " << dT0R <<
             " ns");
    }
    if (havePos_E && haveNeg_E && m_hDimuonDeltaT0_E) {
      const double dT0E = t0_pos_E - t0_neg_E; m_hDimuonDeltaT0_E->Fill(dT0E);
      B2INFO("KLMEventT0Estimator (dimuon, EKLM-Scint): T0(mu+)= " << t0_pos_E << " ns, T0(mu-)= " << t0_neg_E << " ns, ΔT0= " << dT0E <<
             " ns");
    }

    // Combined category ΔT0 (ScintOnly, WithRPC, WithRPCDir)
    // Extract μ+ and μ- T0 from the combined per-track vectors
    auto extractDimuonT0 = [](const std::vector<TrkVal>& trks) -> std::pair<double, double> {
      double t0_plus = std::numeric_limits<double>::quiet_NaN();
      double t0_minus = std::numeric_limits<double>::quiet_NaN();
      for (const auto& t : trks)
      {
        if (t.charge > 0 && std::isfinite(t.mu)) t0_plus = t.mu;
        if (t.charge < 0 && std::isfinite(t.mu)) t0_minus = t.mu;
      }
      return {t0_plus, t0_minus};
    };

    // Scintillator Only (BKLM Scint + EKLM Scint per track)
    {
      const auto [t0p, t0m] = extractDimuonT0(trkScintOnly);
      if (std::isfinite(t0p) && std::isfinite(t0m) && m_hDimuonDeltaT0_ScintOnly) {
        m_hDimuonDeltaT0_ScintOnly->Fill(t0p - t0m);
      }
    }

    // With RPC (Scint + RPC combined per track)
    {
      const auto [t0p, t0m] = extractDimuonT0(trkWithRPC);
      if (std::isfinite(t0p) && std::isfinite(t0m) && m_hDimuonDeltaT0_WithRPC) {
        m_hDimuonDeltaT0_WithRPC->Fill(t0p - t0m);
      }
    }

    // With RPC Direction (Scint + RPC Phi + RPC Z per track, directions weighted separately)
    {
      const auto [t0p, t0m] = extractDimuonT0(trkWithRPCDir);
      if (std::isfinite(t0p) && std::isfinite(t0m) && m_hDimuonDeltaT0_WithRPCDir) {
        m_hDimuonDeltaT0_WithRPCDir->Fill(t0p - t0m);
      }
    }
  }
}

