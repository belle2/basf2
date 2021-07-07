/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    /** Prepare */
    void prepare() override final;
    /** Collect */
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
