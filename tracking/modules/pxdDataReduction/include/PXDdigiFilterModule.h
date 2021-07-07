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
#include <pxd/dataobjects/PXDDigit.h>
#include <framework/database/DBObjPtr.h>
#include <simulation/dbobjects/ROIParameters.h>

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
    void beginRun() override final;

    /**  */
    void event() override final;

    /**  all the actual work is done here */
    void filterDigits();

    /**  all the actual work is done here */
    void copyDigits();


    bool m_CreateOutside = false; /**< if set, create list of outside pixels, too */
    std::string m_PXDDigitsName;  /**< The name of the StoreArray of PXDDigits to be filtered */
    std::string m_PXDDigitsInsideROIName;  /**< The name of the StoreArray of Filtered PXDDigits */
    std::string m_PXDDigitsOutsideROIName;  /**< The name of the StoreArray of Filtered PXDDigits */
    std::string m_ROIidsName;  /**< The name of the StoreArray of ROIs */
    bool m_overrideDB = false; /**< if set, overwrites ROI-finding settings in DB */
    bool m_usePXDDataReduction = false; /**< enables/disables ROI-finding if overwriteDB=True */


    int m_countNthEvent = 0;  /**< Event counter to be able to disable data reduction for every Nth event */
    DBObjPtr<ROIParameters> m_roiParameters;  /**< Configuration parameters for ROIs */
    int m_skipEveryNth = -1;  /**< Parameter from DB for how many events to skip data reduction */

    SelectSubset< PXDDigit > m_selectorIN; /**< selector of the subset of PXDDigits contained in the ROIs*/
    SelectSubset< PXDDigit > m_selectorOUT; /**< selector of the subset of PXDDigits NOT contained in the ROIs*/

  };
}
