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

#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Cluster.h>


namespace Belle2 {

  /**
  Match KLM Clusters to close ECL Clusters.
    */
  class ClusterMatcherModule : public Module {

  public:

    /** Constructor */
    ClusterMatcherModule();

    /** Destructor */
    virtual ~ClusterMatcherModule();

    /** init */
    virtual void initialize() override;

    /** process event */
    virtual void event() override;

    /** cone angle for matching (whole cone) */
    float m_coneInRad = 0.26;

  protected:

  private:

    // required input
    StoreArray<ECLCluster> m_eclClusters; /**< Required array of input ECLClusters */
    StoreArray<KLMCluster> m_klmClusters; /**< Required array of input KLMClusters */

    // output
    StoreArray<Cluster> m_Clusters; /**< array of output Clusters */

  }; // end class
} // end namespace Belle2
