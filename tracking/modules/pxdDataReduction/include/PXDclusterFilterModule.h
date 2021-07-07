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
#include <tracking/dataobjects/ROIid.h>
#include <framework/database/DBObjPtr.h>
#include <simulation/dbobjects/ROIParameters.h>

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
    void initialize() override;

    /**  */
    void beginRun() override final;

    /**  */
    void event() override;

    /**  all the actual work is done here */
    void filterClusters();

    /**  all the actual work is done here */
    void copyClusters();

    bool m_CreateOutside = false; /**< if set, create list of outside pixels, too */
    std::string m_PXDClustersName;  /**< The name of the StoreArray of PXDClusters to be filtered */
    std::string m_PXDClustersInsideROIName;  /**< The name of the StoreArray of Filtered PXDClusters */
    std::string m_PXDClustersOutsideROIName;  /**< The name of the StoreArray of Filtered PXDClusters */
    std::string m_ROIidsName;  /**< The name of the StoreArray of ROIs */
    bool m_overrideDB = false; /**< if set, overwrites ROI-finding settings in DB */
    bool m_enableFiltering = false; /**< enables/disables ROI-finding if overwriteDB=True */


    int m_countNthEvent = 0;  /**< Event counter to be able to disable data reduction for every Nth event */
    DBObjPtr<ROIParameters> m_roiParameters;  /**< Configuration parameters for ROIs */
    int m_skipEveryNth = -1;  /**< Parameter from DB for how many events to skip data reduction */

    SelectSubset< PXDCluster > m_selectorIN; /**< selector of the subset of PXDClusters contained in the ROIs*/
    SelectSubset< PXDCluster > m_selectorOUT; /**< selector of the subset of PXDClusters NOT contained in the ROIs*/

    /// Check for cluster overlaps - a pixel shared with two or more clusters
    bool Overlaps(const ROIid& theROI, const PXDCluster& thePXDCluster);

  };
}
