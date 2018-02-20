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
   * Calibration Collector Module for PXD hot pixel masking
   *
   *    *
   */
  class PXDHotPixelMaskCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDHotPixelMaskCollectorModule();
    void prepare();
    void collect();

  private:
    /** Name of the collection to use for the PXDDigits */
    std::string m_filterSensorName;

    StoreArray<PXDDigit> m_pxdDigits;
  };
}
