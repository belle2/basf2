/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber)                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <trg/ecl/dataobjects/TRGECLCluster.h>

namespace Belle2 {

  /**
   * Match ECLTRGClusters to ECLClusters.
   */
  class ECLTriggerClusterMatcherModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ECLTriggerClusterMatcherModule();

    /** Register input and output data */
    virtual void initialize();

    /** Event */
    virtual void event();

  private:

    StoreArray<ECLCluster> m_eclClusters; /**< eclClusters  */
    StoreArray<TRGECLCluster> m_eclTriggers; /**< eclTriggers  */

    double m_minClusterEnergy {0.1}; /**< minimum cluster energy. */
    double m_maxAngle {0.1}; /**< maximal angular separation */
    double m_minFracEnergy {0.0}; /**< minimum energy fraction tr/cluster */
    double m_maxFracEnergy {2.0}; /**< maximal energy fraction trg/cluster*/
  };
}

