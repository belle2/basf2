// klm/modules/KLMEventT0Estimator/include/KLMEventT0EstimatorModule.h
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)
 * Author: The Belle II Collaboration
 *
 * See git log for contributors and copyright holders.
 * This file is licensed under LGPL-3.0, see LICENSE.md.
 **************************************************************************/

#pragma once

/* KLM geometry & dataobjects */
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/eklm/geometry/TransformData.h>
#include <klm/dataobjects/KLMHit2d.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/bklm/BKLMHit1d.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMMuidLikelihood.h>

/* KLM conditions */
#include <klm/dbobjects/KLMChannelStatus.h>
#include <klm/dbobjects/KLMTimeConstants.h>
#include <klm/dbobjects/KLMTimeCableDelay.h>
#include <klm/dbobjects/KLMEventT0HitResolution.h>

/* Tracking / analysis */
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

/* Framework */
#include <framework/core/HistoModule.h>               // use HistoModule (like KLMDQM2)
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>             // needed for DBObjPtr<T>
#include <framework/datastore/RelationVector.h>       // needed for RelationVector<T>
#include <framework/dataobjects/EventT0.h>
#include <framework/gearbox/Unit.h>

/* C++ */
#include <map>
#include <string>
#include <utility>
#include <limits>                                      // needed for std::numeric_limits

/* ---- ROOT forward declarations (global namespace) ---- */
class TH1D;
class TH1I;
class TH2D;

namespace Belle2 {

  /**
   * Estimate per-event T0 using KLM (BKLM scint, BKLM RPC, EKLM scint).
   *
   * Inputs:
   *   - ParticleList (tracks with relations to KLMHit2d and ExtHit)
   *   - KLM conditions (time constants, cable delays, channel status)
   *   - BKLM/EKLM geometry for propagation distances and transforms
   *
   * Outputs:
   *   - A single KLM EventT0 component per event (combination of all available categories:
   *     BKLM-Scint, BKLM-RPC, EKLM-Scint) saved via addTemporaryEventT0 and setEventT0.
   *   - Monitoring histograms under `histogramDirectoryName` with subdirectories:
   *       <histogramDirectoryName>/uncorrected and <histogramDirectoryName>/truth_corrected
   *       (the latter filled only on MC when m_fillTruthCorrectedTiming is true).
   *
   *       * Track and content diagnostics (uncorrected only)
   *         - h_nKLM2d_pertrk
   *         - h_nDigits_perB1d_rpc, h_nDigits_perB1d_scint, h_nDigits_perE2d_scint
   *         - h_digitQ_bklm_scint, h_digitQ_eklm_scint
   *         - h_sample_type (2-bin counter: Data vs MC)
   *         - h_truth_t0 (MC only, optional)
   *
   *       * Per-digit timing components (booked in BOTH subdirs)
   *           For each D in {BKLM-Scint, BKLM-RPC, EKLM-Scint}:
   *             - h_Trec_<D>   : raw digit time (KLMDigit::getTime) [ns]
   *             - h_Tcable_<D> : cable delay from KLMTimeCableDelay [ns]
   *             - h_Tprop_<D>  : signal propagation along readout [ns]
   *             - h_Tfly_<D>   : flight time from tracking (0.5*(TOF_entry+TOF_exit)) [ns]
   *           Note: In 'truth_corrected', these component histograms are mirrored for MC
   *                 (no truth subtraction) to validate the breakdown alongside truth-subtracted summaries.
   *
   *       * Per-track T0 (per category D in {BKLM-Scint, BKLM-RPC, EKLM-Scint})
   *           Uncorrected:
   *             - h_t0trk_bklm_scint, h_t0trk_bklm_rpc, h_t0trk_eklm_scint
   *           Truth-corrected (MC only; values are (T0 - T0_true)):
   *             - h_t0trk_bklm_scint, h_t0trk_bklm_rpc, h_t0trk_eklm_scint
   *
   *       * Per-event T0 (average across qualified tracks) — MEAN and its UNCERTAINTY (SEM)
   *           Uncorrected:
   *             - Track-average:   mean → h_t0evt_trkavg_bklm_scint / _bklm_rpc / _eklm_scint / _all
   *                                SEM  → h_t0evt_trkavg_*_sem
   *             - Hit-average:     mean → h_t0evt_hitavg_bklm_scint / _bklm_rpc / _eklm_scint / _all
   *                                SEM  → h_t0evt_hitavg_*_sem
   *           Truth-corrected (MC only; means are (T0 - T0_true); SEM is mirrored, no subtraction):
   *             - Track-average:   mean → h_t0evt_trkavg_*      ; SEM → h_t0evt_trkavg_*_sem
   *             - Hit-average:     mean → h_t0evt_hitavg_*      ; SEM → h_t0evt_hitavg_*_sem
   *
   *       * Final KLM Event T0 (single number saved to EventT0)
   *           Uncorrected:
   *             - Value → h_t0evt_klm_final
   *             - Uncertainty (SEM) → h_t0evt_klm_final_sem
   *           Truth-corrected (MC only; value is (final - T0_true); SEM mirrored):
   *             - Value → h_t0evt_klm_final
   *             - Uncertainty (SEM) → h_t0evt_klm_final_sem
   *           Audit of source selection:
   *             - h_final_source (7 bins: "B only", "E only", "R only", "B+E", "B+R", "E+R", "B+E+R")
   *
   *       * (Optional, steering flag) Dimuon ΔT0 (uncorrected only)
   *         - h_dimuon_delta_t0, and per-subdetector variants
   *
   *       * Pull distributions (optional)
   *         - Same-subdetector pulls: BKLM-Scint, BKLM-RPC, EKLM-Scint
   *         - Cross-subdetector pulls: BKLM-Scint vs EKLM-Scint, BKLM-Scint vs BKLM-RPC, EKLM-Scint vs BKLM-RPC
   *
   * Combination rule (final KLM EventT0):
   *   - All 3 detector categories (BKLM-Scint, BKLM-RPC, EKLM-Scint) are included
   *     in the final saved EventT0 whenever they have valid data.
   *   - If only one category present: final = that category's event T0.
   *   - If multiple present: weighted average using inverse-variance (1/SEM^2) weighting
   *     from the selected average mode (see FinalAverageMode).
   *   - The FinalUseRPCInKLM flag controls only the monitoring histograms (scint-only
   *     vs with-RPC comparisons), not the saved EventT0.
   *
   * Notes:
   *   - Truth-corrected histograms are filled only when MC truth Event T0 is available.
   *   - SEM = standard error of the mean computed from the event's contributing items
   *     (per-digits for hit-average; per-tracks for track-average).
   *   - Charge and multiplicity histograms remain uncorrected by design.
   */
  class KLMEventT0EstimatorModule : public HistoModule {

  public:
    KLMEventT0EstimatorModule();
    ~KLMEventT0EstimatorModule() override;

    /** Definition of histograms (called once by HistoManager). */
    void defineHisto() override;

    /** Register inputs/params; get geometry; call REG_HISTOGRAM. */
    void initialize() override;

    /** Per-run resets if desired (histos remain booked). */
    void beginRun() override;

    /** Per-event algorithm: collect hits, compute residuals, fill outputs. */
    void event() override;

    /** Called when the current run ends. */
    void endRun() override;

    /** Called at the end of processing. */
    void terminate() override;

  private:
    /* ---------- Local helpers (implemented in .cc) ---------- */

    using ExtMap  = std::multimap<unsigned int, Belle2::ExtHit>;
    using ExtPair = std::pair<Belle2::ExtHit*, Belle2::ExtHit*>;

    // In the private section, add these member variables:
    double m_ADCCut_BKLM_Scint_Min;  /**< Minimum ADC cut for BKLM scintillator */
    double m_ADCCut_BKLM_Scint_Max;  /**< Maximum ADC cut for BKLM scintillator */
    double m_ADCCut_EKLM_Scint_Min;  /**< Minimum ADC cut for EKLM scintillator */
    double m_ADCCut_EKLM_Scint_Max;  /**< Maximum ADC cut for EKLM scintillator */

    // Add this helper function declaration:
    /**
     * Check if a digit passes the ADC charge cut
     * @param charge ADC charge value
     * @param subdetector KLM subdetector (BKLM or EKLM)
     * @param layer Layer number (1-indexed)
     * @param inRPC Whether the digit is from RPC
     * @return true if passes cut, false otherwise
     */
    bool passesADCCut(double charge, int subdetector, int layer, bool inRPC) const;

    /** Build maps of extrapolated hits for a track (scint: channel key; RPC: module key). */
    void collectExtrapolatedHits(const Track* track, ExtMap& scintMap, ExtMap& rpcMap);

    /** Find earliest (entry) and latest (exit) ExtHits matching a key (channel or module). */
    ExtPair matchExt(unsigned int key, ExtMap& v_ExtHits);

    /**
     * Get per-hit sigma for a digit based on detector category (uses new or old method).
     * @param subdetector KLM subdetector (BKLM or EKLM)
     * @param layer Layer number (1-indexed)
     * @param inRPC Whether the digit is from RPC
     * @param plane Plane number (for RPC: BKLMElementNumbers::c_ZPlane or c_PhiPlane)
     * @return Per-hit sigma in ns
     */
    double getHitSigma(int subdetector, int layer, bool inRPC, int plane = 0) const;

    /**
     * Accumulate EKLM scintillator per-digit T0 estimates.
     * NEW: Now includes sumW_new and sumWT_new for calibrated weighting,
     * plus diagnostic accumulators (nHits, sumPropDist, sumLayer, sumSection, sumSector).
     * Note: EKLM section 1=backward (z<0), section 2=forward (z>0).
     */
    void accumulateEKLM(const RelationVector<KLMHit2d>&, const ExtMap&,
                        double& sumW, double& sumWT, double& sumWT2,
                        double& sumW_new, double& sumWT_new,
                        int& nHits, double& sumPropDist, double& sumLayer,
                        int& sumSection, int& sumSector);

    /**
     * Accumulate BKLM scintillator per-digit T0 estimates.
     * NEW: Now includes sumW_new and sumWT_new for calibrated weighting,
     * plus diagnostic accumulators (nHits, sumPropDist, sumLayer, sumSection, sumSector).
     */
    void accumulateBKLMScint(RelationVector<KLMHit2d>&, const ExtMap&,
                             double& sumW, double& sumWT, double& sumWT2,
                             double& sumW_new, double& sumWT_new,
                             int& nHits, double& sumPropDist, double& sumLayer,
                             int& sumSection, int& sumSector);

    /**
     * Accumulate BKLM RPC per-digit T0 estimates (filtered by readout direction).
     * NEW: Now includes sumW_new and sumWT_new for calibrated weighting,
     * plus diagnostic accumulators (nHits, sumPropDist, sumLayer, sumSection, sumSector).
     * @param acceptPhi if true, only accumulate phi-readout hits; if false, only z-readout
     */
    void accumulateBKLMRPCFiltered(RelationVector<KLMHit2d>&, const ExtMap&,
                                   bool acceptPhi,
                                   double& sumW, double& sumWT, double& sumWT2,
                                   double& sumW_new, double& sumWT_new,
                                   int& nHits, double& sumPropDist, double& sumLayer,
                                   int& sumSection, int& sumSector);

    /**
     * Convenience wrapper: accumulate ALL BKLM RPC hits (both phi and z).
     * Calls accumulateBKLMRPCFiltered twice to get both directions.
     */
    void accumulateBKLMRPC(RelationVector<KLMHit2d>&, const ExtMap&,
                           double& sumW, double& sumWT, double& sumWT2,
                           double& sumW_new, double& sumWT_new,
                           int& nHits, double& sumPropDist, double& sumLayer,
                           int& sumSection, int& sumSector);

    /**
     * Convenience wrapper: accumulate only phi-readout BKLM RPC hits.
     */
    void accumulateBKLMRPCPhi(RelationVector<KLMHit2d>&, const ExtMap&,
                              double& sumW, double& sumWT, double& sumWT2,
                              double& sumW_new, double& sumWT_new,
                              int& nHits, double& sumPropDist, double& sumLayer,
                              int& sumSection, int& sumSector);

    /**
     * Convenience wrapper: accumulate only z-readout BKLM RPC hits.
     */
    void accumulateBKLMRPCZ(RelationVector<KLMHit2d>&, const ExtMap&,
                            double& sumW, double& sumWT, double& sumWT2,
                            double& sumW_new, double& sumWT_new,
                            int& nHits, double& sumPropDist, double& sumLayer,
                            int& sumSection, int& sumSector);

    /* ---------- Parameters (set via addParam in constructor) ---------- */

    /** If true, use calibrated per-hit resolution from KLMEventT0HitResolution payload (default: true). */
    bool m_UseNewHitResolution{true};

    /** Input ParticleList (e.g. "mu+:forT0"). */
    std::string m_MuonListName;

    /** Use CDC temporary EventT0 as a diagnostic seed (not applied to averaging). */
    bool m_useCDCTemporaryT0{true};

    /** Ignore backward-propagated ExtHits when forming entry/exit pairs. */
    bool m_ignoreBackward{false};

    /** Parent directory inside the ROOT file (HistoManager) for this module. */
    std::string m_histDirName;

    /** Subdirectory names for uncorrected and truth-corrected histograms, created under m_histDirName. */
    std::string m_histSubdirUncorr{"uncorrected"};
    std::string m_histSubdirCorr{"truth_corrected"};

    /** If true, duplicate and fill truth-corrected timing histograms when MC truth Event T0 is present. */
    bool m_fillTruthCorrectedTiming{true};

    /** If true, book and fill a 2-bin "sample type" histogram (Data vs MC) and a truth t0 monitor. */
    bool m_fillSampleTypeFlag{true};

    /** If true, fill dimuon ΔT0 histogram when both charges have qualified tracks. */
    bool m_fillDimuonDeltaT0{false};

    /** Final KLM averaging mode: "track" (default) or "hit". */
    std::string m_FinalAverageMode{"track"};

    /** If true, include BKLM-RPC in the final KLM weighting (default: false). */
    bool m_FinalUseRPCInKLM{false};

    /** If true, fill pull histograms. */
    bool m_FillPulls{true};

    /** If true, only use opposite-charge track pairs to form pulls. */
    bool m_PullsUseOppositeChargesOnly{true};

    /** If true, also form cross-subdet pulls using the same track. */
    bool m_PullsUseSameTrackCross{false};

    /* ---------- Geometry / conditions ---------- */

    /** BKLM geometry. */
    Belle2::bklm::GeometryPar* m_geoParB{nullptr};

    /** EKLM geometry and transforms. */
    const Belle2::EKLM::GeometryData* m_geoParE{nullptr};
    Belle2::EKLM::TransformData* m_transformE{nullptr};

    /** Element numbering helpers. */
    const KLMElementNumbers* m_elementNum{&KLMElementNumbers::Instance()};

    /** Channel status (Normal/Dead/etc.). */
    DBObjPtr<KLMChannelStatus> m_channelStatus;

    /** NEW: Per-hit time resolution for EventT0 estimation. */
    DBObjPtr<KLMEventT0HitResolution> m_eventT0HitResolution;

    /* ---------- Inputs ---------- */

    /** Selected particle list and tracks (for relations to KLMHit2d / ExtHit). */
    StoreObjPtr<ParticleList> m_MuonList;
    StoreArray<Track> m_tracks;

    /* ---------- Working buffers (reused each event) ---------- */

    /** Extrapolated hits keyed by channel (scint) and by module (RPC). */
    ExtMap m_extScint;
    ExtMap m_extRPC;

    /** Optional seed from CDC (for logging only). */
    double m_seedT0{0.0};

    /** Cached truth Event T0 for this event (ns). NAN if not available. */
    double m_truthT0{std::numeric_limits<double>::quiet_NaN()};

    /* ---------- Monitoring histograms ---------- */

    /* Diagnostics (uncorrected only) */
    TH1I* m_hNumKLM2DPerTrack{nullptr};          // h_nKLM2d_pertrk
    TH1I* m_hNumDigitsPerB1dRPC{nullptr};        // h_nDigits_perB1d_rpc
    TH1I* m_hNumDigitsPerB1dScint{nullptr};      // h_nDigits_perB1d_scint
    TH1I* m_hNumDigitsPerE2dScint{nullptr};      // h_nDigits_perE2d_scint
    TH1D* m_hDigitCharge_BKLM_Scint{nullptr};    // h_digitQ_bklm_scint
    TH1D* m_hDigitCharge_EKLM_Scint{nullptr};    // h_digitQ_eklm_scint

    // Per-track diagnostics for pull validation
    TH1I* m_hNHits_PerTrack_BKLM_Scint{nullptr}; // h_nhits_pertrk_bklm_scint
    TH1I* m_hNHits_PerTrack_EKLM_Scint{nullptr}; // h_nhits_pertrk_eklm_scint
    TH1D* m_hSEM_PerTrack_BKLM_Scint{nullptr};   // h_sem_pertrk_bklm_scint
    TH1D* m_hSEM_PerTrack_EKLM_Scint{nullptr};   // h_sem_pertrk_eklm_scint

    /* Sample type and truth t0 monitors (booked if m_fillSampleTypeFlag) */
    TH1I* m_hSampleType{nullptr};                // h_sample_type (bin 1: Data, bin 2: MC)
    TH1D* m_hTruthT0{nullptr};                   // h_truth_t0 (ns), MC only

    /* --- Uncorrected timing histograms (in <dir>/uncorrected) --- */

    // Per-track T0 distributions (per category)
    TH1D* m_hT0Trk_BKLM_Scint{nullptr};          // h_t0trk_bklm_scint
    TH1D* m_hT0Trk_BKLM_RPC{nullptr};            // h_t0trk_bklm_rpc
    TH1D* m_hT0Trk_EKLM_Scint{nullptr};          // h_t0trk_eklm_scint

    // Per-event T0 (track-average) MEAN
    TH1D* m_hT0Evt_TrkAvg_BKLM_Scint{nullptr};   // h_t0evt_trkavg_bklm_scint
    TH1D* m_hT0Evt_TrkAvg_BKLM_RPC{nullptr};     // h_t0evt_trkavg_bklm_rpc
    TH1D* m_hT0Evt_TrkAvg_EKLM_Scint{nullptr};   // h_t0evt_trkavg_eklm_scint
    TH1D* m_hT0Evt_TrkAvg_All{nullptr};          // h_t0evt_trkavg_all
    // Per-event T0 (track-average) SEM
    TH1D* m_hT0Evt_TrkAvg_BKLM_Scint_SEM{nullptr};  // h_t0evt_trkavg_bklm_scint_sem
    TH1D* m_hT0Evt_TrkAvg_BKLM_RPC_SEM{nullptr};    // h_t0evt_trkavg_bklm_rpc_sem
    TH1D* m_hT0Evt_TrkAvg_EKLM_Scint_SEM{nullptr};  // h_t0evt_trkavg_eklm_scint_sem
    TH1D* m_hT0Evt_TrkAvg_All_SEM{nullptr};         // h_t0evt_trkavg_all_sem

    // Per-event T0 (hit-average) MEAN
    TH1D* m_hT0Evt_HitAvg_BKLM_Scint{nullptr};   // h_t0evt_hitavg_bklm_scint
    TH1D* m_hT0Evt_HitAvg_BKLM_RPC{nullptr};     // h_t0evt_hitavg_bklm_rpc
    TH1D* m_hT0Evt_HitAvg_EKLM_Scint{nullptr};   // h_t0evt_hitavg_eklm_scint
    TH1D* m_hT0Evt_HitAvg_All{nullptr};          // h_t0evt_hitavg_all
    // Per-event T0 (hit-average) SEM
    TH1D* m_hT0Evt_HitAvg_BKLM_Scint_SEM{nullptr};  // h_t0evt_hitavg_bklm_scint_sem
    TH1D* m_hT0Evt_HitAvg_BKLM_RPC_SEM{nullptr};    // h_t0evt_hitavg_bklm_rpc_sem
    TH1D* m_hT0Evt_HitAvg_EKLM_Scint_SEM{nullptr};  // h_t0evt_hitavg_eklm_scint_sem
    TH1D* m_hT0Evt_HitAvg_All_SEM{nullptr};         // h_t0evt_hitavg_all_sem

    // Final-source audit (uncorrected): 1=B only, 2=E only, 3=R only, 4=B+E, 5=B+R, 6=E+R, 7=B+E+R
    TH1I* m_hFinalSource{nullptr};                  // h_final_source

    // Optional dimuon ΔT0 (uncorrected only)
    TH1D* m_hDimuonDeltaT0{nullptr};                // h_dimuon_delta_t0
    TH1D* m_hDimuonDeltaT0_B{nullptr};
    TH1D* m_hDimuonDeltaT0_R{nullptr};
    TH1D* m_hDimuonDeltaT0_E{nullptr};

    // Dimuon ΔT0 for combined categories (matching final event T0 styles)
    TH1D* m_hDimuonDeltaT0_ScintOnly{nullptr};      // h_dimuon_delta_t0_scint_only (BKLM+EKLM scint per track)
    TH1D* m_hDimuonDeltaT0_WithRPC{nullptr};        // h_dimuon_delta_t0_with_rpc (all detectors per track)
    TH1D* m_hDimuonDeltaT0_WithRPCDir{nullptr};     // h_dimuon_delta_t0_with_rpc_dir (RPC phi/z separate)

    // T0 resolution summaries (filled in endRun from Gaussian fits to ΔT0)
    // Per-track resolution: σ_track = σ(ΔT0)/√2
    // Event T0 resolution (combining 2 tracks): σ_event = σ_track/√2 = σ(ΔT0)/2
    TH1D* m_hPerTrackT0Resolution{nullptr};        // h_per_track_t0_resolution: σ(ΔT0)/√2
    TH1D* m_hEventT0Resolution{nullptr};           // h_event_t0_resolution: σ(ΔT0)/2

    // Per-digit timing components (uncorrected)
    TH1D* m_hTrec_BKLM_Scint{nullptr};              // h_Trec_bklm_scint
    TH1D* m_hTcable_BKLM_Scint{nullptr};            // h_Tcable_bklm_scint
    TH1D* m_hTprop_BKLM_Scint{nullptr};             // h_Tprop_bklm_scint
    TH1D* m_hTfly_BKLM_Scint{nullptr};              // h_Tfly_bklm_scint

    TH1D* m_hTrec_BKLM_RPC{nullptr};                // h_Trec_bklm_rpc
    TH1D* m_hTcable_BKLM_RPC{nullptr};              // h_Tcable_bklm_rpc
    TH1D* m_hTprop_BKLM_RPC{nullptr};               // h_Tprop_bklm_rpc
    TH1D* m_hTfly_BKLM_RPC{nullptr};                // h_Tfly_bklm_rpc

    TH1D* m_hTrec_EKLM_Scint{nullptr};              // h_Trec_eklm_scint
    TH1D* m_hTcable_EKLM_Scint{nullptr};            // h_Tcable_eklm_scint
    TH1D* m_hTprop_EKLM_Scint{nullptr};             // h_Tprop_eklm_scint
    TH1D* m_hTfly_EKLM_Scint{nullptr};              // h_Tfly_eklm_scint

    /* --- Truth-corrected timing histograms (in <dir>/truth_corrected) ---
     * Names are identical to uncorrected counterparts but live in a different subdirectory.
     * They are filled only when MC truth Event T0 is available and m_fillTruthCorrectedTiming is true.
     * Component histograms (T_rec, T_cable, T_prop, T_fly) and SEMs are mirrored (no truth subtraction).
     */

    // Per-track T0 distributions (per category, corrected = T0 - T0_true)
    TH1D* m_hT0Trk_BKLM_Scint_corr{nullptr};        // h_t0trk_bklm_scint
    TH1D* m_hT0Trk_BKLM_RPC_corr{nullptr};          // h_t0trk_bklm_rpc
    TH1D* m_hT0Trk_EKLM_Scint_corr{nullptr};        // h_t0trk_eklm_scint

    // Per-event T0 (track-average, corrected) MEAN and mirrored SEM
    TH1D* m_hT0Evt_TrkAvg_BKLM_Scint_corr{nullptr}; // h_t0evt_trkavg_bklm_scint
    TH1D* m_hT0Evt_TrkAvg_BKLM_RPC_corr{nullptr};   // h_t0evt_trkavg_bklm_rpc
    TH1D* m_hT0Evt_TrkAvg_EKLM_Scint_corr{nullptr}; // h_t0evt_trkavg_eklm_scint
    TH1D* m_hT0Evt_TrkAvg_All_corr{nullptr};        // h_t0evt_trkavg_all
    TH1D* m_hT0Evt_TrkAvg_BKLM_Scint_SEM_corr{nullptr}; // h_t0evt_trkavg_bklm_scint_sem
    TH1D* m_hT0Evt_TrkAvg_BKLM_RPC_SEM_corr{nullptr};   // h_t0evt_trkavg_bklm_rpc_sem
    TH1D* m_hT0Evt_TrkAvg_EKLM_Scint_SEM_corr{nullptr}; // h_t0evt_trkavg_eklm_scint_sem
    TH1D* m_hT0Evt_TrkAvg_All_SEM_corr{nullptr};        // h_t0evt_trkavg_all_sem

    // Per-event T0 (hit-average, corrected) MEAN and mirrored SEM
    TH1D* m_hT0Evt_HitAvg_BKLM_Scint_corr{nullptr}; // h_t0evt_hitavg_bklm_scint
    TH1D* m_hT0Evt_HitAvg_BKLM_RPC_corr{nullptr};   // h_t0evt_hitavg_bklm_rpc
    TH1D* m_hT0Evt_HitAvg_EKLM_Scint_corr{nullptr}; // h_t0evt_hitavg_eklm_scint
    TH1D* m_hT0Evt_HitAvg_All_corr{nullptr};        // h_t0evt_hitavg_all
    TH1D* m_hT0Evt_HitAvg_BKLM_Scint_SEM_corr{nullptr}; // h_t0evt_hitavg_bklm_scint_sem
    TH1D* m_hT0Evt_HitAvg_BKLM_RPC_SEM_corr{nullptr};   // h_t0evt_hitavg_bklm_rpc_sem
    TH1D* m_hT0Evt_HitAvg_EKLM_Scint_SEM_corr{nullptr}; // h_t0evt_hitavg_eklm_scint_sem
    TH1D* m_hT0Evt_HitAvg_All_SEM_corr{nullptr};        // h_t0evt_hitavg_all_sem

    // Per-digit timing components (truth-corrected dir; mirrored values)
    TH1D* m_hTrec_BKLM_Scint_corr{nullptr};         // h_Trec_bklm_scint
    TH1D* m_hTcable_BKLM_Scint_corr{nullptr};       // h_Tcable_bklm_scint
    TH1D* m_hTprop_BKLM_Scint_corr{nullptr};        // h_Tprop_bklm_scint
    TH1D* m_hTfly_BKLM_Scint_corr{nullptr};         // h_Tfly_bklm_scint

    TH1D* m_hTrec_BKLM_RPC_corr{nullptr};           // h_Trec_bklm_rpc
    TH1D* m_hTcable_BKLM_RPC_corr{nullptr};         // h_Tcable_bklm_rpc
    TH1D* m_hTprop_BKLM_RPC_corr{nullptr};          // h_Tprop_bklm_rpc
    TH1D* m_hTfly_BKLM_RPC_corr{nullptr};           // h_Tfly_bklm_rpc

    TH1D* m_hTrec_EKLM_Scint_corr{nullptr};         // h_Trec_eklm_scint
    TH1D* m_hTcable_EKLM_Scint_corr{nullptr};       // h_Tcable_eklm_scint
    TH1D* m_hTprop_EKLM_Scint_corr{nullptr};        // h_Tprop_eklm_scint
    TH1D* m_hTfly_EKLM_Scint_corr{nullptr};         // h_Tfly_eklm_scint

    /* ---------- Pull histograms (uncorrected) ---------- */

    // Same-subdetector pulls: (T0_i − T0_j) / sqrt(SEM_i^2 + SEM_j^2)
    TH1D* m_hPull_BKLM_Scint{nullptr};              // h_pull_bklm_scint
    TH1D* m_hPull_BKLM_RPC{nullptr};                // h_pull_bklm_rpc (combined phi+z for backward compat)
    TH1D* m_hPull_BKLM_RPC_Phi{nullptr};            // h_pull_bklm_rpc_phi (direction-specific)
    TH1D* m_hPull_BKLM_RPC_Z{nullptr};              // h_pull_bklm_rpc_z (direction-specific)
    TH1D* m_hPull_EKLM_Scint{nullptr};              // h_pull_eklm_scint

    // Cross-subdetector pulls
    TH1D* m_hPull_B_vs_E{nullptr};                  // h_pull_bklm_scint_vs_eklm_scint
    TH1D* m_hPull_B_vs_R{nullptr};                  // h_pull_bklm_scint_vs_bklm_rpc
    TH1D* m_hPull_E_vs_R{nullptr};                  // h_pull_eklm_scint_vs_bklm_rpc

    /* ---------- Pairwise sector pull histograms ---------- */

    // Sector counts for pairwise analysis
    static constexpr int c_nBKLMSectors = 8;        // BKLM sectors 0-7
    static constexpr int c_nEKLMSectors = 4;        // EKLM sectors 1-4

    // Pairwise sector pull histograms: pull_sector_i_vs_sector_j
    // For each pair (i,j), stores pulls computed from track1 hits in sector i vs track2 hits in sector j
    // BKLM Scintillator: 8x8 matrix
    TH1D* m_hPullPairwise_BScint[c_nBKLMSectors][c_nBKLMSectors] {};
    // EKLM Scintillator cross-endcap: Forward sector vs Backward sector (4x4 matrix)
    // Index [i][j] = pull for track in forward sector (i+1) vs track in backward sector (j+1)
    TH1D* m_hPullPairwise_EScint_FwdVsBwd[c_nEKLMSectors][c_nEKLMSectors] {};
    // RPC Phi: 8x8 matrix
    TH1D* m_hPullPairwise_RPC_Phi[c_nBKLMSectors][c_nBKLMSectors] {};
    // RPC Z: 8x8 matrix
    TH1D* m_hPullPairwise_RPC_Z[c_nBKLMSectors][c_nBKLMSectors] {};

    // Summary 2D histograms: bin (i,j) contains fitted mean/sigma from pairwise histogram
    // BKLM Scintillator
    TH2D* m_h2PullSummary_BScint_Mean{nullptr};     // Fitted Gaussian mean for each (sector_i, sector_j) pair
    TH2D* m_h2PullSummary_BScint_Sigma{nullptr};    // Fitted Gaussian sigma for each (sector_i, sector_j) pair
    // EKLM Scintillator cross-endcap: Forward sector (X-axis) vs Backward sector (Y-axis)
    TH2D* m_h2PullSummary_EScint_FwdVsBwd_Mean{nullptr};
    TH2D* m_h2PullSummary_EScint_FwdVsBwd_Sigma{nullptr};
    // RPC Phi
    TH2D* m_h2PullSummary_RPC_Phi_Mean{nullptr};
    TH2D* m_h2PullSummary_RPC_Phi_Sigma{nullptr};
    // RPC Z
    TH2D* m_h2PullSummary_RPC_Z_Mean{nullptr};
    TH2D* m_h2PullSummary_RPC_Z_Sigma{nullptr};

    // Overall pull summary: one bin per detector category (4 bins)
    TH1D* m_hPullSummary_Mean{nullptr};             // Mean of each overall pull distribution
    TH1D* m_hPullSummary_Width{nullptr};            // Sigma of each overall pull distribution

    /* ---------- Residual histograms (uncorrected) ---------- */
    // Mirror of pull histograms but without normalization: ΔT0 = T0_i − T0_j

    // Same-subdetector residuals
    TH1D* m_hResidual_BKLM_Scint{nullptr};          // h_residual_bklm_scint
    TH1D* m_hResidual_BKLM_RPC{nullptr};            // h_residual_bklm_rpc (combined phi+z)
    TH1D* m_hResidual_BKLM_RPC_Phi{nullptr};        // h_residual_bklm_rpc_phi (direction-specific)
    TH1D* m_hResidual_BKLM_RPC_Z{nullptr};          // h_residual_bklm_rpc_z (direction-specific)
    TH1D* m_hResidual_EKLM_Scint{nullptr};          // h_residual_eklm_scint

    // Cross-subdetector residuals
    TH1D* m_hResidual_B_vs_E{nullptr};              // h_residual_bklm_scint_vs_eklm_scint
    TH1D* m_hResidual_B_vs_R{nullptr};              // h_residual_bklm_scint_vs_bklm_rpc
    TH1D* m_hResidual_E_vs_R{nullptr};              // h_residual_eklm_scint_vs_bklm_rpc

    // Pairwise sector residual histograms (same structure as pulls)
    TH1D* m_hResidualPairwise_BScint[c_nBKLMSectors][c_nBKLMSectors] {};
    TH1D* m_hResidualPairwise_EScint_FwdVsBwd[c_nEKLMSectors][c_nEKLMSectors] {};
    TH1D* m_hResidualPairwise_RPC_Phi[c_nBKLMSectors][c_nBKLMSectors] {};
    TH1D* m_hResidualPairwise_RPC_Z[c_nBKLMSectors][c_nBKLMSectors] {};

    // Summary 2D histograms for residuals
    TH2D* m_h2ResidualSummary_BScint_Mean{nullptr};
    TH2D* m_h2ResidualSummary_BScint_Sigma{nullptr};
    TH2D* m_h2ResidualSummary_EScint_FwdVsBwd_Mean{nullptr};
    TH2D* m_h2ResidualSummary_EScint_FwdVsBwd_Sigma{nullptr};
    TH2D* m_h2ResidualSummary_RPC_Phi_Mean{nullptr};
    TH2D* m_h2ResidualSummary_RPC_Phi_Sigma{nullptr};
    TH2D* m_h2ResidualSummary_RPC_Z_Mean{nullptr};
    TH2D* m_h2ResidualSummary_RPC_Z_Sigma{nullptr};

    // Overall residual summary: one bin per detector category (4 bins)
    TH1D* m_hResidualSummary_Mean{nullptr};         // Mean of each overall residual distribution
    TH1D* m_hResidualSummary_Width{nullptr};        // Sigma of each overall residual distribution

    /* ---------- Cross-detector ΔT0 analysis (by region) ---------- */
    // Regions: 0=EKLM Backward, 1=EKLM Forward, 2=BKLM RPC, 3=BKLM Scint
    // This answers: "Are there biases when tracks are in different subdetectors?"

    static constexpr int c_nDetectorRegions = 4;    // EKLM Bwd, EKLM Fwd, BKLM RPC, BKLM Scint

    // 4x4 matrix of ΔT0 histograms for each region combination
    // [i][j] = ΔT0 for track in region i vs track in region j
    TH1D* m_hDeltaT0_DetectorCombo[c_nDetectorRegions][c_nDetectorRegions] {};

    // 2D summary histograms: fitted mean and sigma for each region combination
    TH2D* m_h2DeltaT0_DetectorCombo_Mean{nullptr};
    TH2D* m_h2DeltaT0_DetectorCombo_Sigma{nullptr};
    TH2D* m_h2DeltaT0_DetectorCombo_Entries{nullptr};

    /* ---------- Multiple Final EventT0 combinations ---------- */

    // Scintillator only (BKLM Scint + EKLM Scint) - default, no RPC
    TH1D* m_hT0Evt_Final_ScintOnly{nullptr};        // h_t0evt_final_scint_only
    TH1D* m_hT0Evt_Final_ScintOnly_SEM{nullptr};    // h_t0evt_final_scint_only_sem

    // Scintillator + RPC combined (all detectors, RPC phi+z combined)
    TH1D* m_hT0Evt_Final_WithRPC{nullptr};          // h_t0evt_final_with_rpc
    TH1D* m_hT0Evt_Final_WithRPC_SEM{nullptr};      // h_t0evt_final_with_rpc_sem

    // Scintillator + RPC with directions separate (phi and z as separate categories)
    TH1D* m_hT0Evt_Final_WithRPCDir{nullptr};       // h_t0evt_final_with_rpc_dir
    TH1D* m_hT0Evt_Final_WithRPCDir_SEM{nullptr};   // h_t0evt_final_with_rpc_dir_sem

    // Truth-corrected versions (MC only)
    TH1D* m_hT0Evt_Final_ScintOnly_corr{nullptr};
    TH1D* m_hT0Evt_Final_WithRPC_corr{nullptr};
    TH1D* m_hT0Evt_Final_WithRPCDir_corr{nullptr};
  };

} // namespace Belle2