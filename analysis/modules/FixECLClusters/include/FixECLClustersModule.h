/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>


namespace Belle2 {
  /**
   * Sets ConnectedRegionID and ClusterID to ECLClusters on MC samples with old ECLCluster design.
   *
   * In release-08-00 new design of ECLCluster mdst class was introduced that is conceptually
   * very different from the design used before. The change in the ECLCluster design requires
   * also substantial changes in the analysis package modules (which take as an input ECLClusters
   * or operate with them). In order for the analysis package modules to work with older MC
   * samples the ECLCluster needs to be modified before it is used. In particular the
   * ConnectedRegionID and ClusterID need to be set (on older MC samples these members
   * are set to 0). The ClusterID is set to 1 for all ECLClusters. ConnectedRegionID has to
   * be unique for all clusters and is set by this module to cluster's StoreArray index + 1.
   */
  class FixECLClustersModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    FixECLClustersModule();

    /** Register input and output data */
    virtual void initialize() override;

    /** Event function */
    virtual void event() override;


  private:

    StoreArray<ECLCluster> m_eclClusters; /**< Reconstructed ECLClusters */
  };
}

