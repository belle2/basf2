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
   * ECLClusterRounder module
   * Rounds all Double32_t members of all ECLClusters to precision as if read from mdst
   */
  class ECLClusterRounderModule : public Module {

  public:

    /**
     * Constructor.
     */
    ECLClusterRounderModule();

    /**
     * Initialization.
     */
    void initialize() override;

    /**
     * Event handling.
     */
    void event() override;

  private:

    static double roundToPrecision(Double32_t value, double min, double max, int nBits);

    StoreArray<ECLCluster> m_eclclusters;

  };

}