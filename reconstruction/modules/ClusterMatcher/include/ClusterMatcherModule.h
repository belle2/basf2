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

namespace Belle2 {
  class KLMCluster;
  class ECLCluster;
  class Cluster;

  /** Match KLM Clusters to close ECL Clusters. */
  class ClusterMatcherModule : public Module {

  public:

    /** Constructor */
    ClusterMatcherModule();

    /** Initialize. */
    void initialize() override;

    /** Event. */
    void event() override;

  private:

    StoreArray<ECLCluster> m_eclClusters; /**< Required array of input ECLClusters */
    StoreArray<KLMCluster> m_klmClusters; /**< Required array of input KLMClusters */
    StoreArray<Cluster> m_Clusters; /**< Array of output Clusters */
    float m_coneInRad = 0.26; /**< Cone angle for matching (whole cone) */
  };
}
