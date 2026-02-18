/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/RelationVector.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/gearbox/Unit.h>

/* C++ */
#include <map>
#include <string>
#include <utility>
#include <limits>

/* ROOT forward declarations. */
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
    /* Local helpers. */

    /** Multimap of ExtHit objects keyed by channel or module number. */
    using ExtMap  = std::multimap<unsigned int, Belle2::ExtHit>;

    /** Pair of entry and exit ExtHit pointers. */
    using ExtPair = std::pair<Belle2::ExtHit*, Belle2::ExtHit*>;

    double m_ADCCut_BKLM_Scint_Min;  /**< Minimum ADC cut for BKLM scintillator */
    double m_ADCCut_BKLM_Scint_Max;  /**< Maximum ADC cut for BKLM scintillator */
    double m_ADCCut_EKLM_Scint_Min;  /**< Minimum ADC cut for EKLM scintillator */
    double m_ADCCut_EKLM_Scint_Max;  /**< Maximum ADC cut for EKLM scintillator */

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
     * Note: EKLM section 1=backward (z<0), section 2=forward (z>0).
     */
    void accumulateEKLM(const RelationVector<KLMHit2d>&, const ExtMap&,
                        double& sumW, double& sumWT, double& sumWT2,
                        double& sumW_new, double& sumWT_new,
                        int& nHits, double& sumPropDist, double& sumLayer,
                        int& sumSection, int& sumSector);

    /**
     * Accumulate BKLM scintillator per-digit T0 estimates.
     */
    void accumulateBKLMScint(RelationVector<KLMHit2d>&, const ExtMap&,
                             double& sumW, double& sumWT, double& sumWT2,
                             double& sumW_new, double& sumWT_new,
                             int& nHits, double& sumPropDist, double& sumLayer,
                             int& sumSection, int& sumSector);

    /**
     * Accumulate BKLM RPC per-digit T0 estimates (filtered by readout direction).
     * @param[in]  klmHit2ds    KLM 2D hits associated with the track.
     * @param[in]  rpcMap       Map of extrapolated RPC hits keyed by module number.
     * @param[in]  acceptPhi    If true, accumulate phi-readout hits; if false, z-readout.
     * @param[out] sumW         Sum of inverse-variance weights.
     * @param[out] sumWT        Sum of weight times time.
     * @param[out] sumWT2       Sum of weight times time squared.
     * @param[out] sumW_new     Sum of calibrated inverse-variance weights.
     * @param[out] sumWT_new    Sum of calibrated weight times time.
     * @param[out] nHits        Number of accepted hits.
     * @param[out] sumPropDist  Sum of propagation distances [cm].
     * @param[out] sumLayer     Sum of layer numbers.
     * @param[out] sumSection   Sum of section indices.
     * @param[out] sumSector    Sum of sector indices.
     */
    void accumulateBKLMRPCFiltered(RelationVector<KLMHit2d>& klmHit2ds, const ExtMap& rpcMap,
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

    /** Subdirectory name for uncorrected timing histograms. */
    std::string m_histSubdirUncorr{"uncorrected"};

    /** Subdirectory name for truth-corrected timing histograms. */
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

    /* Geometry and conditions. */

    /** BKLM geometry. */
    Belle2::bklm::GeometryPar* m_geoParB{nullptr};

    /** EKLM geometry data. */
    const Belle2::EKLM::GeometryData* m_geoParE{nullptr};

    /** EKLM strip transformation data. */
    Belle2::EKLM::TransformData* m_transformE{nullptr};

    /** Element numbering helpers. */
    const KLMElementNumbers* m_elementNum{&KLMElementNumbers::Instance()};

    /** Channel status (Normal/Dead/etc.). */
    DBObjPtr<KLMChannelStatus> m_channelStatus;

    /** NEW: Per-hit time resolution for EventT0 estimation. */
    DBObjPtr<KLMEventT0HitResolution> m_eventT0HitResolution;

    /* Inputs. */

    /** Selected muon particle list. */
    StoreObjPtr<ParticleList> m_MuonList;

    /** Reconstructed tracks. */
    StoreArray<Track> m_tracks;

    /* Working buffers. */

    /** Extrapolated hits keyed by channel number (scintillator). */
    ExtMap m_extScint;

    /** Extrapolated hits keyed by module number (RPC). */
    ExtMap m_extRPC;

    /** Optional seed from CDC (for logging only). */
    double m_seedT0{0.0};

    /** Cached truth Event T0 for this event (ns). NAN if not available. */
    double m_truthT0{std::numeric_limits<double>::quiet_NaN()};

    /* Monitoring histograms. */

    /** Number of KLM 2D hits per track. */
    TH1I* m_hNumKLM2DPerTrack{nullptr};

    /** Number of digits per BKLM RPC 1D hit. */
    TH1I* m_hNumDigitsPerB1dRPC{nullptr};

    /** Number of digits per BKLM scintillator 1D hit. */
    TH1I* m_hNumDigitsPerB1dScint{nullptr};

    /** Number of digits per EKLM scintillator 2D hit. */
    TH1I* m_hNumDigitsPerE2dScint{nullptr};

    /** ADC charge distribution for BKLM scintillator digits. */
    TH1D* m_hDigitCharge_BKLM_Scint{nullptr};

    /** ADC charge distribution for EKLM scintillator digits. */
    TH1D* m_hDigitCharge_EKLM_Scint{nullptr};

    /* Per-track diagnostics. */

    /** Number of BKLM scintillator hits per track. */
    TH1I* m_hNHits_PerTrack_BKLM_Scint{nullptr};

    /** Number of EKLM scintillator hits per track. */
    TH1I* m_hNHits_PerTrack_EKLM_Scint{nullptr};

    /** T0 standard error of mean per track for BKLM scintillator [ns]. */
    TH1D* m_hSEM_PerTrack_BKLM_Scint{nullptr};

    /** T0 standard error of mean per track for EKLM scintillator [ns]. */
    TH1D* m_hSEM_PerTrack_EKLM_Scint{nullptr};

    /** Sample type counter (bin 1: Data, bin 2: MC). */
    TH1I* m_hSampleType{nullptr};

    /** MC truth Event T0 [ns]. */
    TH1D* m_hTruthT0{nullptr};

    /** Per-track T0 for BKLM scintillator [ns]. */
    TH1D* m_hT0Trk_BKLM_Scint{nullptr};

    /** Per-track T0 for BKLM RPC [ns]. */
    TH1D* m_hT0Trk_BKLM_RPC{nullptr};

    /** Per-track T0 for EKLM scintillator [ns]. */
    TH1D* m_hT0Trk_EKLM_Scint{nullptr};

    /** Per-event T0 track-average for BKLM scintillator (mean) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_BKLM_Scint{nullptr};

    /** Per-event T0 track-average for BKLM RPC (mean) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_BKLM_RPC{nullptr};

    /** Per-event T0 track-average for EKLM scintillator (mean) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_EKLM_Scint{nullptr};

    /** Per-event T0 track-average combined (mean) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_All{nullptr};

    /** Per-event T0 track-average for BKLM scintillator (SEM) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_BKLM_Scint_SEM{nullptr};

    /** Per-event T0 track-average for BKLM RPC (SEM) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_BKLM_RPC_SEM{nullptr};

    /** Per-event T0 track-average for EKLM scintillator (SEM) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_EKLM_Scint_SEM{nullptr};

    /** Per-event T0 track-average combined (SEM) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_All_SEM{nullptr};

    /** Per-event T0 hit-average for BKLM scintillator (mean) [ns]. */
    TH1D* m_hT0Evt_HitAvg_BKLM_Scint{nullptr};

    /** Per-event T0 hit-average for BKLM RPC (mean) [ns]. */
    TH1D* m_hT0Evt_HitAvg_BKLM_RPC{nullptr};

    /** Per-event T0 hit-average for EKLM scintillator (mean) [ns]. */
    TH1D* m_hT0Evt_HitAvg_EKLM_Scint{nullptr};

    /** Per-event T0 hit-average combined (mean) [ns]. */
    TH1D* m_hT0Evt_HitAvg_All{nullptr};

    /** Per-event T0 hit-average for BKLM scintillator (SEM) [ns]. */
    TH1D* m_hT0Evt_HitAvg_BKLM_Scint_SEM{nullptr};

    /** Per-event T0 hit-average for BKLM RPC (SEM) [ns]. */
    TH1D* m_hT0Evt_HitAvg_BKLM_RPC_SEM{nullptr};

    /** Per-event T0 hit-average for EKLM scintillator (SEM) [ns]. */
    TH1D* m_hT0Evt_HitAvg_EKLM_Scint_SEM{nullptr};

    /** Per-event T0 hit-average combined (SEM) [ns]. */
    TH1D* m_hT0Evt_HitAvg_All_SEM{nullptr};

    /** Final EventT0 source selection (7 bins). */
    TH1I* m_hFinalSource{nullptr};

    /** Dimuon delta-T0 distribution [ns]. */
    TH1D* m_hDimuonDeltaT0{nullptr};

    /** Dimuon delta-T0 for BKLM scintillator [ns]. */
    TH1D* m_hDimuonDeltaT0_B{nullptr};

    /** Dimuon delta-T0 for BKLM RPC [ns]. */
    TH1D* m_hDimuonDeltaT0_R{nullptr};

    /** Dimuon delta-T0 for EKLM scintillator [ns]. */
    TH1D* m_hDimuonDeltaT0_E{nullptr};

    /** Dimuon delta-T0 for scintillator-only combination [ns]. */
    TH1D* m_hDimuonDeltaT0_ScintOnly{nullptr};

    /** Dimuon delta-T0 with all detectors [ns]. */
    TH1D* m_hDimuonDeltaT0_WithRPC{nullptr};

    /** Dimuon delta-T0 with RPC phi/z separate [ns]. */
    TH1D* m_hDimuonDeltaT0_WithRPCDir{nullptr};

    /** Per-track T0 resolution from dimuon fits [ns]. */
    TH1D* m_hPerTrackT0Resolution{nullptr};

    /** Per-event T0 resolution from dimuon fits [ns]. */
    TH1D* m_hEventT0Resolution{nullptr};

    /** Reconstructed time for BKLM scintillator digits [ns]. */
    TH1D* m_hTrec_BKLM_Scint{nullptr};

    /** Cable delay for BKLM scintillator digits [ns]. */
    TH1D* m_hTcable_BKLM_Scint{nullptr};

    /** Propagation time for BKLM scintillator digits [ns]. */
    TH1D* m_hTprop_BKLM_Scint{nullptr};

    /** Flight time for BKLM scintillator digits [ns]. */
    TH1D* m_hTfly_BKLM_Scint{nullptr};

    /** Reconstructed time for BKLM RPC digits [ns]. */
    TH1D* m_hTrec_BKLM_RPC{nullptr};

    /** Cable delay for BKLM RPC digits [ns]. */
    TH1D* m_hTcable_BKLM_RPC{nullptr};

    /** Propagation time for BKLM RPC digits [ns]. */
    TH1D* m_hTprop_BKLM_RPC{nullptr};

    /** Flight time for BKLM RPC digits [ns]. */
    TH1D* m_hTfly_BKLM_RPC{nullptr};

    /** Reconstructed time for EKLM scintillator digits [ns]. */
    TH1D* m_hTrec_EKLM_Scint{nullptr};

    /** Cable delay for EKLM scintillator digits [ns]. */
    TH1D* m_hTcable_EKLM_Scint{nullptr};

    /** Propagation time for EKLM scintillator digits [ns]. */
    TH1D* m_hTprop_EKLM_Scint{nullptr};

    /** Flight time for EKLM scintillator digits [ns]. */
    TH1D* m_hTfly_EKLM_Scint{nullptr};

    /** Truth-corrected per-track T0 for BKLM scintillator [ns]. */
    TH1D* m_hT0Trk_BKLM_Scint_corr{nullptr};

    /** Truth-corrected per-track T0 for BKLM RPC [ns]. */
    TH1D* m_hT0Trk_BKLM_RPC_corr{nullptr};

    /** Truth-corrected per-track T0 for EKLM scintillator [ns]. */
    TH1D* m_hT0Trk_EKLM_Scint_corr{nullptr};

    /** Truth-corrected per-event T0 track-average for BKLM scintillator (mean) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_BKLM_Scint_corr{nullptr};

    /** Truth-corrected per-event T0 track-average for BKLM RPC (mean) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_BKLM_RPC_corr{nullptr};

    /** Truth-corrected per-event T0 track-average for EKLM scintillator (mean) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_EKLM_Scint_corr{nullptr};

    /** Truth-corrected per-event T0 track-average combined (mean) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_All_corr{nullptr};

    /** Truth-corrected per-event T0 track-average for BKLM scintillator (SEM) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_BKLM_Scint_SEM_corr{nullptr};

    /** Truth-corrected per-event T0 track-average for BKLM RPC (SEM) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_BKLM_RPC_SEM_corr{nullptr};

    /** Truth-corrected per-event T0 track-average for EKLM scintillator (SEM) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_EKLM_Scint_SEM_corr{nullptr};

    /** Truth-corrected per-event T0 track-average combined (SEM) [ns]. */
    TH1D* m_hT0Evt_TrkAvg_All_SEM_corr{nullptr};

    /** Truth-corrected per-event T0 hit-average for BKLM scintillator (mean) [ns]. */
    TH1D* m_hT0Evt_HitAvg_BKLM_Scint_corr{nullptr};

    /** Truth-corrected per-event T0 hit-average for BKLM RPC (mean) [ns]. */
    TH1D* m_hT0Evt_HitAvg_BKLM_RPC_corr{nullptr};

    /** Truth-corrected per-event T0 hit-average for EKLM scintillator (mean) [ns]. */
    TH1D* m_hT0Evt_HitAvg_EKLM_Scint_corr{nullptr};

    /** Truth-corrected per-event T0 hit-average combined (mean) [ns]. */
    TH1D* m_hT0Evt_HitAvg_All_corr{nullptr};

    /** Truth-corrected per-event T0 hit-average for BKLM scintillator (SEM) [ns]. */
    TH1D* m_hT0Evt_HitAvg_BKLM_Scint_SEM_corr{nullptr};

    /** Truth-corrected per-event T0 hit-average for BKLM RPC (SEM) [ns]. */
    TH1D* m_hT0Evt_HitAvg_BKLM_RPC_SEM_corr{nullptr};

    /** Truth-corrected per-event T0 hit-average for EKLM scintillator (SEM) [ns]. */
    TH1D* m_hT0Evt_HitAvg_EKLM_Scint_SEM_corr{nullptr};

    /** Truth-corrected per-event T0 hit-average combined (SEM) [ns]. */
    TH1D* m_hT0Evt_HitAvg_All_SEM_corr{nullptr};

    /** Truth-corrected reconstructed time for BKLM scintillator digits [ns]. */
    TH1D* m_hTrec_BKLM_Scint_corr{nullptr};

    /** Truth-corrected cable delay for BKLM scintillator digits [ns]. */
    TH1D* m_hTcable_BKLM_Scint_corr{nullptr};

    /** Truth-corrected propagation time for BKLM scintillator digits [ns]. */
    TH1D* m_hTprop_BKLM_Scint_corr{nullptr};

    /** Truth-corrected flight time for BKLM scintillator digits [ns]. */
    TH1D* m_hTfly_BKLM_Scint_corr{nullptr};

    /** Truth-corrected reconstructed time for BKLM RPC digits [ns]. */
    TH1D* m_hTrec_BKLM_RPC_corr{nullptr};

    /** Truth-corrected cable delay for BKLM RPC digits [ns]. */
    TH1D* m_hTcable_BKLM_RPC_corr{nullptr};

    /** Truth-corrected propagation time for BKLM RPC digits [ns]. */
    TH1D* m_hTprop_BKLM_RPC_corr{nullptr};

    /** Truth-corrected flight time for BKLM RPC digits [ns]. */
    TH1D* m_hTfly_BKLM_RPC_corr{nullptr};

    /** Truth-corrected reconstructed time for EKLM scintillator digits [ns]. */
    TH1D* m_hTrec_EKLM_Scint_corr{nullptr};

    /** Truth-corrected cable delay for EKLM scintillator digits [ns]. */
    TH1D* m_hTcable_EKLM_Scint_corr{nullptr};

    /** Truth-corrected propagation time for EKLM scintillator digits [ns]. */
    TH1D* m_hTprop_EKLM_Scint_corr{nullptr};

    /** Truth-corrected flight time for EKLM scintillator digits [ns]. */
    TH1D* m_hTfly_EKLM_Scint_corr{nullptr};

    /* Pull histograms. */

    /** Pull distribution for BKLM scintillator. */
    TH1D* m_hPull_BKLM_Scint{nullptr};

    /** Pull distribution for BKLM RPC (phi+z combined). */
    TH1D* m_hPull_BKLM_RPC{nullptr};

    /** Pull distribution for BKLM RPC phi-readout. */
    TH1D* m_hPull_BKLM_RPC_Phi{nullptr};

    /** Pull distribution for BKLM RPC z-readout. */
    TH1D* m_hPull_BKLM_RPC_Z{nullptr};

    /** Pull distribution for EKLM scintillator. */
    TH1D* m_hPull_EKLM_Scint{nullptr};

    /** Pull distribution: BKLM scintillator vs EKLM scintillator. */
    TH1D* m_hPull_B_vs_E{nullptr};

    /** Pull distribution: BKLM scintillator vs BKLM RPC. */
    TH1D* m_hPull_B_vs_R{nullptr};

    /** Pull distribution: EKLM scintillator vs BKLM RPC. */
    TH1D* m_hPull_E_vs_R{nullptr};

    /* Pairwise sector pull histograms. */

    /** Number of BKLM sectors. */
    static constexpr int c_nBKLMSectors = 8;

    /** Number of EKLM sectors per endcap. */
    static constexpr int c_nEKLMSectors = 4;

    /** Pairwise sector pull histograms for BKLM scintillator. */
    TH1D* m_hPullPairwise_BScint[c_nBKLMSectors][c_nBKLMSectors] {};

    /** Pairwise sector pull histograms for EKLM scintillator (forward vs backward). */
    TH1D* m_hPullPairwise_EScint_FwdVsBwd[c_nEKLMSectors][c_nEKLMSectors] {};

    /** Pairwise sector pull histograms for BKLM RPC phi-readout. */
    TH1D* m_hPullPairwise_RPC_Phi[c_nBKLMSectors][c_nBKLMSectors] {};

    /** Pairwise sector pull histograms for BKLM RPC z-readout. */
    TH1D* m_hPullPairwise_RPC_Z[c_nBKLMSectors][c_nBKLMSectors] {};

    /** Pull summary 2D: fitted mean per sector pair (BKLM scintillator). */
    TH2D* m_h2PullSummary_BScint_Mean{nullptr};

    /** Pull summary 2D: fitted sigma per sector pair (BKLM scintillator). */
    TH2D* m_h2PullSummary_BScint_Sigma{nullptr};

    /** Pull summary 2D: fitted mean per sector pair (EKLM fwd vs bwd). */
    TH2D* m_h2PullSummary_EScint_FwdVsBwd_Mean{nullptr};

    /** Pull summary 2D: fitted sigma per sector pair (EKLM fwd vs bwd). */
    TH2D* m_h2PullSummary_EScint_FwdVsBwd_Sigma{nullptr};

    /** Pull summary 2D: fitted mean per sector pair (RPC phi). */
    TH2D* m_h2PullSummary_RPC_Phi_Mean{nullptr};

    /** Pull summary 2D: fitted sigma per sector pair (RPC phi). */
    TH2D* m_h2PullSummary_RPC_Phi_Sigma{nullptr};

    /** Pull summary 2D: fitted mean per sector pair (RPC z). */
    TH2D* m_h2PullSummary_RPC_Z_Mean{nullptr};

    /** Pull summary 2D: fitted sigma per sector pair (RPC z). */
    TH2D* m_h2PullSummary_RPC_Z_Sigma{nullptr};

    /** Overall pull summary: mean per detector category. */
    TH1D* m_hPullSummary_Mean{nullptr};

    /** Overall pull summary: sigma per detector category. */
    TH1D* m_hPullSummary_Width{nullptr};

    /* Residual histograms. */

    /** Residual (delta-T0) for BKLM scintillator [ns]. */
    TH1D* m_hResidual_BKLM_Scint{nullptr};

    /** Residual (delta-T0) for BKLM RPC (phi+z combined) [ns]. */
    TH1D* m_hResidual_BKLM_RPC{nullptr};

    /** Residual (delta-T0) for BKLM RPC phi-readout [ns]. */
    TH1D* m_hResidual_BKLM_RPC_Phi{nullptr};

    /** Residual (delta-T0) for BKLM RPC z-readout [ns]. */
    TH1D* m_hResidual_BKLM_RPC_Z{nullptr};

    /** Residual (delta-T0) for EKLM scintillator [ns]. */
    TH1D* m_hResidual_EKLM_Scint{nullptr};

    /** Residual: BKLM scintillator vs EKLM scintillator [ns]. */
    TH1D* m_hResidual_B_vs_E{nullptr};

    /** Residual: BKLM scintillator vs BKLM RPC [ns]. */
    TH1D* m_hResidual_B_vs_R{nullptr};

    /** Residual: EKLM scintillator vs BKLM RPC [ns]. */
    TH1D* m_hResidual_E_vs_R{nullptr};

    /** Pairwise sector residual histograms for BKLM scintillator [ns]. */
    TH1D* m_hResidualPairwise_BScint[c_nBKLMSectors][c_nBKLMSectors] {};

    /** Pairwise sector residual histograms for EKLM scintillator (forward vs backward) [ns]. */
    TH1D* m_hResidualPairwise_EScint_FwdVsBwd[c_nEKLMSectors][c_nEKLMSectors] {};

    /** Pairwise sector residual histograms for BKLM RPC phi-readout [ns]. */
    TH1D* m_hResidualPairwise_RPC_Phi[c_nBKLMSectors][c_nBKLMSectors] {};

    /** Pairwise sector residual histograms for BKLM RPC z-readout [ns]. */
    TH1D* m_hResidualPairwise_RPC_Z[c_nBKLMSectors][c_nBKLMSectors] {};

    /** Residual summary 2D: fitted mean per sector pair (BKLM scintillator) [ns]. */
    TH2D* m_h2ResidualSummary_BScint_Mean{nullptr};

    /** Residual summary 2D: fitted sigma per sector pair (BKLM scintillator) [ns]. */
    TH2D* m_h2ResidualSummary_BScint_Sigma{nullptr};

    /** Residual summary 2D: fitted mean per sector pair (EKLM fwd vs bwd) [ns]. */
    TH2D* m_h2ResidualSummary_EScint_FwdVsBwd_Mean{nullptr};

    /** Residual summary 2D: fitted sigma per sector pair (EKLM fwd vs bwd) [ns]. */
    TH2D* m_h2ResidualSummary_EScint_FwdVsBwd_Sigma{nullptr};

    /** Residual summary 2D: fitted mean per sector pair (RPC phi) [ns]. */
    TH2D* m_h2ResidualSummary_RPC_Phi_Mean{nullptr};

    /** Residual summary 2D: fitted sigma per sector pair (RPC phi) [ns]. */
    TH2D* m_h2ResidualSummary_RPC_Phi_Sigma{nullptr};

    /** Residual summary 2D: fitted mean per sector pair (RPC z) [ns]. */
    TH2D* m_h2ResidualSummary_RPC_Z_Mean{nullptr};

    /** Residual summary 2D: fitted sigma per sector pair (RPC z) [ns]. */
    TH2D* m_h2ResidualSummary_RPC_Z_Sigma{nullptr};

    /** Overall residual summary: mean per detector category [ns]. */
    TH1D* m_hResidualSummary_Mean{nullptr};

    /** Overall residual summary: sigma per detector category [ns]. */
    TH1D* m_hResidualSummary_Width{nullptr};

    /* Cross-detector delta-T0 analysis. */

    /** Number of detector regions for cross-detector analysis. */
    static constexpr int c_nDetectorRegions = 4;

    /** Delta-T0 histograms for each detector region pair [ns]. */
    TH1D* m_hDeltaT0_DetectorCombo[c_nDetectorRegions][c_nDetectorRegions] {};

    /** Delta-T0 summary 2D: fitted mean per detector region pair [ns]. */
    TH2D* m_h2DeltaT0_DetectorCombo_Mean{nullptr};

    /** Delta-T0 summary 2D: fitted sigma per detector region pair [ns]. */
    TH2D* m_h2DeltaT0_DetectorCombo_Sigma{nullptr};

    /** Delta-T0 summary 2D: number of entries per detector region pair. */
    TH2D* m_h2DeltaT0_DetectorCombo_Entries{nullptr};

    /* Final EventT0 combinations. */

    /** Final EventT0 scintillator-only combination (mean) [ns]. */
    TH1D* m_hT0Evt_Final_ScintOnly{nullptr};

    /** Final EventT0 scintillator-only combination (SEM) [ns]. */
    TH1D* m_hT0Evt_Final_ScintOnly_SEM{nullptr};

    /** Final EventT0 with all detectors (mean) [ns]. */
    TH1D* m_hT0Evt_Final_WithRPC{nullptr};

    /** Final EventT0 with all detectors (SEM) [ns]. */
    TH1D* m_hT0Evt_Final_WithRPC_SEM{nullptr};

    /** Final EventT0 with RPC phi/z separate (mean) [ns]. */
    TH1D* m_hT0Evt_Final_WithRPCDir{nullptr};

    /** Final EventT0 with RPC phi/z separate (SEM) [ns]. */
    TH1D* m_hT0Evt_Final_WithRPCDir_SEM{nullptr};

    /** Truth-corrected final EventT0 scintillator-only combination [ns]. */
    TH1D* m_hT0Evt_Final_ScintOnly_corr{nullptr};

    /** Truth-corrected final EventT0 with all detectors [ns]. */
    TH1D* m_hT0Evt_Final_WithRPC_corr{nullptr};

    /** Truth-corrected final EventT0 with RPC phi/z separate [ns]. */
    TH1D* m_hT0Evt_Final_WithRPCDir_corr{nullptr};
  };

} // namespace Belle2
