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
#include <framework/datastore/SelectSubset.h>
#include <tracking/dataobjects/ROIid.h>
#include <svd/dataobjects/SVDShaperDigit.h>

namespace Belle2 {

  /**
   * The module produce a StoreArray of SVDShaperDigit inside the ROIs.
   *
   *    *
   */
  class SVDShaperDigitFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDShaperDigitFilterModule();

    /** Destructor. */
    ~SVDShaperDigitFilterModule();

    /**  register new SVDSShaperDigits store arrays (inside/ouside ROIs)*/
    void initialize() override;


    /**  Filtering of digits inside/outside ROIs*/
    void event() override;


  private:

    StoreArray<ROIid> m_ROIs; /**< rois store array */
    StoreArray<SVDShaperDigit> m_SVDShaperDigits;   /**< The SVDShaperDigits to be filtered */

    bool m_CreateOutside; /**< if set, create list of outside pixels, too */
    std::string m_SVDShaperDigitsName;  /**< The name of the StoreArray of SVDShaperDigits to be filtered */
    std::string m_SVDShaperDigitsInsideROIName;  /**< The name of the StoreArray of Filtered SVDShaperDigits */
    std::string m_SVDShaperDigitsOutsideROIName;  /**< The name of the StoreArray of Filtered SVDShaperDigits */
    std::string m_ROIidsName;  /**< The name of the StoreArray of ROIs */

    SelectSubset< SVDShaperDigit > m_selectorIN; /**< selector of the subset of SVDShaperDigits contained in the ROIs*/
    SelectSubset< SVDShaperDigit > m_selectorOUT; /**< selector of the subset of SVDShaperDigits NOT contained in the ROIs*/

  };
}

