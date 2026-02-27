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
#include <map>
#include <string>
#include <utility>

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
   *   - A single KLM EventT0 component per event (combination of all available
   *     categories: BKLM-Scint, BKLM-RPC, EKLM-Scint) saved via
   *     addTemporaryEventT0 and setEventT0.
   *   - Monitoring histograms under histogramDirectoryName with subdirectories:
   *       per_track/  — per-track T0 by detector category
   *       per_event/  — per-event averages (track-avg and hit-avg) and SEMs
   *       diagnostics/ — hit/digit counting and timing component breakdowns
   *       final/      — final combined EventT0 and source audit
   *
   * Validation histograms (pulls, residuals, dimuon ΔT0, pairwise sector
   * analysis, cross-detector ΔT0) are handled separately by
   * KLMEventT0ValidationModule in klm/validation/KLMEventT0Validation.py.
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
                        double& sumW_new, double& sumWT_new);

    /**
     * Accumulate BKLM scintillator per-digit T0 estimates.
     */
    void accumulateBKLMScint(RelationVector<KLMHit2d>&, const ExtMap&,
                             double& sumW, double& sumWT, double& sumWT2,
                             double& sumW_new, double& sumWT_new);

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
     */
    void accumulateBKLMRPCFiltered(RelationVector<KLMHit2d>& klmHit2ds, const ExtMap& rpcMap,
                                   bool acceptPhi,
                                   double& sumW, double& sumWT, double& sumWT2,
                                   double& sumW_new, double& sumWT_new);

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

    /** Final KLM averaging mode: "track" (default) or "hit". */
    std::string m_FinalAverageMode{"track"};

    /** If true, include BKLM-RPC in the final KLM weighting (default: false). */
    bool m_FinalUseRPCInKLM{false};

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

    /* Monitoring histograms. */

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
  };

} // namespace Belle2
