/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMEventT0Estimator/KLMEventT0EstimatorModule.h>

/* KLM headers — only those not already pulled in via the own header. */
#include <klm/bklm/geometry/Module.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

/* Basf2 headers — only those not already pulled in via the own header. */
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>

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

    } // end diagnostics/ directory

    // --- final/ subdirectory (skipping to here; pulls/residuals/cross_detector
    //     are handled by KLMEventT0ValidationModule in klm/validation/) ---
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
  B2DEBUG(20, "KLMEventT0Estimator: ADC cuts configured:"
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

    B2DEBUG(20, "KLMEventT0Estimator: Using NEW calibrated per-hit resolution method."
            << LogVar("sigma_RPC (ns)", m_eventT0HitResolution->getSigmaRPC())
            << LogVar("sigma_BKLM_Scint (ns)", m_eventT0HitResolution->getSigmaBKLMScint())
            << LogVar("sigma_EKLM_Scint (ns)", m_eventT0HitResolution->getSigmaEKLMScint()));
  } else {
    B2WARNING("KLMEventT0Estimator: Using OLD per-event SEM calculation (may be unreliable with few hits).");
  }
}

void KLMEventT0EstimatorModule::endRun()
{
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
                                               double& sumW_new, double& sumWT_new)
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


    }

    if (m_hNumDigitsPerE2dScint) m_hNumDigitsPerE2dScint->Fill(nGoodDigits);
  }
}

/* BKLM scintillator */
void KLMEventT0EstimatorModule::accumulateBKLMScint(RelationVector<KLMHit2d>& klmHit2ds,
                                                    const ExtMap& scintMap,
                                                    double& sumW, double& sumWT, double& sumWT2,
                                                    double& sumW_new, double& sumWT_new)
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


      }

      if (m_hNumDigitsPerB1dScint) m_hNumDigitsPerB1dScint->Fill(nGoodDigitsForThisB1d);
    }
  }
}

/* BKLM RPC */
// Accumulate BKLM RPC hits with a specific readout plane filter
void KLMEventT0EstimatorModule::accumulateBKLMRPCFiltered(RelationVector<KLMHit2d>& klmHit2ds,
                                                          const ExtMap& rpcMap,
                                                          bool acceptPhi,
                                                          double& sumW, double& sumWT, double& sumWT2,
                                                          double& sumW_new, double& sumWT_new)
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


      }

      if (m_hNumDigitsPerB1dRPC) m_hNumDigitsPerB1dRPC->Fill(nGoodDigitsForThisB1d);
    }
  }
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

    // Per-track digit sums per category
    double wE = 0, wTE = 0, wT2E = 0, wE_new = 0, wTE_new = 0;
    accumulateEKLM(hit2ds, m_extScint, wE, wTE, wT2E, wE_new, wTE_new);

    double wB = 0, wTB = 0, wT2B = 0, wB_new = 0, wTB_new = 0;
    accumulateBKLMScint(hit2ds, m_extScint, wB, wTB, wT2B, wB_new, wTB_new);

    // RPC: accumulate separately for phi and z
    double wRphi = 0, wTRphi = 0, wT2Rphi = 0, wRphi_new = 0, wTRphi_new = 0;
    accumulateBKLMRPCFiltered(hit2ds, m_extRPC, true,  wRphi, wTRphi, wT2Rphi, wRphi_new, wTRphi_new);
    double wRz = 0, wTRz = 0, wT2Rz = 0, wRz_new = 0, wTRz_new = 0;
    accumulateBKLMRPCFiltered(hit2ds, m_extRPC, false, wRz,   wTRz,   wT2Rz,   wRz_new,   wTRz_new);

    // Combined RPC for backward compatibility
    const double wR     = wRphi + wRz;
    const double wTR    = wTRphi + wTRz;
    const double wT2R   = wT2Rphi + wT2Rz;
    const double wR_new  = wRphi_new + wRz_new;
    const double wTR_new = wTRphi_new + wTRz_new;

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

    // Per-track means and SEMs by category
    double trkW_all = 0.0, trkWT_all = 0.0;

    if (wB > 0.0) {
      double muB, seB;
      if (m_UseNewHitResolution) {
        std::tie(muB, seB) = weighted_result(wB_new, wTB_new);
      } else {
        std::tie(muB, seB) = mu_sem_from_sums(wB, wTB, wT2B);
      }
      if (m_hT0Trk_BKLM_Scint && std::isfinite(muB)) m_hT0Trk_BKLM_Scint->Fill(muB);
      if (std::isfinite(muB)) {
        vTrk_B.push_back({muB, seB});
        trkW_all += wB;
        trkWT_all += wTB;
      }
    }

    // RPC combined (backward compatibility)
    if (wR > 0.0) {
      double muR, seR;
      if (m_UseNewHitResolution) {
        std::tie(muR, seR) = weighted_result(wR_new, wTR_new);
      } else {
        std::tie(muR, seR) = mu_sem_from_sums(wR, wTR, wT2R);
      }
      if (m_hT0Trk_BKLM_RPC && std::isfinite(muR)) m_hT0Trk_BKLM_RPC->Fill(muR);
      if (std::isfinite(muR)) {
        vTrk_R.push_back({muR, seR});
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
      if (std::isfinite(muE)) {
        vTrk_E.push_back({muE, seE});
        trkW_all += wE;
        trkWT_all += wTE;
      }
    }

    // Per-track overall (if any category present)
    {
      const double wAll_trk  = (m_UseNewHitResolution) ? (wB_new + wE_new + wR_new) : trkW_all;
      const double wtAll_trk = (m_UseNewHitResolution) ? (wTB_new + wTE_new + wTR_new) : trkWT_all;
      if (wAll_trk > 0.0) {
        const double t0_trk_all = wtAll_trk / wAll_trk;
        const double se_trk_all = (m_UseNewHitResolution) ? std::sqrt(1.0 / wAll_trk) : 0.0;
        vTrk_All.push_back({t0_trk_all, se_trk_all});
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
  auto mean_sem_tracks_weighted = [](const std::vector<std::pair<double, double>>& v) -> std::pair<double, double> {
    if (v.empty()) return {NAN, NAN};
    if (v.size() == 1)
    {
      return {v[0].first, std::isfinite(v[0].second) ? v[0].second : 0.0};
    }

    bool allValid = true;
    for (const auto& [t0, sem] : v)
    {
      if (!std::isfinite(sem) || sem <= 0.0) { allValid = false; break; }
    }

    if (allValid)
    {
      double wsum = 0.0, wtsum = 0.0;
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
  const auto [muB_trk,    seB_trk]    = mean_sem_tracks_weighted(vTrk_B);
  const auto [muR_trk,    seR_trk]    = mean_sem_tracks_weighted(vTrk_R);
  const auto [muRphi_trk, seRphi_trk] = mean_sem_tracks_weighted(vTrk_Rphi);
  const auto [muRz_trk,   seRz_trk]   = mean_sem_tracks_weighted(vTrk_Rz);
  const auto [muE_trk,    seE_trk]    = mean_sem_tracks_weighted(vTrk_E);
  const auto [muAll_trk,  seAll_trk]  = mean_sem_tracks_weighted(vTrk_All);

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
  double t0_hit_Rphi, seRphi_hit, t0_hit_Rz, seRz_hit;

  if (m_UseNewHitResolution) {
    std::tie(t0_hit_E,    seE_hit)    = weighted_result(sumWE_new,    sumWTE_new);
    std::tie(t0_hit_B,    seB_hit)    = weighted_result(sumWB_new,    sumWTB_new);
    std::tie(t0_hit_R,    seR_hit)    = weighted_result(sumWR_new,    sumWTR_new);
    std::tie(t0_hit_Rphi, seRphi_hit) = weighted_result(sumWRphi_new, sumWTRphi_new);
    std::tie(t0_hit_Rz,   seRz_hit)   = weighted_result(sumWRz_new,   sumWTRz_new);
    std::tie(t0_hit_all,  seAll_hit)  = weighted_result(sumW_new,     sumWT_new);
  } else {
    std::tie(t0_hit_E,    seE_hit)    = mean_sem_hits(sumWE,    sumWTE,    sumWT2E);
    std::tie(t0_hit_B,    seB_hit)    = mean_sem_hits(sumWB,    sumWTB,    sumWT2B);
    std::tie(t0_hit_R,    seR_hit)    = mean_sem_hits(sumWR,    sumWTR,    sumWT2R);
    std::tie(t0_hit_Rphi, seRphi_hit) = mean_sem_hits(sumWRphi, sumWTRphi, sumWT2Rphi);
    std::tie(t0_hit_Rz,   seRz_hit)   = mean_sem_hits(sumWRz,   sumWTRz,   sumWT2Rz);
    std::tie(t0_hit_all,  seAll_hit)  = mean_sem_hits(sumW,     sumWT,     sumWT2);
  }

  if (m_hT0Evt_HitAvg_BKLM_Scint && std::isfinite(t0_hit_B))   m_hT0Evt_HitAvg_BKLM_Scint->Fill(t0_hit_B);
  if (m_hT0Evt_HitAvg_BKLM_RPC   && std::isfinite(t0_hit_R))   m_hT0Evt_HitAvg_BKLM_RPC->Fill(t0_hit_R);
  if (m_hT0Evt_HitAvg_EKLM_Scint && std::isfinite(t0_hit_E))   m_hT0Evt_HitAvg_EKLM_Scint->Fill(t0_hit_E);
  if (m_hT0Evt_HitAvg_All        && std::isfinite(t0_hit_all)) m_hT0Evt_HitAvg_All->Fill(t0_hit_all);

  if (m_hT0Evt_HitAvg_BKLM_Scint_SEM && std::isfinite(seB_hit))   m_hT0Evt_HitAvg_BKLM_Scint_SEM->Fill(seB_hit);
  if (m_hT0Evt_HitAvg_BKLM_RPC_SEM   && std::isfinite(seR_hit))   m_hT0Evt_HitAvg_BKLM_RPC_SEM->Fill(seR_hit);
  if (m_hT0Evt_HitAvg_EKLM_Scint_SEM && std::isfinite(seE_hit))   m_hT0Evt_HitAvg_EKLM_Scint_SEM->Fill(seE_hit);
  if (m_hT0Evt_HitAvg_All_SEM        && std::isfinite(seAll_hit)) m_hT0Evt_HitAvg_All_SEM->Fill(seAll_hit);

  // ---------------- Final KLM combination (single saved component) ----------------
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

  const bool useB = std::isfinite(muB);
  const bool useE = std::isfinite(muE);
  const bool useR = std::isfinite(muR);

  double finalT0 = NAN, finalSE = NAN;
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
      bool okW = true;
      for (auto& pr : parts) { const double se = pr.second; if (!std::isfinite(se) || se <= 0.0) { okW = false; break; } }
      if (okW) {
        double wsum = 0.0, wtsum = 0.0;
        for (auto& pr : parts) { const double w = 1.0 / (pr.second * pr.second); wsum += w; wtsum += w * pr.first; }
        if (wsum > 0.0) { finalT0 = wtsum / wsum; finalSE = std::sqrt(1.0 / wsum); }
      }
      if (!std::isfinite(finalT0)) {
        double s = 0.0;
        for (auto& pr : parts) s += pr.first;
        finalT0 = s / parts.size();
        double ss = 0.0;
        for (auto& pr : parts) { const double d = pr.first - finalT0; ss += d * d; }
        finalSE = std::sqrt((ss / (parts.size() - 1)) / parts.size());
      }
      if (useB && useE && useR) sourceBin = 7;
      else if (useB && useE) sourceBin = 4;
      else if (useB && useR) sourceBin = 5;
      else if (useE && useR) sourceBin = 6;
      else if (useB) sourceBin = 1;
      else if (useE) sourceBin = 2;
      else sourceBin = 3;
    }
  }

  B2DEBUG(20, "KLMEventT0Estimator: "
          << "T0_hitavg_all=" << t0_hit_all << " ns  (seed CDC=" << m_seedT0 << " ns)"
          << " | E=" << t0_hit_E << " | Bsc=" << t0_hit_B << " | Brpc=" << t0_hit_R
          << (std::isfinite(finalT0) ? (std::string(" | FINAL KLM=") + std::to_string(finalT0) + " ns") : std::string("")));

  StoreObjPtr<EventT0> outT0("EventT0", DataStore::c_Event);
  if (!outT0.isValid()) outT0.construct();

  if (std::isfinite(finalT0)) {
    if (m_hFinalSource && sourceBin > 0) m_hFinalSource->Fill(sourceBin);
    const double quality = static_cast<double>((useB ? 1 : 0) + (useE ? 1 : 0) + (useR ? 1 : 0));
    EventT0::EventT0Component klmT0Component(finalT0, std::isfinite(finalSE) ? finalSE : 0.0,
                                             Const::KLM, "KLM", quality);
    outT0->addTemporaryEventT0(klmT0Component);
    outT0->setEventT0(klmT0Component);
  }

  // ---------------- Multiple Final EventT0 combinations ----------------
  auto combineFinal = [](const std::vector<std::pair<double, double>>& parts) -> std::pair<double, double> {
    if (parts.empty()) return {NAN, NAN};
    if (parts.size() == 1)
    {
      return {parts[0].first, std::isfinite(parts[0].second) ? parts[0].second : 0.0};
    }
    bool okW = true;
    for (const auto& pr : parts)
    {
      if (!std::isfinite(pr.second) || pr.second <= 0.0) { okW = false; break; }
    }
    if (okW)
    {
      double wsum = 0.0, wtsum = 0.0;
      for (const auto& pr : parts) {
        const double w = 1.0 / (pr.second * pr.second);
        wsum += w;
        wtsum += w * pr.first;
      }
      if (wsum > 0.0) return {wtsum / wsum, std::sqrt(1.0 / wsum)};
    }
    double s = 0.0;
    for (const auto& pr : parts) s += pr.first;
    double mu = s / parts.size();
    double ss = 0.0;
    for (const auto& pr : parts) { const double d = pr.first - mu; ss += d * d; }
    double se = (parts.size() > 1) ? std::sqrt((ss / (parts.size() - 1)) / parts.size()) : 0.0;
    return {mu, se};
  };

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
    }
  }

  // 2. Scintillator + RPC combined (all detectors, RPC as single category)
  {
    std::vector<std::pair<double, double>> partsWithRPC;
    if (std::isfinite(muB)) partsWithRPC.emplace_back(muB, seB);
    if (std::isfinite(muE)) partsWithRPC.emplace_back(muE, seE);
    if (std::isfinite(muR)) partsWithRPC.emplace_back(muR, seR);
    const auto [t0_rpc, se_rpc] = combineFinal(partsWithRPC);
    if (std::isfinite(t0_rpc)) {
      if (m_hT0Evt_Final_WithRPC) m_hT0Evt_Final_WithRPC->Fill(t0_rpc);
      if (m_hT0Evt_Final_WithRPC_SEM && std::isfinite(se_rpc)) m_hT0Evt_Final_WithRPC_SEM->Fill(se_rpc);
    }
  }

  // 3. Scintillator + RPC with directions separate (phi and z as separate categories)
  {
    std::vector<std::pair<double, double>> partsWithRPCDir;
    if (std::isfinite(muB)) partsWithRPCDir.emplace_back(muB, seB);
    if (std::isfinite(muE)) partsWithRPCDir.emplace_back(muE, seE);
    if (std::isfinite(muRphi_final)) partsWithRPCDir.emplace_back(muRphi_final, seRphi_final);
    if (std::isfinite(muRz_final)) partsWithRPCDir.emplace_back(muRz_final, seRz_final);
    const auto [t0_dir, se_dir] = combineFinal(partsWithRPCDir);
    if (std::isfinite(t0_dir)) {
      if (m_hT0Evt_Final_WithRPCDir) m_hT0Evt_Final_WithRPCDir->Fill(t0_dir);
      if (m_hT0Evt_Final_WithRPCDir_SEM && std::isfinite(se_dir)) m_hT0Evt_Final_WithRPCDir_SEM->Fill(se_dir);
    }
  }
}
