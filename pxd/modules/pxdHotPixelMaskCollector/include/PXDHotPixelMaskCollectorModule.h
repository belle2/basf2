/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <calibration/CalibrationCollectorModule.h>
#include <string>

namespace Belle2 {
  /**
   * Calibration Collector Module for PXD hot pixel masking from PXDDigits
   *
   *
   */
  class PXDHotPixelMaskCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDHotPixelMaskCollectorModule();
    void prepare() override final;
    void collect() override final;

  private:
    /**< Required input for PXDigits  */
    StoreArray<PXDDigit> m_pxdDigit;

    /** Name of the collection to use for PXDDigits */
    std::string m_storeDigitsName;

    /** Minimum charge (ADU) for detecting a hit. */
    int m_0cut;

  };
}
