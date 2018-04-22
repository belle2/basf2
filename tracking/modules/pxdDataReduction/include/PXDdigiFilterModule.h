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
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/SelectSubset.h>
#include <pxd/dataobjects/PXDDigit.h>

namespace Belle2 {

  /**
   * The module produce a StoreArray of PXDDigit inside the ROIs.
   * Thus simulation "ONSEN" ROI selection.
   * An oitside of ROI array can be produced on demand.
   */
  class PXDdigiFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDdigiFilterModule();

  private:

    /**  */
    void initialize() override final;


    /**  */
    void event() override final;


    bool m_CreateOutside; /**< if set, create list of outside pixels, too */
    std::string m_PXDDigitsName;  /**< The name of the StoreArray of PXDDigits to be filtered */
    std::string m_PXDDigitsInsideROIName;  /**< The name of the StoreArray of Filtered PXDDigits */
    std::string m_PXDDigitsOutsideROIName;  /**< The name of the StoreArray of Filtered PXDDigits */
    std::string m_ROIidsName;  /**< The name of the StoreArray of ROIs */

    SelectSubset< PXDDigit > m_selectorIN; /**< selector of the subset of PXDDigits contained in the ROIs*/
    SelectSubset< PXDDigit > m_selectorOUT; /**< selector of the subset of PXDDigits NOT contained in the ROIs*/

  };
}
