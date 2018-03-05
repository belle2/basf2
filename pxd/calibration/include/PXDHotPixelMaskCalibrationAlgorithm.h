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

    /// Mask complete ucells with too high average occupancy after single pixel masking
    bool maskUCells;

    /// Minimum number of hits per uCell
    int minHitsU;

    /// Complete uCells with higher average occupancy are hot and will be masked
    float maxOccupancyU;

    /// Mask complete vcells with too high average occupancy after single pixel masking
    bool maskVCells;

    /// Minimum number of hits per vCell
    int minHitsV;

    /// Complete vCells with higher average occupancy are hot and will be masked
    float maxOccupancyV;

  protected:

    /// Run algo on data
    virtual EResult calibrate();


  };
} // namespace Belle2


