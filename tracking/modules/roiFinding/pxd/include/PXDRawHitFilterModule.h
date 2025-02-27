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
#include <pxd/dataobjects/PXDRawHit.h>

namespace Belle2 {
  class ROIid;

  /**
   * The module produce a StoreArray of PXDRawHit inside the ROIs.
   * Thus simulation "ONSEN" ROI selection.
   * An outside of ROI array can be produced on demand.
   */
  class PXDRawHitFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDRawHitFilterModule();

  private:

    /** Initialize all necessary variables */
    void initialize() override final;


    /** Event function */
    void event() override final;

    StoreArray<ROIid> m_ROIs;   /**< ROIs StoreArray */
    StoreArray<PXDRawHit> m_PXDRawHits;   /**< PXDRawHits StoreArray */

    SelectSubset< PXDRawHit > m_selectorIN; /**< selector of the subset of PXDRawHits contained in the ROIs*/
    SelectSubset< PXDRawHit > m_selectorOUT; /**< selector of the subset of PXDRawHits NOT contained in the ROIs*/

    bool m_CreateOutside; /**< if set, create list of outside pixels, too */
    std::string m_PXDRawHitsName;  /**< The name of the StoreArray of PXDRawHits to be filtered */
    std::string m_PXDRawHitsInsideROIName;  /**< The name of the StoreArray of Filtered PXDRawHits */
    std::string m_PXDRawHitsOutsideROIName;  /**< The name of the StoreArray of Filtered PXDRawHits */
    std::string m_ROIidsName;  /**< The name of the StoreArray of ROIs */

  };
}
