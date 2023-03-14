/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/calibration/KLMTimeAlgorithm.h>
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/dataobjects/KLMHit2d.h>
#include <klm/dbobjects/KLMChannelStatus.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/eklm/geometry/TransformData.h>

/* Basf2 headers */
#include <calibration/CalibrationCollectorModule.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

/* ROOT headers */
#include <TH1D.h>
#include <TH1I.h>
#include <TH2D.h>
#include <TTree.h>

namespace Belle2 {

  /**
   * Collect hit information for KLM time calibration with CAF.
   */
  class KLMTimeCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor.
     */
    KLMTimeCollectorModule();

    /**
     * Destructor.
     */
    virtual ~KLMTimeCollectorModule();

    /**
     * Initializes the module.
     */
    void prepare() override;

    /**
     * Event action, collect information for calibration.
     */
    void collect() override;

    /**
     * Termination action.
     */
    void finish() override;

  private:

    /**
     * Collect hits information for RPC of BKLM.
     */
    void collectRPC(RelationVector<KLMHit2d>&);

    /**
     * Collect hits information for scintillator of BKLM.
     */
    void collectScint(RelationVector<KLMHit2d>&);

    /**
     * Collect hits information for scintillator of EKLM.
     */
    void collectScintEnd(const RelationVector<KLMHit2d>&);

    /**
     * Match KLM hit and extHit.
     */
    std::pair<ExtHit*, ExtHit*> matchExt(
      KLMChannelNumber channelID, std::multimap<unsigned int, ExtHit>&);

    /** Save position difference betwen matched kLMHit and ExtHit. */
    void storeDistDiff(ROOT::Math::XYZVector&);

    /** Use event T0 or not. */
    bool m_useEvtT0;

    /** Whether to ignore ExtHits with backward propagation. */
    bool m_IgnoreBackwardPropagation;

    /** Input partilce list name */
    std::string m_inputListName;

    /** Map for handle the extHit related to RPC. */
    std::multimap<unsigned int, ExtHit> m_vExtHits_RPC;

    /** Map for handle the extHit related to scint. */
    std::multimap<unsigned int, ExtHit> m_vExtHits;

    /** Global tracks. */
    StoreArray<Track> m_tracks;

    /** Event T0 array. */
    StoreObjPtr<EventT0> m_eventT0;

    /** Time calibration data event. */
    KLMTimeAlgorithm::Event m_Event;

    /** BKLM geometry parameters. */
    const bklm::GeometryPar* m_geoParB;

    /** EKLM geometry parameters. */
    const EKLM::GeometryData* m_geoParE;

    /** Transformation data. */
    EKLM::TransformData* m_TransformData;

    /** KLM element numbers. */
    const KLMElementNumbers* m_elementNum;

    /** Channel status. */
    DBObjPtr<KLMChannelStatus> m_channelStatus;

    /* Monitor histograms. */

    /** Event T0 distribution before track selection. */
    TH1D* m_HeventT0_0;

    /** Event T0 distribution after track selection. */
    TH1D* m_HeventT0_1;

    /** Number of tracks. */
    TH1I* m_HnumTrack;

    /** Number of KLM hits related to track. */
    TH1I* m_HnKLMHit2dOfTrack;

    /** Difference between global and local position. */
    TH1D* m_HpositionDiff;

    /** Difference between global and local position (X). */
    TH1D* m_HpositionXDiff;

    /** Difference between global and local position (Y). */
    TH1D* m_HpositionYDiff;

    /** Difference between global and local position (Z). */
    TH1D* m_HpositionZDiff;

    /** Particle flying time versus detector layers (for BKLM). */
    TH2D* m_HflyTimeB;

    /** Particle flying time versus detector layers (for EKLM). */
    TH2D* m_HflyTimeE;

    /* Sum number of digits collected. */

    /** EKLM parts. */
    TH1I* m_HnumDigit_scint_end;

    /** BKLM scitillator part. */
    TH1I* m_HnumDigit_scint;

    /** BKLM RPC part. */
    TH1I* m_HnumDigit_rpc;

    /** Data collection tree. */
    TTree* m_outTree;

  };
}

