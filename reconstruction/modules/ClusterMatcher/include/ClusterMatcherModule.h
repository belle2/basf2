
/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *************************************************************************/
#ifndef ClusterMatcherModule_H
#define ClusterMatcherModule_H

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
    virtual void initialize();

    /** process event */
    virtual void event();

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

#endif
