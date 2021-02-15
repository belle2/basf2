/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>

#include <pxd/utilities/PXD2TrackEvent.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>

//#include <pxd/dbobjects/PXDClusterChargeMapPar.h>
#include <pxd/dbobjects/PXDGainMapPar.h>

#include <string>

namespace Belle2 {
  /**
   * Collector module for PXD gain calibration and PXD calibration validation
   *
   * PXD2TrackEvent data store is required to fill TTree objects or histograms
   * used in gain calibration or the validation algorithm.
   *
   * For gain calibration, track clusters are already selected in the event class.
   * Further selections based on cluster size/charge and track properties are available.
   * As in the ClusterChargeCollector, this collector also creates a grid of rectangular
   * regions on each sensor (granularity is steerable with nBinsU and nBinsV) and creates
   * TTree objects for each grid region. Cluster charge values and the expected ones are
   * filled into these trees. The gain is later estimated with the MPV of the ratio,
   * cluster charge / expected value, in the calibration algorithm.
   *
   */
  class PXDPerformanceCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDPerformanceCollectorModule();
    void prepare() override final;
    void collect() override final;
    void startRun() override final;

    /**
     * Collect info on track level
     * @param track the track struct defined in PXD2TrackEvent
     */
    void collectFromTrack(const PXD2TrackEvent::baseType& track);
    /**
     * Collect info for impact parameter study on event level
     * @param event PXD2TrackEvent
     */
    void collectDeltaIP(const PXD2TrackEvent& event);

  private:
    /** Required input PXD2TrackEvent  */
    StoreArray<PXD2TrackEvent> m_pxd2TrackEvents;
    /** Required input EventMetaData */
    StoreObjPtr<EventMetaData> m_evtMetaData;

    /** Name of the collection to use for PXD2TrackEvents */
    std::string m_store2TrackEventsName;

    /** Minimum pt cut for efficiency monitoring */
    float m_minPt4Eff;
    /** Maximum absolute value for x coordinate of vertex */
    float m_maxAbsVx;
    /** Maximum absolute value for y coordinate of vertex */
    float m_maxAbsVy;
    /** Maximum absolute value for z coordinate of vertex */
    float m_maxAbsVz;

    /** Minimum pt cut for resolution monitoring */
    float m_minPt4Res;
    /** Minimum number of SVD hits for resolution */
    int m_minSVDHits;
    /** Minimum number of CDC hits for resolution */
    int m_minCDCHits;
    /** Maximum absolute dip angle (lambda) */
    float m_maxAbsLambda;
    /** Minimum p*Beta*sin(theta_0)^{3/2} */
    float m_minPBetaSinTheta3o2;
    /** Maximum abs(z0) */
    float m_maxAbsZ0;
    /** Maximum abs(d0) */
    float m_maxAbsD0;

    /** Minimum pt cut */
    float m_minPt;
    /** Minimum cluster charge cut  */
    int m_minClusterCharge;
    /**  Minimum cluster size cut */
    int m_minClusterSize;
    /**  Maximum cluster size cut */
    int m_maxClusterSize;

    /** Number of corrections per sensor along u side */
    int m_nBinsU;
    /** Number of corrections per sensor along v side */
    int m_nBinsV;
    /** Payload name for Gain to be read from DB */
    std::string m_gainName;
    /** Flag to fill cluster charge ratio (relative to expected MPV) histograms*/
    bool m_fillChargeRatioHistogram;
    /** Flag to fill cluster charge and its estimated MPV in TTree*/
    bool m_fillChargeTree;

    /** Flag to fill event tree for validation */
    bool m_fillEventTree;
    /** Flag of selection for efficiency validation */
    bool m_selectedEff;
    /** Flag of selection for resolution validation */
    bool m_selectedRes;

    /** Event holder */
    PXD2TrackEvent m_pxd2TrackEvent;
    /** (d0_p + d0_m)/sqrt(2) */
    float m_deltaD0oSqrt2;
    /** (z0_p - z0_m)/sqrt(2) */
    float m_deltaZ0oSqrt2;
    /** Cluster charge in ADU */
    int m_signal;
    /** Estimated cluster charge in ADU */
    float m_estimated;
    /** Run number to be stored in dbtree */
    int m_run;
    /** Experiment number to be stored in dbtree */
    int m_exp;
    /** GainMap to be stored in dbtree */
    PXDGainMapPar m_gainMap;

  };
}
