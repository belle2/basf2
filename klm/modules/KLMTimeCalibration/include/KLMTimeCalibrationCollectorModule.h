/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/calibration/KLMTimeCalibrationAlgorithm.h>
#include <klm/bklm/geometry/Module.h>
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/eklm/geometry/TransformData.h>
#include <klm/bklm/dataobjects/BKLMHit2d.h>
#include <klm/eklm/dataobjects/EKLMHit2d.h>

/* Belle2 headers */
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/dataobjects/EventT0.h>
#include <calibration/CalibrationCollectorModule.h>

/* ROOT headers */
#include <TFile.h>
#include <TTree.h>
#include <TH1I.h>
#include <TH2I.h>
#include <TH1D.h>
#include <TH2D.h>


namespace Belle2 {
  /** Collect hit information for BKLM time calibration with CAF */
  class KLMTimeCalibrationCollectorModule : public CalibrationCollectorModule {

  public:
    /** Constructor */
    KLMTimeCalibrationCollectorModule();

    /** Destructor */
    virtual ~KLMTimeCalibrationCollectorModule();

    /** Initializes the Module */
    void prepare() override;

    /** Event action, collect information for calibration */
    void collect() override;

    /** Termination action */
    void finish() override;

  private:
    /** collect hits information for RPC of BKLM */
    void collectRPC(RelationVector<BKLMHit2d>);

    /** collect hits information for scintillator of BKLM */
    void collectScint(RelationVector<BKLMHit2d>);

    /** collect hits information for scintillator of EKLM */
    void collectScintEnd(RelationVector<EKLMHit2d>);

    /** debug flag */
    bool m_Debug;

    /** use event T0 or not. */
    bool m_useEvtT0;

    /** Input partilce list name */
    std::string m_inputListName;

    /** map for handle the extHit, key is the channel id. */
    std::multimap<int, ExtHit*> m_mapExtHits;

    /** Global tracks */
    StoreArray<Track> m_tracks;

    /** Event T0 array */
    StoreObjPtr<EventT0> m_eventT0;

    /** Time calibration data event */
    struct KLMTimeCalibrationAlgorithm::Event m_ev;

    /** BKLM geometry para. */
    bklm::GeometryPar* m_geoParB;

    /** EKLM geometry para. */
    const EKLM::GeometryData* m_geoParE;

    /** Transformation data. */
    EKLM::TransformData* m_TransformData;

    /** KLM element numbers pointer */
    const KLMElementNumbers* m_elementNum;

    /**
     * Monitor histograms
     */
    /** Event T0 distribution before track selection */
    TH1D* m_HeventT0_0;

    /** Event T0 distribution after track selection */
    TH1D* m_HeventT0_1;

    /** Number of tracks */
    TH1I* m_HnumTrack;

    /** Number of BKLM hits related to track */
    TH1I* m_HnBHit2dOfTrack;

    /** Number of EKLM hits related to track */
    TH1I* m_HnEHit2dOfTrack;

    /** Difference between Global and Local position */
    TH1D* m_HpositionDiff;

    /** Difference between Global and Local position (X) */
    TH1D* m_HpositionXDiff;

    /** Difference between Global and Local position (Y) */
    TH1D* m_HpositionYDiff;

    /** Difference between Global and Local position (Z) */
    TH1D* m_HpositionZDiff;

    /** Particle flying time versus detector layers (for BKLM)*/
    TH2D* m_HflyTimeB;

    /** Particle flying time versus detector layers (for EKLM)*/
    TH2D* m_HflyTimeE;

    /** Sum number of digits collected */
    /** EKLM parts */
    TH1I* m_HnumDigit_scint_end;

    /** BKLM scitillator part */
    TH1I* m_HnumDigit_scint;

    /** BKLM RPC part */
    TH1I* m_HnumDigit_rpc;

    /** data collection tree */
    TTree* m_outTree;
  };
}
