/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Benjamin Schwenker                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * Class implementing PXD hot pixel masking calibration algorithm
   */
  class  PXDHotPixelMaskCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDHotPixelMaskCalibrationAlgorithm
    PXDHotPixelMaskCalibrationAlgorithm();

    /// Destructor
    virtual ~PXDHotPixelMaskCalibrationAlgorithm() {}

    /// Minimum number of collected events
    int minEvents;

    /// Minimum number of hits per pixel
    int minHits;

    /// Pixels with higher occupancy are hot and will be masked
    float maxOccupancy;

  protected:

    /// Run algo on data
    virtual EResult calibrate();
  };
} // namespace Belle2


