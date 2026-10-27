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
#include <framework/dataobjects/EventExtraInfo.h>
#include <framework/gearbox/Unit.h>

/* ROOT headers. */
#include <TDirectory.h>
#include <TH1D.h>
#include <TH1I.h>

/* C++ headers. */
#include <cmath>
#include <vector>
#include <map>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * Estimate per-event T0 using KLM (BKLM scint, BKLM RPC, EKLM scint).
   *
   * Inputs:
   *   - ParticleList (tracks with relations to KLMHit2d and ExtHit)
   *   - KLM conditions (time constants, cable delays, channel status,
   *     calibrated per-hit resolution; all RPC hits use the RPC resolution measured with
   *     the phi and z hits together)
   *   - BKLM/EKLM geometry for propagation distances and transforms
   *
   * Outputs:
   *   - A single KLM EventT0 component per event (combination of all available
   *     categories: BKLM-Scint, BKLM-RPC, EKLM-Scint) added to EventT0 via
   *     addTemporaryEventT0, alongside the other detectors' values. The event T0
   *     itself is never set or changed by this module.
   *   - The per-track result of each category as extra info of the muons in MuonListName:
   *     klmTrackT0<C>, klmTrackT0Err<C>, klmTrackNHits<C>, klmTrackSector<C>, klmTrackSection<C>
   *     for <C> = BKLMScint, BKLMRPC, EKLMScint, and klmTrackT0All, klmTrackT0ErrAll over all
   *     categories of the track.
   *   - The per-event result of each category in EventExtraInfo: klmEventT0<C>, klmEventT0Err<C>.
   *   - Monitoring histograms under histogramDirectoryName with subdirectories:
   *       per_track/  — per-track T0 by detector category
   *       per_event/  — per-event track-averages and SEMs
   *       final/      — final combined EventT0 and source audit
   *
   * Validation histograms (pulls, residuals, dimuon ΔT0, pairwise sector
   * analysis, cross-detector ΔT0) are handled separately by
   * KLMEventT0ValidationModule in klm/scripts/KLMEventT0Validation.py, which reads
   * the extra info above instead of recomputing it.
   */
  class KLMEventT0EstimatorModule : public HistoModule {

  public:
    KLMEventT0EstimatorModule();
    ~KLMEventT0EstimatorModule() override;

    /** Copying is not allowed: the module manages raw pointers. */
    KLMEventT0EstimatorModule(const KLMEventT0EstimatorModule&) = delete;

    /** Copying is not allowed: the module manages raw pointers. */
    KLMEventT0EstimatorModule& operator=(const KLMEventT0EstimatorModule&) = delete;

    /** Definition of histograms (called once by HistoManager). */
    void defineHisto() override;

    /** Register inputs/params; get geometry; call REG_HISTOGRAM. */
    void initialize() override;

    /** Per-run resets if desired (histos remain booked). */
    void beginRun() override;

    /** Per-event algorithm: collect hits, compute residuals, fill outputs. */
    void event() override;

    /** Running sums of the hits of one category on one track. */
    struct CategorySums {
      double sumW{0.0};   /**< Sum of the hit weights 1/sigma^2 [1/ns^2]. */
      double sumWT{0.0};  /**< Sum of weight times hit T0 [1/ns]. */
      int nHits{0};       /**< Number of hits used. */
      int sumSector{0};   /**< Sum of the sector numbers of the hits used. */
      int sumSection{0};  /**< Sum of the section numbers of the hits used. */
    };

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
     * Get per-hit sigma for a digit based on detector category.
     * Uses calibrated per-hit resolution from KLMEventT0HitResolution payload; RPC hits of
     * both readout directions use the RPC resolution measured with phi and z hits together.
     * @param subdetector KLM subdetector (BKLM or EKLM)
     * @param layer Layer number (1-indexed)
     * @param inRPC Whether the digit is from RPC
     * @return Per-hit sigma in ns
     */
    double getHitSigma(int subdetector, int layer, bool inRPC) const;

    /**
     * Accumulate EKLM scintillator per-digit T0 estimates (weighted).
     * Note: EKLM section 1=backward (z<0), section 2=forward (z>0).
     */
    void accumulateEKLM(const RelationVector<KLMHit2d>&, const ExtMap&,
                        CategorySums& sums);

    /**
     * Accumulate BKLM scintillator per-digit T0 estimates (weighted).
     */
    void accumulateBKLMScint(RelationVector<KLMHit2d>&, const ExtMap&,
                             CategorySums& sums);

    /**
     * Accumulate BKLM RPC per-digit T0 estimates (weighted, both readout directions).
     * @param[in]  klmHit2ds    KLM 2D hits associated with the track.
     * @param[in]  rpcMap       Map of extrapolated RPC hits keyed by module number.
     * @param[out] sums         Running sums of the RPC hits of the track.
     */
    void accumulateBKLMRPC(RelationVector<KLMHit2d>& klmHit2ds, const ExtMap& rpcMap,
                           CategorySums& sums);

    /* ---------- Parameters (set via addParam in constructor) ---------- */

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

    /** Per-hit time resolution for EventT0 estimation. */
    DBObjPtr<KLMEventT0HitResolution> m_eventT0HitResolution;

    /* Inputs. */

    /** Selected muon particle list. */
    StoreObjPtr<ParticleList> m_MuonList;

    /** Reconstructed tracks. */
    StoreArray<Track> m_tracks;

    /** Event extra info, receives the per-event result of each category. */
    StoreObjPtr<EventExtraInfo> m_eventExtraInfo;

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

    /** Final EventT0 source selection (7 bins). */
    TH1I* m_hFinalSource{nullptr};
  };

} // namespace Belle2
