/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni, Giulia Casarosa                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>
#include <pxd/dataobjects/PXDRawHit.h>

namespace Belle2 {

  /**
   * The module produce a StoreArray of PXDRawHit inside the ROIs.
   * Thus simulation "ONSEN" ROI selection.
   * An oitside of ROI array can be produced on demand.
   */
  class PXDRawHitFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDRawHitFilterModule();

  private:

    /**  */
    void initialize() override final;


    /**  */
    void event() override final;


    bool m_CreateOutside; /**< if set, create list of outside pixels, too */
    std::string m_PXDRawHitsName;  /**< The name of the StoreArray of PXDRawHits to be filtered */
    std::string m_PXDRawHitsInsideROIName;  /**< The name of the StoreArray of Filtered PXDRawHits */
    std::string m_PXDRawHitsOutsideROIName;  /**< The name of the StoreArray of Filtered PXDRawHits */
    std::string m_ROIidsName;  /**< The name of the StoreArray of ROIs */

    SelectSubset< PXDRawHit > m_selectorIN; /**< selector of the subset of PXDRawHits contained in the ROIs*/
    SelectSubset< PXDRawHit > m_selectorOUT; /**< selector of the subset of PXDRawHits NOT contained in the ROIs*/

  };
}
