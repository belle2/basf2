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

/* C++ headers */
#include <string>

/* ROOT headers */
#include <TFile.h>
#include <TTree.h>
#include <TH1I.h>
#include <TH2I.h>
#include <TH1D.h>
#include <TH2D.h>

/* Belle2 headers */
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/ExtHit.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/dataobjects/EventT0.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/ExtHit.h>
#include <calibration/CalibrationCollectorModule.h>

#include <klm/calibration/KLMTimeCalibrationAlgorithm.h>

#include <klm/bklm/geometry/Module.h>
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/eklm/geometry/TransformData.h>

#include <klm/bklm/dataobjects/BKLMHit2d.h>
#include <klm/eklm/dataobjects/EKLMHit2d.h>


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
    /** collect hits information for RPC of BKLM **/
    void collect_RPC(RelationVector<BKLMHit2d>);

    /** collect hits information for scintillator of BKLM **/
    void collect_scint(RelationVector<BKLMHit2d>);

    /** collect hits information for scintillator of EKLM **/
    void collect_scint_end(RelationVector<EKLMHit2d>);

    /** debug flag */
    bool m_debug;

    std::multimap<int, ExtHit*> map_extHits;

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

    const KLMElementNumbers* m_elementNum;

    /** monitor histograms */
    TH1D* h_eventT0_0;
    TH1D* h_eventT0_1;
    TH1I* h_numTrack;

    TH1I* h_nBHit2dOfTrack;
    TH1I* h_nEHit2dOfTrack;

    TH1D* h_positionDiff;
    TH1D* h_positionXDiff;
    TH1D* h_positionYDiff;
    TH1D* h_positionZDiff;

    TH2D* h_flyTimeB;
    TH2D* h_flyTimeE;

    TH1I* h_numDigit_scint_end;
    TH1I* h_numDigit_scint;
    TH1I* h_numDigit_rpc;

    /** data collection tree */
    TTree* m_outTree;

    /** output infor string */
    std::string infoString;
  };
}
