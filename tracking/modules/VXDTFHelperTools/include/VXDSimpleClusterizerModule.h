/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/MCParticle.h>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>

namespace Belle2 {

  /** Module to convert TrueHits into Clusters using a simplified process. Supports PXD (2D) and SVD (1D)
   * usefull for tracking purposes and crosschecks with the PXD/SVDDigitizer and Clusterizer modules.
   * so far only the coordinates (smeared) and the timestamp (sharp) are useful values
   */
  class VXDSimpleClusterizerModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    VXDSimpleClusterizerModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    void initialize() override;

    /** Called when entering a new run.
     */
    void beginRun() override;

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    void event() override;

    /** This method is called if the current run ends.
     */
    void endRun() override;

    /** initialize variables to avoid nondeterministic behavior */
    void InitializeVariables()
    {
      m_weakSVDHitCtr = 0;
      m_weakPXDHitCtr = 0;
      m_fakeSVDHitCtr = 0;
      m_fakePXDHitCtr = 0;
    }

  protected:
    std::string m_pxdTrueHitsName; /**< PXDTrueHit collection name */
    StoreArray<PXDTrueHit>
    m_pxdTrueHits; /**< the storeArray for pxdTrueHits as member, is faster than recreating link for each event */
    std::string m_svdTrueHitsName; /**< SVDTrueHit collection name */
    StoreArray<SVDTrueHit>
    m_svdTrueHits; /**< the storeArray for svdTrueHits as member, is faster than recreating link for each event */
    std::string m_mcParticlesName; /**< MCParticle collection name */
    StoreArray<MCParticle>
    m_mcParticles; /**< the storeArray for mcParticles as member, is faster than recreating link for each event */
    std::string m_pxdClustersName; /**< PXDCluster collection name */
    StoreArray<PXDCluster>
    m_pxdClusters; /**< the storeArray for pxdClusters as member, is faster than recreating link for each event */
    std::string m_svdClustersName; /**< SVDCluster collection name */
    StoreArray<SVDCluster>
    m_svdClusters; /**< the storeArray for svdClusters as member, is faster than recreating link for each event */

    double m_energyThresholdU; /**< set energy threshold for SVDClusters in u-direction in GeV (standard is 17.4E-6) */
    double m_energyThresholdV; /**< set energy threshold for SVDClusters in v-direction in GeV (standard is 28.6E-6) */
    double m_energyThreshold; /**< set energy threshold for PXDClusters in GeV (standard is 7E-6) */
    bool m_onlyPrimaries; /**< set True if you do not want to have hits by secondary particles */
    double m_uniSigma;  /**< you can define the sigma of the smearing. Standard value is the sigma of the unifom distribution for 0-1: 1/sqrt(12) */
    double m_setMeasSigma; /**< if positive value (in cm) is given it will be used as the sigma to smear the Clusters otherwise pitch/uniSigma will be used */
    int m_weakSVDHitCtr; /**< counts SVDHits whose energy deposit is lower than energyThreshold */
    int m_weakPXDHitCtr; /**< counts PXDHits whose energy deposit is lower than energyThreshold */
    int m_fakeSVDHitCtr; /**< counts SVDHits which were not caused by a primary partice */
    int m_fakePXDHitCtr; /**< counts PXDHits which were not caused by a primary partice */

  };
} // end namespace Belle2
