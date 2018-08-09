/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: bjoern.spruck@belle2.org                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/SelectSubset.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <tracking/dataobjects/ROIid.h>

namespace Belle2 {

  /**
   * The module produce a StoreArray of PXDCluster inside the ROIs.
   *
   *    *
   */
  class PXDclusterFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDclusterFilterModule();

  private:

    /**  */
    void initialize();

    /**  */
    void event();

    bool m_CreateOutside; /**< if set, create list of outside pixels, too */
    std::string m_PXDClustersName;  /**< The name of the StoreArray of PXDClusters to be filtered */
    std::string m_PXDClustersInsideROIName;  /**< The name of the StoreArray of Filtered PXDClusters */
    std::string m_PXDClustersOutsideROIName;  /**< The name of the StoreArray of Filtered PXDClusters */
    std::string m_ROIidsName;  /**< The name of the StoreArray of ROIs */

    SelectSubset< PXDCluster > m_selectorIN; /**< selector of the subset of PXDClusters contained in the ROIs*/
    SelectSubset< PXDCluster > m_selectorOUT; /**< selector of the subset of PXDClusters NOT contained in the ROIs*/

    bool Overlaps(const ROIid& theROI, const PXDCluster& thePXDCluster);

  };
}
