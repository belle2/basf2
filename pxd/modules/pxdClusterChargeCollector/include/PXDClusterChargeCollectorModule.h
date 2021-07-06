/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

#include <pxd/dataobjects/PXDCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <pxd/dbobjects/PXDClusterChargeMapPar.h>
#include <pxd/dbobjects/PXDGainMapPar.h>

#include <string>

namespace Belle2 {
  /**
   * Collector module for PXD gain calibration
   *
   * Firstly, the collector module selects signal like clusters based on cluster size
   * and cluster charge. Secondly, the collector creates a grid of rectangular regions
   * on each sensor (granularity is steerable with nBinsU and nBinsV) and creates TTree
   * objects for each grid region. Cluster charge values are filled into these trees,
   * or optionally into histograms, to be used in calibration algorithms.
   *
   * Option to use track matched clusters (with or without angle projection of charge).
   */
  class PXDClusterChargeCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDClusterChargeCollectorModule();
    /** Prepare */
    void prepare() override final;
    /** Collect */
    void collect() override final;
    /** Start run */
    void startRun() override final;

  private:
    /** Required input PXDClusters  */
    StoreArray<PXDCluster> m_pxdClusters;
    /** Optional input MCParticles  */
    StoreArray<MCParticle> m_mcParticles;
    /** Required input Tracks */
    StoreArray<Track> m_tracks;
    /** Required input RecoTracks */
    StoreArray<RecoTrack> m_recoTracks;
    /** Required input EventMetaData */
    StoreObjPtr<EventMetaData> m_evtMetaData;

    /** Name of the collection to use for PXDClusters */
    std::string m_storeClustersName;
    /** Name of the collection to use for Tracks */
    std::string m_storeTracksName;
    /** Name of the collection to use for RecoTracks */
    std::string m_storeRecoTracksName;
    /** Name of the collection to use for MCParticles */
    std::string m_storeMCParticlesName;

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
    /** Payload name for Cluster Charge to be read from DB */
    std::string m_chargeName;
    /** Payload name for Gain to be read from DB */
    std::string m_gainName;
    /** Flag to deal with MC samples*/
    bool m_mcSamples;
    /** Flag to check relations between PXDClusters and PXDClustersFromTracks*/
    bool m_relationCheck;
    /** Flag to fill cluster charge histograms*/
    bool m_fillChargeHistogram;
    /** Flag to use track matched clusters (=1) and apply theta angle projection to cluster charge (=2)*/
    int m_matchTrack;

    /** Cluster charge in ADU */
    int m_signal;
    /** Run number to be stored in dbtree */
    int m_run;
    /** Experiment number to be stored in dbtree */
    int m_exp;
    /** ChargeMap to be stored in dbtree */
    PXDClusterChargeMapPar m_chargeMap;
    /** GainMap to be stored in dbtree */
    PXDGainMapPar m_gainMap;

    /** Pointer to ChargeMap calibration in DB */
    DBObjPtr<PXDClusterChargeMapPar> m_DBChargeMapPar;
    /** Pointer to GainMap calibration in DB */
    DBObjPtr<PXDGainMapPar> m_DBGainMapPar;
  };
}
