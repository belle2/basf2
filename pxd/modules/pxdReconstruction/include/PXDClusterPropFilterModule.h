/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>
#include <pxd/dataobjects/PXDCluster.h>

namespace Belle2 {

  /**
   * The module produce a StoreArray of PXDCluster with specific properties.
   *
   *    *
   */
  class PXDClusterPropFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDClusterPropFilterModule();

  private:

    /**  */
    void initialize() override final;

    /**  */
    void event() override final;

    /**  all the actual work is done here */
    void filterClusters();

    bool m_CreateInside = true; /**< if set, create list of inside cuts */
    bool m_CreateOutside = false; /**< if set, create list of outside cuts */
    double m_minCharge = 0.0;  /**< minimum charge, including value */
    double m_maxCharge = 9999.0;  /**< maximum charge, excluding value */
    int m_minSize = 1;  /**< minimum size, including value */
    int m_maxSize = 99;  /**< maximum size, excluding value */

    std::string m_PXDClustersName;  /**< The name of the StoreArray of PXDClusters to be filtered */
    std::string m_PXDClustersInsideCutsName;  /**< The name of the StoreArray of Filtered PXDClusters inside cuts */
    std::string m_PXDClustersOutsideCutsName;  /**< The name of the StoreArray of Filtered PXDClusters outside cuts */

    SelectSubset< PXDCluster > m_selectorIN; /**< selector of the subset of PXDClusters contained in the ROIs*/
    SelectSubset< PXDCluster > m_selectorOUT; /**< selector of the subset of PXDClusters NOT contained in the ROIs*/

    bool CheckCuts(const PXDCluster& thePXDCluster);/**< the actual check for cluster properties */

  };
}
