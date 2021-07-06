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
  class ECLCluster;
  class TRGECLCluster;

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
    virtual void initialize() override;

    /** Event */
    virtual void event() override;

  private:

    StoreArray<ECLCluster> m_eclClusters; /**< eclClusters  */
    StoreArray<TRGECLCluster> m_eclTriggers; /**< eclTriggers  */

    double m_minClusterEnergy {0.1}; /**< minimum cluster energy. */
    double m_maxAngle {0.1}; /**< maximal angular separation */
    double m_minFracEnergy {0.0}; /**< minimum energy fraction tr/cluster */
    double m_maxFracEnergy {2.0}; /**< maximal energy fraction trg/cluster*/
  };
}
