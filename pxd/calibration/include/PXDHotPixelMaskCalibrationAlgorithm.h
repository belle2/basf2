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

    /// Minimum number of collected events
    int minEvents;

    /// Minimum number of hits per pixel
    int minHits;

    /// Pixels with higher occupancy are hot and will be masked
    float maxOccupancy;

    /// Mask drain lines with too high average occupancy after single pixel masking
    bool maskDrains;

    /// Minimum number of hits per drain line
    int minHitsDrain;

    /// Mask drain lines whose avaerage occupancy exceeds this limit
    float maxOccupancyDrain;

    /// Mask rows with too high average occupancy after single pixel masking
    bool maskRows;

    /// Minimum number of hits per row
    int minHitsRow;

    /// Mask rows whose average occupancy exceeds this limit
    float maxOccupancyRow;

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:
    /** Number of vCells of Belle II PXD sensors*/
    const unsigned short c_nVCells = 768;
    /** Number of uCells of Belle II PXD sensors*/
    const unsigned short c_nUCells = 250;
    /** Number of drain lines of Belle II PXD sensors*/
    const unsigned short c_nDrains = 1000;

  };
} // namespace Belle2


